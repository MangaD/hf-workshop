#ifndef ANDROIDBINXMLPARSER_HPP
#define ANDROIDBINXMLPARSER_HPP

#include "ByteBuffer.hpp"
#include "../utils.hpp"   // int_to_hex

#include <cassert>
#include <climits>        // INT32_MAX
#include <codecvt>
#include <cstddef>        // std::size_t
#include <locale>         // codecvt
#include <optional>
#include <unordered_map>

namespace apksigner {
	
/**
 * Indicates that an error occurred while parsing a document.
 */
class xml_parser_exception : public std::exception {
	public:
		explicit xml_parser_exception(const std::string &message = "xml_parser_exception")
			: std::exception(), error_message(message) {}
		const char *what() const noexcept
		{
			return error_message.c_str();
		}
	private:
		std::string error_message;
};

/**
 * Chunk of a document. Each chunk is tagged with a type and consists of a header followed by
 * contents.
 */
class Chunk {

public:

	constexpr static int TYPE_STRING_POOL = 1;
	constexpr static int TYPE_RES_XML = 3;
	constexpr static int RES_XML_TYPE_START_ELEMENT = 0x0102;
	constexpr static int RES_XML_TYPE_END_ELEMENT = 0x0103;
	constexpr static int RES_XML_TYPE_RESOURCE_MAP = 0x0180;
	constexpr static size_t HEADER_MIN_SIZE_BYTES = 8;

private:

	int mType;
	ByteBuffer mHeader;
	ByteBuffer mContents;

	Chunk(int type, ByteBuffer& header, ByteBuffer& contents) :
	    mType(type), mHeader(header), mContents(contents) { }
	Chunk(int type, ByteBuffer&& header, ByteBuffer&& contents) :
	    mType(type), mHeader(header), mContents(contents) { }

public:

	ByteBuffer getContents() { return { mContents, mContents.getPosition(), mContents.getSize() }; }
	ByteBuffer getHeader() { return { mHeader, mHeader.getPosition(), mHeader.getSize() }; }
	int getType() { return mType; }

	/**
	 * Consumes the chunk located at the current position of the input and returns the chunk
	 * or std::nullopt if there is no chunk left in the input.
	 */
	static std::optional<Chunk> get(ByteBuffer& input) {

		if (input.remaining() < HEADER_MIN_SIZE_BYTES) {
			// Android ignores the last chunk if its header is too big to fit into the file
			input.setPosition(input.getSize());
			return std::nullopt;
		}

		size_t originalPosition = input.getPosition();
		int type = static_cast<int>(input.getUnsignedInt16());
		size_t headerSize = input.getUnsignedInt16();
		size_t chunkSize = input.getUnsignedInt32();
		size_t chunkRemaining = chunkSize - 8;

		if (chunkRemaining > input.remaining()) {
			// Android ignores the last chunk if it's too big to fit into the file
			input.setPosition(input.getSize());
			return std::nullopt;
		}

		if (headerSize < HEADER_MIN_SIZE_BYTES) {
			throw xml_parser_exception(std::string("Malformed chunk: header too short: ") +
			                           std::to_string(headerSize) + " bytes");
		} else if (headerSize > chunkSize) {
			throw xml_parser_exception(std::string("Malformed chunk: header too long: ") +
			                           std::to_string(headerSize) + " bytes. Chunk size: " +
			                           std::to_string(chunkSize) + " bytes");
		}

		size_t contentStartPosition = originalPosition + headerSize;
		size_t chunkEndPosition = originalPosition + chunkSize;

		input.setPosition(chunkEndPosition);
		return { {type,
			{ input, originalPosition, contentStartPosition },
			{ input, contentStartPosition, chunkEndPosition}} };
	}
};

/**
 * String pool of a document. Strings are referenced by their {@code 0}-based index in the pool.
 */
class StringPool {

private:
	constexpr static int FLAG_UTF8 = 1 << 8;

