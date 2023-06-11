#ifndef ZIPUTILS_HPP
#define ZIPUTILS_HPP

#include "ByteBuffer.hpp"
#include "CentralDirectoryRecord.hpp"
#include "ZipSections.hpp"
#include "../utils.hpp"   // endsWith

#include <algorithm>      // std::min
#include <climits>        // INT32_MAX
#include <cstddef>        // std::size_t
#include <list>
#include <optional>
#include <stdexcept>      // std::invalid_argument
#include <utility>        // std::pair

namespace apksigner {
	
/**
 * Indicates that an APK is not well-formed. For example, this may indicate that the APK is not
 * a well-formed ZIP archive, or that the APK contains multiple ZIP entries with the same name.
 */
class apk_format_exception : public std::exception {
	public:
		explicit apk_format_exception(const std::string &message = "apk_format_exception")
			: std::exception(), error_message(message) {}
		const char *what() const noexcept
		{
			return error_message.c_str();
		}
	private:
		std::string error_message;
};

/**
 * Indicates that a ZIP archive is not well-formed.
 */
class zip_format_exception : public std::exception {
	public:
		explicit zip_format_exception(const std::string &message = "zip_format_exception")
			: std::exception(), error_message(message) {}
		const char *what() const noexcept
		{
			return error_message.c_str();
		}
	private:
		std::string error_message;
};


/**
 * Assorted ZIP format helpers.
 *
 * NOTE: Most helper methods operating on \ref ByteBuffer instances expect that the byte
 * order of these buffers is little-endian.
 */
class ZipUtils {
public:
	constexpr static short COMPRESSION_METHOD_STORED = 0;
	constexpr static short COMPRESSION_METHOD_DEFLATED = 8;
	
	constexpr static short GP_FLAG_DATA_DESCRIPTOR_USED = 0x08;
	constexpr static short GP_FLAG_EFS = 0x0800;
	
	/**
	 * Sets the offset of the start of the ZIP Central Directory in the archive.
	 *
	 * NOTE: Byte order of \p zipEndOfCentralDirectory must be little-endian.
	 */
	static void setZipEocdCentralDirectoryOffset(
			ByteBuffer& zipEndOfCentralDirectory, long offset) {
		
		setUnsignedInt32(
			zipEndOfCentralDirectory,
			zipEndOfCentralDirectory.getPosition() + ZIP_EOCD_CENTRAL_DIR_OFFSET_FIELD_OFFSET,
			offset);
	}
	
	/**
	 * Sets the length of EOCD comment.
	 *
	 * NOTE: Byte order of \p zipEndOfCentralDirectory must be little-endian.
	 */
	static void updateZipEocdCommentLen(ByteBuffer& zipEndOfCentralDirectory) {
		int commentLen = zipEndOfCentralDirectory.remaining() - ZIP_EOCD_REC_MIN_SIZE;
		setUnsignedInt16(
		                 zipEndOfCentralDirectory,
		                 zipEndOfCentralDirectory.getPosition() + ZIP_EOCD_COMMENT_LENGTH_FIELD_OFFSET,
		                 commentLen);
	}
	
	/**
	 * Returns the offset of the start of the ZIP Central Directory in the archive.
	 *
	 * NOTE: Byte order of \p zipEndOfCentralDirectory must be little-endian.
	 */
	static long getZipEocdCentralDirectoryOffset(ByteBuffer& zipEndOfCentralDirectory) {
		return getUnsignedInt32(
		                        zipEndOfCentralDirectory,
		                        zipEndOfCentralDirectory.getPosition() + ZIP_EOCD_CENTRAL_DIR_OFFSET_FIELD_OFFSET);
	}
	
	/**
	 * Returns the size (in bytes) of the ZIP Central Directory.
	 *
	 * «NOTE: Byte order of \p zipEndOfCentralDirectory must be little-endian.
	 */
	static long getZipEocdCentralDirectorySizeBytes(ByteBuffer zipEndOfCentralDirectory) {
		return getUnsignedInt32(
		                        zipEndOfCentralDirectory,
		                        zipEndOfCentralDirectory.getPosition() + ZIP_EOCD_CENTRAL_DIR_SIZE_FIELD_OFFSET);
	}
	
