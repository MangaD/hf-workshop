#ifndef ZIPSECTIONS_HPP
#define ZIPSECTIONS_HPP

#include "ByteBuffer.hpp"

#include <cstddef>        // std::size_t

namespace apksigner {

class ZipSections {
	
public:
	ZipSections(
			std::size_t centralDirectoryOffset,
			std::size_t centralDirectorySizeBytes,
			int centralDirectoryRecordCount,
			std::size_t eocdOffset,
			ByteBuffer eocd) {
		mCentralDirectoryOffset = centralDirectoryOffset;
		mCentralDirectorySizeBytes = centralDirectorySizeBytes;
		mCentralDirectoryRecordCount = centralDirectoryRecordCount;
		mEocdOffset = eocdOffset;
		mEocd = eocd;
	}
	
	/**
	 * Returns the start offset of the ZIP Central Directory. This value is taken from the
	 * ZIP End of Central Directory record.
	 */
	std::size_t getZipCentralDirectoryOffset() {
		return mCentralDirectoryOffset;
	}
	
	/**
	 * Returns the size (in bytes) of the ZIP Central Directory. This value is taken from the
	 * ZIP End of Central Directory record.
	 */
	std::size_t getZipCentralDirectorySizeBytes() {
		return mCentralDirectorySizeBytes;
	}
	
	/**
	 * Returns the number of records in the ZIP Central Directory. This value is taken from the
	 * ZIP End of Central Directory record.
	 */
	int getZipCentralDirectoryRecordCount() {
		return mCentralDirectoryRecordCount;
	}
	
	/**
	 * Returns the start offset of the ZIP End of Central Directory record. The record extends
	 * until the very end of the APK.
	 */
	std::size_t getZipEndOfCentralDirectoryOffset() {
		return mEocdOffset;
	}
	
	/**
	 * Returns the contents of the ZIP End of Central Directory.
	 */
	ByteBuffer getZipEndOfCentralDirectory() {
		return mEocd;
	}
	
private:
	std::size_t mCentralDirectoryOffset;
	std::size_t mCentralDirectorySizeBytes;
	int mCentralDirectoryRecordCount;
	std::size_t mEocdOffset;
	ByteBuffer mEocd;
};

}  // namespace apksigner

#endif