	ByteBuffer mChunkContents;
	ByteBuffer mStringsSection;
	size_t mStringCount;
	bool mUtf8Encoded;
	std::unordered_map<size_t, std::string> mCachedStrings;

public:
	StringPool() : mChunkContents(), mStringsSection(), mStringCount(0),
	mUtf8Encoded(false), mCachedStrings() {}

	/**
	 * Constructs a new string pool from the provided chunk.
	 */
	StringPool(Chunk chunk) : mChunkContents(), mStringsSection(), mStringCount(0),
		mUtf8Encoded(false), mCachedStrings() {

		auto header = chunk.getHeader();
		size_t headerSizeBytes = header.remaining();
		header.setPosition(Chunk::HEADER_MIN_SIZE_BYTES);
		if (header.remaining() < 20) {
			throw xml_parser_exception(std::string("XML chunk's header too short. Required at least 20 bytes. Available: ")
			                           + std::to_string(header.remaining()) + " bytes");
		}
		size_t stringCount = header.getUnsignedInt32();
		if (stringCount > INT32_MAX) {
			throw xml_parser_exception("Too many strings: " + std::to_string(stringCount));
		}
		mStringCount = stringCount;
		size_t styleCount = header.getUnsignedInt32();
		if (styleCount > INT32_MAX) {
			throw xml_parser_exception("Too many styles: " + std::to_string(styleCount));
		}
		int flags = static_cast<int>(header.getUnsignedInt32());
		size_t stringsStartOffset = header.getUnsignedInt32();
		size_t stylesStartOffset = header.getUnsignedInt32();

		auto contents = chunk.getContents();

		if (mStringCount > 0) {
			assert((static_cast<int>(stringsStartOffset) - static_cast<int>(headerSizeBytes)) >= 0);
			size_t stringsSectionStartOffsetInContents = stringsStartOffset - headerSizeBytes;
			size_t stringsSectionEndOffsetInContents;
			if (styleCount > 0) {
				// Styles section follows the strings section
				if (stylesStartOffset < stringsStartOffset) {
					throw xml_parser_exception(std::string("Styles offset (") +
					                           std::to_string(stylesStartOffset) + ") < strings offset (" +
					                           std::to_string(stringsStartOffset) + ")");
				}
				assert((static_cast<int>(stylesStartOffset) - static_cast<int>(headerSizeBytes)) >= 0);
				stringsSectionEndOffsetInContents = stylesStartOffset - headerSizeBytes;
			} else {
				stringsSectionEndOffsetInContents = contents.remaining();
			}
			mStringsSection = {contents, stringsSectionStartOffsetInContents, stringsSectionEndOffsetInContents};
		}
		mUtf8Encoded = ((flags & FLAG_UTF8) != 0);
		mChunkContents = contents;
	}