	/**
	 * Returns the total number of records in ZIP Central Directory.
	 *
	 * NOTE: Byte order of \p zipEndOfCentralDirectory must be little-endian.
	 */
	static int getZipEocdCentralDirectoryTotalRecordCount(
	        ByteBuffer zipEndOfCentralDirectory) {
	    return getUnsignedInt16(
	            zipEndOfCentralDirectory,
	            zipEndOfCentralDirectory.getPosition()
	                    + ZIP_EOCD_CENTRAL_DIR_TOTAL_RECORD_COUNT_OFFSET);
	}
	
	/**
	 * Returns the ZIP End of Central Directory record of the provided ZIP file.
	 *
	 * \return contents of the ZIP End of Central Directory record and the record's offset in the
	 *         file or \c std::nullopt if the file does not contain the record.
	 */
	static std::optional<std::pair<ByteBuffer, size_t>>
	findZipEndOfCentralDirectoryRecord(ByteBuffer& zip) {
		// ZIP End of Central Directory (EOCD) record is located at the very end of the ZIP archive.
		// The record can be identified by its 4-byte signature/magic which is located at the very
		// beginning of the record. A complication is that the record is variable-length because of
		// the comment field.
		// The algorithm for locating the ZIP EOCD record is as follows. We search backwards from
		// end of the buffer for the EOCD record signature. Whenever we find a signature, we check
		// the candidate record's comment length is such that the remainder of the record takes up
		// exactly the remaining bytes in the buffer. The search is bounded because the maximum
		// size of the comment field is 65535 bytes because the field is an unsigned 16-bit number.
	
		long fileSize = zip.getSize();
		if (fileSize < ZIP_EOCD_REC_MIN_SIZE) {
			return std::nullopt;
		}
	
		// Optimization: 99.99% of APKs have a zero-length comment field in the EoCD record and thus
		// the EoCD record offset is known in advance. Try that offset first to avoid unnecessarily
		// reading more data.
		std::optional<std::pair<ByteBuffer, size_t>> result = findZipEndOfCentralDirectoryRecord(zip, 0);
		if (result != std::nullopt) {
			return result;
		}
	
		// EoCD does not start where we expected it to. Perhaps it contains a non-empty comment
		// field. Expand the search. The maximum size of the comment field in EoCD is 65535 because
		// the comment length field is an unsigned 16-bit number.
		return findZipEndOfCentralDirectoryRecord(zip, UINT16_MAX_VALUE);
	}
	
	static void setUnsignedInt32(ByteBuffer& buffer, int offset, long value) {
		if ((value < 0) || (value > 0xffffffffL)) {
			throw std::invalid_argument(std::string("uint32 value of out range: ") + std::to_string(value));
		}
		buffer.putUnsignedInt32(offset, static_cast<uint32_t>(value));
	}
	
	static void setUnsignedInt16(ByteBuffer& buffer, int offset, int value) {
		if ((value < 0) || (value > 0xffff)) {
			throw std::invalid_argument(std::string("uint16 value of out range: ") + std::to_string(value));
		}
		buffer.putUnsignedInt16(offset, static_cast<uint16_t>(value));
	}
	
	static void putUnsignedInt32(ByteBuffer& buffer, long value) {
		if ((value < 0) || (value > 0xffffffffL)) {
			throw std::invalid_argument(std::string("uint32 value of out range: ") + std::to_string(value));
		}
		buffer.putUnsignedInt32(static_cast<uint32_t>(value));
	}
	
	static void putUnsignedInt16(ByteBuffer& buffer, int value) {
		if ((value < 0) || (value > 0xffff)) {
			throw std::invalid_argument(std::string("uint16 value of out range: ") + std::to_string(value));
		}
		buffer.putUnsignedInt16(static_cast<uint16_t>(value));
	}
	
