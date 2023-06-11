#ifndef CENTRALDIRECTORYRECORD_HPP
#define CENTRALDIRECTORYRECORD_HPP

#include "ByteBuffer.hpp"
#include "../utils.hpp"   // int_to_hex
#include "ZipUtils.hpp"

#include <cstddef>        // std::size_t
#include <stdexcept>      // std::runtime_error
#include <string>

namespace apksigner {
	
/**
 * ZIP Central Directory (CD) Record.
 */
class CentralDirectoryRecord {

public:
	
	/**
	 * Comparator which compares records by the offset of the corresponding Local File Header in the
	 * archive.
	 */
	static bool byLocalFileHeaderOffsetComparator(CentralDirectoryRecord& r1,
	                                              CentralDirectoryRecord& r2) {
		
		std::size_t offset1 = r1.getLocalFileHeaderOffset();
		std::size_t offset2 = r2.getLocalFileHeaderOffset();
		if (offset1 < offset2) {
			return true;
		}
		return false;
	}

	std::size_t getSize() {
		return mData.remaining();
	}

	std::string getName() {
		return mName;
	}

	std::size_t getNameSizeBytes() {
		return mNameSizeBytes;
	}

	short getGpFlags() {
		return mGpFlags;
	}

	short getCompressionMethod() {
		return mCompressionMethod;
	}

	int getLastModificationTime() {
		return mLastModificationTime;
	}

	int getLastModificationDate() {
		return mLastModificationDate;
	}

	long getCrc32() {
		return mCrc32;
	}

	std::size_t getCompressedSize() {
		return mCompressedSize;
	}

	std::size_t getUncompressedSize() {
		return mUncompressedSize;
	}

	std::size_t getLocalFileHeaderOffset() {
		return mLocalFileHeaderOffset;
	}

	/**
	 * Returns the Central Directory Record starting at the current position of the provided buffer
	 * and advances the buffer's position immediately past the end of the record.
	 */
	static CentralDirectoryRecord getRecord(ByteBuffer& buf) {
		
		if (buf.remaining() < HEADER_SIZE_BYTES) {
			throw zip_format_exception(std::string("Input too short. Need at least: ")
			                           + std::to_string(HEADER_SIZE_BYTES)
			                           + std::string(" bytes, available: ")
			                           + std::to_string(buf.remaining())
			                           + std::string(" bytes"));
		}
		int originalPosition = buf.getPosition();
		int recordSignature = buf.getUnsignedInt32();
		if (recordSignature != RECORD_SIGNATURE) {
			throw zip_format_exception(
			                           std::string("Not a Central Directory record. Signature: 0x")
			                           + int_to_hex(recordSignature & 0xffffffffL));
		}
		buf.setPosition(originalPosition + GP_FLAGS_OFFSET);
		short gpFlags = buf.getUnsignedInt16();
		short compressionMethod = buf.getUnsignedInt16();
		int lastModificationTime = ZipUtils::getUnsignedInt16(buf);
		int lastModificationDate = ZipUtils::getUnsignedInt16(buf);
		long crc32 = ZipUtils::getUnsignedInt32(buf);
		long compressedSize = ZipUtils::getUnsignedInt32(buf);
		long uncompressedSize = ZipUtils::getUnsignedInt32(buf);
		int nameSize = ZipUtils::getUnsignedInt16(buf);
		int extraSize = ZipUtils::getUnsignedInt16(buf);
		int commentSize = ZipUtils::getUnsignedInt16(buf);
		buf.setPosition(originalPosition + LOCAL_FILE_HEADER_OFFSET_OFFSET);
		long localFileHeaderOffset = ZipUtils::getUnsignedInt32(buf);
		buf.setPosition(originalPosition);
		int recordSize = HEADER_SIZE_BYTES + nameSize + extraSize + commentSize;
		if (recordSize > buf.remaining()) {
			throw zip_format_exception(
					std::string("Input too short. Need: ")
					+ std::to_string(recordSize)
					+ std::string(" bytes, available: ")
					+ std::to_string(buf.remaining())
					+ std::string(" bytes"));
		}
		std::string name = getName(buf, originalPosition + NAME_OFFSET, nameSize);
		buf.setPosition(originalPosition);
		int recordEndInBuf = originalPosition + recordSize;
		ByteBuffer recordBuf{ buf, buf.getPosition(), buf.getPosition() + recordEndInBuf };
		
		// Consume this record
		buf.setPosition(recordEndInBuf);
		return CentralDirectoryRecord(
				recordBuf,
				gpFlags,
				compressionMethod,
				lastModificationTime,
				lastModificationDate,
				crc32,
				compressedSize,
				uncompressedSize,
				localFileHeaderOffset,
				name,
				nameSize);
	}

	void copyTo(ByteBuffer& output) {
		ByteBuffer tmpBuf{ mData, mData.getPosition(), mData.getSize() };
		output.put(tmpBuf);
	}