	/**
	 * Returns the string located at the specified {@code 0}-based index in this pool.
	 */
	std::string getString(size_t index) {

		if (index >= mStringCount) {
			throw xml_parser_exception(std::string("Unsupported string index: ") +
			                           std::to_string(index) +
			                           ", max: " + std::to_string((static_cast<int>(mStringCount) - 1)));
		}
		std::string result;
		try {
			result = mCachedStrings.at(index);
		} catch (const std::out_of_range&) {
			size_t offsetInStringsSection = mChunkContents.getUnsignedInt32(index * 4);
			if (offsetInStringsSection >= mStringsSection.getSize()) {
				throw xml_parser_exception(std::string("Offset of string index ") +
				                           std::to_string(index) +
				                           " out of bounds: " +
				                           std::to_string(offsetInStringsSection) +
				                           ", max: " + std::to_string((static_cast<int>(mStringsSection.getSize()) - 1)));
			}
			mStringsSection.setPosition(offsetInStringsSection);
			result = (mUtf8Encoded ? this->getLengthPrefixedUtf8EncodedString(mStringsSection)
			          : this->getLengthPrefixedUtf16EncodedString(mStringsSection));
			mCachedStrings.emplace(index, result);
		}
		return result;
	}

private:
	static std::string getLengthPrefixedUtf16EncodedString(ByteBuffer &encoded) {
		// If the length (in uint16s) is 0x7fff or lower, it is stored as a single uint16.
		// Otherwise, it is stored as a big-endian uint32 with highest bit set. Thus, the range
		// of supported values is 0 to 0x7fffffff inclusive.
		size_t lengthChars = encoded.getUnsignedInt16();
		if ((lengthChars & 0x8000) != 0) {
			lengthChars = ((lengthChars & 0x7fff) << 16) | encoded.getUnsignedInt16();
		}
		if (lengthChars > (INT32_MAX / 2)) {
			throw xml_parser_exception(std::string("String too long: ") + std::to_string(lengthChars) + " uint16s");
		}
		size_t lengthBytes = lengthChars * 2;
		auto buffer = encoded.getBuffer();
		size_t bufOffset = encoded.getPosition();
		encoded.setPosition(encoded.getPosition() + lengthBytes);
		// Reproduce the behavior of Android runtime which requires that the UTF-16 encoded
		// array of bytes is NULL terminated.
		if ((buffer.at(bufOffset + lengthBytes) != 0) || (buffer.at(bufOffset + lengthBytes + 1) != 0)) {
			throw xml_parser_exception("UTF-16 encoded form of string not NULL terminated");
		}
		// https://stackoverflow.com/a/34115397/3049315
		// David - must cast to char16_t or the internal size of the string will be doubled
		std::u16string s16{ reinterpret_cast<char16_t*>(buffer.data() + bufOffset), reinterpret_cast<char16_t*>(buffer.data() + bufOffset + lengthBytes)};
		std::string u8_conv = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(s16);
		return u8_conv;
	}

	static std::string getLengthPrefixedUtf8EncodedString(ByteBuffer &encoded) {
		// If the length (in bytes) is 0x7f or lower, it is stored as a single uint8. Otherwise,
		// it is stored as a big-endian uint16 with highest bit set. Thus, the range of
		// supported values is 0 to 0x7fff inclusive.

		// Skip UTF-16 encoded length (in uint16s)
		size_t lengthBytes = encoded.getUnsignedInt8();
		if ((lengthBytes & 0x80) != 0) {
			lengthBytes = ((lengthBytes & 0x7f) << 8) | encoded.getUnsignedInt8();
		}
		// Read UTF-8 encoded length (in bytes)
		lengthBytes = encoded.getUnsignedInt8();
		if ((lengthBytes & 0x80) != 0) {
			lengthBytes = ((lengthBytes & 0x7f) << 8) | encoded.getUnsignedInt8();
		}
		auto buffer = encoded.getBuffer();
		size_t bufOffset = encoded.getPosition();
		encoded.setPosition(encoded.getPosition() + lengthBytes);
		// Reproduce the behavior of Android runtime which requires that the UTF-8 encoded array
		// of bytes is NULL terminated.
		if (buffer.at(bufOffset + lengthBytes) != 0) {
			throw xml_parser_exception("UTF-8 encoded form of string not NULL terminated");
		}
		return { buffer.begin() + bufOffset, buffer.begin() + bufOffset + lengthBytes };
	}

};

/**
 * Resource map of a document. Resource IDs are referenced by their zero-based index in the
 * map.
 */
class ResourceMap {
	ByteBuffer mChunkContents;
	size_t mEntryCount;
public:
	ResourceMap() : mChunkContents(), mEntryCount(0) {}

	/**
	 * Constructs a new resource map from the provided chunk.
	 */
	ResourceMap(Chunk chunk) : mChunkContents(chunk.getContents()),
	            mEntryCount(mChunkContents.remaining() /  4) // Each entry of the map is four bytes long, containing the int32 resource ID.
	{}

	/**
	 * Returns the resource ID located at the specified zero-based index in this pool or
	 * zero if the index is out of range.
	 */
	int getResourceId(size_t index) {
		if (index >= mEntryCount) {
			return 0;
		}
		return static_cast<int>(mChunkContents.getUnsignedInt32(index * 4));
	}
};


class Attribute {

private:

	long mNsId;
	long mNameId;
	int mValueType;
	int mValueData;
	StringPool mStringPool;
	ResourceMap mResourceMap;

public:
	constexpr static int TYPE_REFERENCE = 1;
	constexpr static int TYPE_STRING = 3;
	constexpr static int TYPE_INT_DEC = 0x10;
	constexpr static int TYPE_INT_HEX = 0x11;
	constexpr static int TYPE_INT_BOOLEAN = 0x12;
	constexpr static long  NO_NAMESPACE = 0xffffffffL;

	Attribute(long nsId, long nameId, int valueType, int valueData,
	          StringPool stringPool, ResourceMap resourceMap) : mNsId(nsId),
		mNameId(nameId), mValueType(valueType), mValueData(valueData),
		mStringPool(stringPool), mResourceMap(resourceMap) { }

	int getNameResourceId() { return  mResourceMap.getResourceId(mNameId); }

	std::string getName() { return mStringPool.getString(mNameId); }

	std::string getNamespace() {
		return (mNsId != NO_NAMESPACE) ? mStringPool.getString(mNsId) : "";
	}

	int getValueType() { return mValueType; }

	int getIntValue() {
		switch (mValueType) {
			case TYPE_REFERENCE:
			case TYPE_INT_DEC:
			case TYPE_INT_HEX:
			case TYPE_INT_BOOLEAN:
				return mValueData;
			default:
				throw xml_parser_exception("Cannot coerce to int: value type " + std::to_string(mValueType));
		}
	}

	bool getBooleanValue() {
		switch (mValueType) {
			case TYPE_INT_BOOLEAN:
				return mValueData != 0;
			default:
				throw xml_parser_exception("Cannot coerce to boolean: value type " + std::to_string(mValueType));
		}
	}

	std::string getStringValue() {
		switch (mValueType) {
			case TYPE_STRING:
				return mStringPool.getString(mValueData & 0xffffffffL);
			case TYPE_INT_DEC:
				return std::to_string(mValueData);
			case TYPE_INT_HEX:
				return "0x" + int_to_hex(mValueData, 4);
			case TYPE_INT_BOOLEAN:
				return (mValueData != 0 ? "true" : "false");
			case TYPE_REFERENCE:
				return "@" + int_to_hex(mValueData, 4);
			default:
				throw xml_parser_exception("Cannot coerce to string: value type " + std::to_string(mValueType));
		}
	}
};


// Inspired from: src/main/java/com/android/apksig/internal/apk/AndroidBinXmlParser.java
class AndroidBinXmlParser {

public:

	/** Event: start of document. */
	constexpr static int EVENT_START_DOCUMENT = 1;

	/** Event: end of document. */
	constexpr static int EVENT_END_DOCUMENT = 2;

	/** Event: start of an element. */
	constexpr static int EVENT_START_ELEMENT = 3;

	/** Event: end of an document. */
	constexpr static int EVENT_END_ELEMENT = 4;

	/** Attribute value type is not supported by this parser. */
	constexpr static int VALUE_TYPE_UNSUPPORTED = 0;

	/** Attribute value is a string. Use {@link #getAttributeStringValue(int)} to obtain it. */
	constexpr static int VALUE_TYPE_STRING = 1;

	/** Attribute value is an integer. Use {@link #getAttributeIntValue(int)} to obtain it. */
	constexpr static int VALUE_TYPE_INT = 2;

	/**
	 * Attribute value is a resource reference. Use {@link #getAttributeIntValue(int)} to obtain it.
	 */
	constexpr static int VALUE_TYPE_REFERENCE = 3;

	/** Attribute value is a boolean. Use {@link #getAttributeBooleanValue(int)} to obtain it. */
	constexpr static int VALUE_TYPE_BOOLEAN = 4;

private:

	constexpr static long NO_NAMESPACE = 0xffffffffL;

	std::optional<StringPool> mStringPool;
	std::optional<ResourceMap> mResourceMap;
	int mCurrentEvent;
	int mDepth;
	std::string mCurrentElementName;
	std::string mCurrentElementNamespace;
	size_t mCurrentElementAttributeCount;
	size_t mCurrentElementAttrSizeBytes;
	ByteBuffer mCurrentElementAttributesContents;
	std::vector<Attribute> mCurrentElementAttributes;