	static std::list<CentralDirectoryRecord> parseZipCentralDirectory(
	                                                             ByteBuffer& apk,
	                                                             ZipSections& apkSections) {
		// Read the ZIP Central Directory
		size_t cdSizeBytes = apkSections.getZipCentralDirectorySizeBytes();
		if (cdSizeBytes > INT32_MAX) {
			throw apk_format_exception(std::string("ZIP Central Directory too large: ") + std::to_string(cdSizeBytes));
		}
		size_t cdOffset = apkSections.getZipCentralDirectoryOffset();
		ByteBuffer cd{ ByteBuffer(apk, cdOffset, cdSizeBytes) };
		
		// Parse the ZIP Central Directory
		size_t expectedCdRecordCount = apkSections.getZipCentralDirectoryRecordCount();
		std::list<CentralDirectoryRecord> cdRecords{ expectedCdRecordCount };
		for (int i = 0; i < expectedCdRecordCount; i++) {
			int offsetInsideCd = cd.getPosition();
			try {
				CentralDirectoryRecord cdRecord = CentralDirectoryRecord::getRecord(cd);
				std::string entryName = cdRecord.getName();
				if (endsWith(entryName, "/")) {
					// Ignore directory entries
					continue;
				}
				cdRecords.push_back(cdRecord);
			} catch (zip_format_exception e) {
				throw apk_format_exception(
				                           std::string("Malformed ZIP Central Directory record #")
				                           + std::to_string(i + 1)
				                           + std::string(" at file offset ")
				                           + std::to_string(static_cast<int>(cdOffset) + offsetInsideCd)
				                           + "\n" + std::string(e.what()));
			}
		}
		// There may be more data in Central Directory, but we don't warn or throw because Android
		// ignores unused CD data.
		
		return cdRecords;
	}
	
	static size_t getUnsignedInt32(ByteBuffer& buffer, int offset) {
		return buffer.getUnsignedInt32(offset) & 0xffffffffL;
	}
	
	static size_t getUnsignedInt32(ByteBuffer& buffer) {
		return buffer.getUnsignedInt32() & 0xffffffffL;
	}
	
	static int getUnsignedInt16(ByteBuffer& buffer, int offset) {
		return buffer.getUnsignedInt16(offset) & 0xffff;
	}
	
	static int getUnsignedInt16(ByteBuffer& buffer) {
		return buffer.getUnsignedInt16() & 0xffff;
	}
	
	
private:
	ZipUtils() {}
	constexpr static int ZIP_EOCD_REC_MIN_SIZE = 22;
	constexpr static int ZIP_EOCD_REC_SIG = 0x06054b50;
	constexpr static int ZIP_EOCD_CENTRAL_DIR_TOTAL_RECORD_COUNT_OFFSET = 10;
	constexpr static int ZIP_EOCD_CENTRAL_DIR_SIZE_FIELD_OFFSET = 12;
	constexpr static int ZIP_EOCD_CENTRAL_DIR_OFFSET_FIELD_OFFSET = 16;
	constexpr static int ZIP_EOCD_COMMENT_LENGTH_FIELD_OFFSET = 20;
	
	constexpr static int UINT16_MAX_VALUE = 0xffff;
	