	CentralDirectoryRecord createWithModifiedLocalFileHeaderOffset(
			std::size_t localFileHeaderOffset) {
		ByteBuffer result{ mData, mData.getPosition(), mData.getPosition() + mData.remaining() };
		ZipUtils::setUnsignedInt32(result, LOCAL_FILE_HEADER_OFFSET_OFFSET, localFileHeaderOffset);
		return CentralDirectoryRecord {
				result,
				mGpFlags,
				mCompressionMethod,
				mLastModificationTime,
				mLastModificationDate,
				mCrc32,
				mCompressedSize,
				mUncompressedSize,
				localFileHeaderOffset,
				mName,
				mNameSizeBytes };
	}

	static CentralDirectoryRecord createWithDeflateCompressedData(
			std::string name,
			int lastModifiedTime,
			int lastModifiedDate,
			long crc32,
			long compressedSize,
			long uncompressedSize,
			long localFileHeaderOffset) {
		
		short gpFlags = ZipUtils::GP_FLAG_EFS; // UTF-8 character encoding used for entry name
		short compressionMethod = ZipUtils::COMPRESSION_METHOD_DEFLATED;
		size_t recordSize = HEADER_SIZE_BYTES + name.size();
		ByteBuffer result{ recordSize };
		result.putUnsignedInt32(RECORD_SIGNATURE);
		ZipUtils::putUnsignedInt16(result, 0x14); // Version made by
		ZipUtils::putUnsignedInt16(result, 0x14); // Minimum version needed to extract
		result.putUnsignedInt16(gpFlags);
		result.putUnsignedInt16(compressionMethod);
		ZipUtils::putUnsignedInt16(result, lastModifiedTime);
		ZipUtils::putUnsignedInt16(result, lastModifiedDate);
		ZipUtils::putUnsignedInt32(result, crc32);
		ZipUtils::putUnsignedInt32(result, compressedSize);
		ZipUtils::putUnsignedInt32(result, uncompressedSize);
		ZipUtils::putUnsignedInt16(result, name.size());
		ZipUtils::putUnsignedInt16(result, 0); // Extra field length
		ZipUtils::putUnsignedInt16(result, 0); // File comment length
		ZipUtils::putUnsignedInt16(result, 0); // Disk number
		ZipUtils::putUnsignedInt16(result, 0); // Internal file attributes
		ZipUtils::putUnsignedInt32(result, 0); // External file attributes
		ZipUtils::putUnsignedInt32(result, localFileHeaderOffset);
		result.put(reinterpret_cast<uint8_t*>(name.data()), 0L, name.size());

		if (result.hasRemaining()) {
			throw std::runtime_error(std::string("pos: ")
			                         + std::to_string(result.getPosition())
			                         + std::string(", size: ")
			                         + std::to_string(result.getSize()));
		}
		result.setPosition(0);
		return CentralDirectoryRecord(
				result,
				gpFlags,
				compressionMethod,
				lastModifiedTime,
				lastModifiedDate,
				crc32,
				compressedSize,
				uncompressedSize,
				localFileHeaderOffset,
				name,
				name.size());
	}

private:

	static std::string getName(ByteBuffer& record, int position, size_t nameLengthBytes) {
		return { reinterpret_cast<const char*>(record.getBuffer().data()) + position, nameLengthBytes };
	}
	
	static constexpr int RECORD_SIGNATURE = 0x02014b50;
	static constexpr size_t HEADER_SIZE_BYTES = 46;
	
	static constexpr size_t GP_FLAGS_OFFSET = 8;
	static constexpr size_t LOCAL_FILE_HEADER_OFFSET_OFFSET = 42;
	static constexpr size_t NAME_OFFSET = HEADER_SIZE_BYTES;
	
	const ByteBuffer mData;
	const short mGpFlags = 0;
	const short mCompressionMethod = 0;
	const int mLastModificationTime = 0;
	const int mLastModificationDate = 0;
	const long mCrc32 = 0;
	const size_t mCompressedSize = 0;
	const size_t mUncompressedSize = 0;
	const size_t mLocalFileHeaderOffset = 0;
	const std::string mName;
	const int mNameSizeBytes = 0;
	
	CentralDirectoryRecord(
			ByteBuffer& data,
			short gpFlags,
			short compressionMethod,
			int lastModificationTime,
			int lastModificationDate,
			long crc32,
			size_t compressedSize,
			size_t uncompressedSize,
			size_t localFileHeaderOffset,
			std::string name,
			int nameSizeBytes)
		: mData{ data },
		mGpFlags{ gpFlags },
		mCompressionMethod{ compressionMethod },
		mLastModificationDate{ lastModificationDate },
		mLastModificationTime{ lastModificationTime },
		mCrc32{ crc32 },
		mCompressedSize{ compressedSize },
		mUncompressedSize{ uncompressedSize },
		mLocalFileHeaderOffset{ localFileHeaderOffset },
		mName{ name },
		mNameSizeBytes{ nameSizeBytes } { }
};

	
}  // namespace apksigner

#endif