	ByteBuffer mXml;

public:

	AndroidBinXmlParser(ByteBuffer& xml) : mStringPool(std::nullopt), mResourceMap(std::nullopt),
		mCurrentEvent(EVENT_START_DOCUMENT), mDepth(0), mCurrentElementName(),
		mCurrentElementNamespace(), mCurrentElementAttributeCount(0),
		mCurrentElementAttrSizeBytes(0), mCurrentElementAttributesContents(),
		mCurrentElementAttributes(), mXml() {

		std::optional<Chunk> resXmlChunk = std::nullopt;
		while (xml.hasRemaining()) {
			auto chunk = Chunk::get(xml);
			if (chunk == std::nullopt) break;
			if (chunk->getType() == Chunk::TYPE_RES_XML) {
				resXmlChunk = chunk;
				break;
			}
		}
		if (resXmlChunk == std::nullopt) {
			throw xml_parser_exception("No XML chunk in file");
		}
		this->mXml = resXmlChunk->getContents();
	}

	int getEventType() { return mCurrentEvent; }
	int getDepth() { return mDepth; }
	std::string getName() {
		if ((mCurrentEvent != EVENT_START_ELEMENT) && (mCurrentEvent != EVENT_END_ELEMENT)) {
			return "";
		}
		return mCurrentElementName;
	}
	std::string getNamespace() {
		if ((mCurrentEvent != EVENT_START_ELEMENT) && (mCurrentEvent != EVENT_END_ELEMENT)) {
			return "";
		}
		return mCurrentElementNamespace;
	}
	size_t getAttributeCount() {
		if (mCurrentEvent != EVENT_START_ELEMENT) {
			return -1;
		}
		return mCurrentElementAttributeCount;
	}
	int getAttributeNameResourceId(size_t index) {
		return getAttribute(index).getNameResourceId();
	}
	Attribute getAttribute(size_t index) {
		if (mCurrentEvent != EVENT_START_ELEMENT) {
			throw xml_parser_exception("Current event not a START_ELEMENT");
		}
		if (index >= mCurrentElementAttributeCount) {
			throw xml_parser_exception("index must be <= attr count (" + std::to_string(mCurrentElementAttributeCount) + ")");
		}
		parseCurrentElementAttributesIfNotParsed();
		return mCurrentElementAttributes.at(index);
	}
	void parseCurrentElementAttributesIfNotParsed() {
		if (!mCurrentElementAttributes.empty()) {
			return;
		}
		for (size_t i = 0; i < mCurrentElementAttributeCount; ++i) {
			auto startPosition = i * mCurrentElementAttrSizeBytes;
			ByteBuffer attr{mCurrentElementAttributesContents, startPosition, startPosition + mCurrentElementAttrSizeBytes};
			size_t nsId = attr.getUnsignedInt32();
			size_t nameId = attr.getUnsignedInt32();
			attr.setPosition(attr.getPosition() + 7); // skip ignored fields
			size_t valueType = attr.getUnsignedInt8();
			size_t valueData = attr.getUnsignedInt32();
			mCurrentElementAttributes.emplace_back(nsId, nameId, valueType, valueData, mStringPool.value(), mResourceMap.value());
		}
	}
	int getAttributeValueType(size_t index) {
		int type = getAttribute(index).getValueType();
		switch (type) {
		case Attribute::TYPE_STRING:
			return VALUE_TYPE_STRING;
		case Attribute::TYPE_INT_DEC:
		case Attribute::TYPE_INT_HEX:
			return VALUE_TYPE_INT;
		case Attribute::TYPE_REFERENCE:
			return VALUE_TYPE_REFERENCE;
		case Attribute::TYPE_INT_BOOLEAN:
			return VALUE_TYPE_BOOLEAN;
		default:
			return VALUE_TYPE_UNSUPPORTED;
		}
	}
	int getAttributeIntValue(size_t index) {
		return getAttribute(index).getIntValue();
	}
	std::string getAttributeStringValue(size_t index) {
		return getAttribute(index).getStringValue();
	}
	int next() {
		// Decrement depth if the previous event was "end element".
		if (mCurrentEvent == EVENT_END_ELEMENT) {
			--mDepth;
		}
		// Read events from document, ignoring events that we don't report to caller. Stop at the
		// earliest event which we report to caller.
		while (mXml.hasRemaining()) {
			auto chunk = Chunk::get(mXml);
			if (chunk == std::nullopt) {
				break;
			}
			switch (chunk->getType()) {
			case Chunk::TYPE_STRING_POOL:
				if (mStringPool != std::nullopt) {
					throw xml_parser_exception("Multiple string pools not supported");
				}
				mStringPool = StringPool(chunk.value());
				break;

			case Chunk::RES_XML_TYPE_START_ELEMENT: {
				if (mStringPool == std::nullopt) {
					throw xml_parser_exception("Named element encountered before string pool");
				}
				ByteBuffer contents = chunk->getContents();
				if (contents.remaining() < 20) {
					throw xml_parser_exception("Start element chunk too short. Need at least 20 bytes. Available: "
					                           + std::to_string(contents.remaining()) + " bytes");
				}
				size_t nsId = contents.getUnsignedInt32();
				size_t nameId = contents.getUnsignedInt32();
				size_t attrStartOffset = contents.getUnsignedInt16();
				size_t attrSizeBytes = contents.getUnsignedInt16();
				size_t attrCount = contents.getUnsignedInt16();
				size_t attrEndOffset = attrStartOffset + attrCount * attrSizeBytes;
				contents.setPosition(0);
				if (attrStartOffset > contents.remaining()) {
					throw xml_parser_exception("Attributes start offset out of bounds: "
					                           + std::to_string(attrStartOffset) + ", max: "
					                           + std::to_string(contents.remaining()));
				}
				if (attrEndOffset > contents.remaining()) {
					throw xml_parser_exception("Attributes end offset out of bounds: "
					                           + std::to_string(attrEndOffset) + ", max: "
					                           + std::to_string(contents.remaining()));
				}
				mCurrentElementName = mStringPool->getString(nameId);
				mCurrentElementNamespace = (nsId == NO_NAMESPACE) ? "" : mStringPool->getString(nsId);
				mCurrentElementAttributeCount = attrCount;
				mCurrentElementAttributes.clear();
				mCurrentElementAttrSizeBytes = attrSizeBytes;
				mCurrentElementAttributesContents = {contents, attrStartOffset, attrEndOffset};
				++mDepth;
				mCurrentEvent = EVENT_START_ELEMENT;
				return mCurrentEvent;
			}

			case Chunk::RES_XML_TYPE_END_ELEMENT: {
				if (mStringPool == std::nullopt) {
					throw xml_parser_exception("Named element encountered before string pool");
				}
				ByteBuffer contents = chunk->getContents();
				if (contents.remaining() < 8) {
					throw xml_parser_exception("End element chunk too short. Need at least 8 bytes. Available: "
					                           + std::to_string(contents.remaining()) + " bytes");
				}
				size_t nsId = contents.getUnsignedInt32();
				size_t nameId = contents.getUnsignedInt32();
				mCurrentElementName = mStringPool->getString(nameId);
				mCurrentElementNamespace = (nsId == NO_NAMESPACE) ? "" : mStringPool->getString(nsId);
				mCurrentEvent = EVENT_END_ELEMENT;
				mCurrentElementAttributes.clear();
				mCurrentElementAttributesContents = {};
				return mCurrentEvent;
			}

			case Chunk::RES_XML_TYPE_RESOURCE_MAP:
				if (mResourceMap != std::nullopt) {
					throw xml_parser_exception("Multiple resource maps not supported");
				}
				mResourceMap = ResourceMap(chunk.value());
				break;

			default:
				// Unknown chunk type -- ignore
				break;
			}
		}
		mCurrentEvent = EVENT_END_DOCUMENT;
		return mCurrentEvent;
	}
};

}  // namespace apksigner

#endif