	/**
	 * Returns the ZIP End of Central Directory record of the provided ZIP file.
	 *
	 * \param maxCommentSize maximum accepted size (in bytes) of EoCD comment field. The permitted
	 *        value is from 0 to 65535 inclusive. The smaller the value, the faster this method
	 *        locates the record, provided its comment field is no longer than this value.
	 *
	 * \return contents of the ZIP End of Central Directory record and the record's offset in the
	 *         file or \c null if the file does not contain the record.
	 */
	static std::optional<std::pair<ByteBuffer, size_t>>
		findZipEndOfCentralDirectoryRecord(ByteBuffer& zip, int maxCommentSize) {
		
		// ZIP End of Central Directory (EOCD) record is located at the very end of the ZIP archive.
		// The record can be identified by its 4-byte signature/magic which is located at the very
		// beginning of the record. A complication is that the record is variable-length because of
		// the comment field.
		// The algorithm for locating the ZIP EOCD record is as follows. We search backwards from
		// end of the buffer for the EOCD record signature. Whenever we find a signature, we check
		// the candidate record's comment length is such that the remainder of the record takes up
		// exactly the remaining bytes in the buffer. The search is bounded because the maximum
		// size of the comment field is 65535 bytes because the field is an unsigned 16-bit number.
		
		if ((maxCommentSize < 0) || (maxCommentSize > UINT16_MAX_VALUE)) {
			throw std::invalid_argument("maxCommentSize: " + maxCommentSize);
		}
		
		size_t fileSize = zip.getSize();
		if (fileSize < ZIP_EOCD_REC_MIN_SIZE) {
			// No space for EoCD record in the file.
			return std::nullopt;
		}
		// Lower maxCommentSize if the file is too small.
		maxCommentSize = static_cast<int>(std::min(static_cast<size_t>(maxCommentSize),
		    fileSize - static_cast<size_t>(ZIP_EOCD_REC_MIN_SIZE)));
		
		size_t maxEocdSize = ZIP_EOCD_REC_MIN_SIZE + maxCommentSize;
		size_t bufOffsetInFile = fileSize - static_cast<size_t>(maxEocdSize);
		ByteBuffer buf = ByteBuffer(zip, bufOffsetInFile, maxEocdSize);

		int eocdOffsetInBuf = findZipEndOfCentralDirectoryRecordPos(buf);
		if (eocdOffsetInBuf == -1) {
			// No EoCD record found in the buffer
			return std::nullopt;
		}
		// EoCD found
		buf.setPosition(eocdOffsetInBuf);
		ByteBuffer eocd{ buf, static_cast<std::size_t>(eocdOffsetInBuf), buf.getSize() };

		return std::make_pair(eocd, bufOffsetInFile + static_cast<size_t>(eocdOffsetInBuf));
	}
	
	/**
	 * Returns the position at which ZIP End of Central Directory record starts in the provided
	 * buffer or -1 if the record is not present.
	 *
	 * NOTE: Byte order of \p zipContents must be little-endian.
	 */
	static int findZipEndOfCentralDirectoryRecordPos(ByteBuffer& zipContents) {
	
		// ZIP End of Central Directory (EOCD) record is located at the very end of the ZIP archive.
		// The record can be identified by its 4-byte signature/magic which is located at the very
		// beginning of the record. A complication is that the record is variable-length because of
		// the comment field.
		// The algorithm for locating the ZIP EOCD record is as follows. We search backwards from
		// end of the buffer for the EOCD record signature. Whenever we find a signature, we check
		// the candidate record's comment length is such that the remainder of the record takes up
		// exactly the remaining bytes in the buffer. The search is bounded because the maximum
		// size of the comment field is 65535 bytes because the field is an unsigned 16-bit number.
		
		int archiveSize = zipContents.getBuffer().capacity();
		if (archiveSize < ZIP_EOCD_REC_MIN_SIZE) {
			return -1;
		}
		int maxCommentLength = std::min(archiveSize - ZIP_EOCD_REC_MIN_SIZE, UINT16_MAX_VALUE);
		int eocdWithEmptyCommentStartPosition = archiveSize - ZIP_EOCD_REC_MIN_SIZE;
		for (int expectedCommentLength = 0; expectedCommentLength <= maxCommentLength;
		     expectedCommentLength++) {
			
			int eocdStartPos = eocdWithEmptyCommentStartPosition - expectedCommentLength;
			if (zipContents.getUnsignedInt32(eocdStartPos) == ZIP_EOCD_REC_SIG) {
				int actualCommentLength =
					getUnsignedInt16(
					                 zipContents, eocdStartPos + ZIP_EOCD_COMMENT_LENGTH_FIELD_OFFSET);
				if (actualCommentLength == expectedCommentLength) {
					return eocdStartPos;
				}
			}
		}
		
		return -1;
	}
	
};

}  // namespace apksigner

#endif
