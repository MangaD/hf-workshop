/**
 * C++ Wrapper to minizip library
 */


#ifndef MINIZIP_WRAPPER_HPP
#define MINIZIP_WRAPPER_HPP

// Unzipper
#include <vector>
#include <string>
#include <cstdint>   // uint8_t
#include <ctime>     // time_t
#include <exception> // exception

#include <minizip/zip.h>
#include <minizip/unzip.h>

/**
 * Tutorials:
 *     - https://stackoverflow.com/questions/10440113/simple-way-to-unzip-a-zip-file-using-zlib
 *     - https://stackoverflow.com/questions/4696907/zlib-c-and-extracting-files
 *     - https://github.com/sebastiandev/zipper
 *     - https://nachtimwald.com/2019/09/08/making-minizip-easier-to-use/
 *
 * References:
 *     - https://github.com/luvit/zlib/blob/master/contrib/minizip/zip.h
 *     - https://github.com/luvit/zlib/blob/master/contrib/minizip/unzip.h
 */

namespace minizip {

	class minizip_exception : public std::exception {
		public:
			explicit minizip_exception(const std::string &message = "minizip_exception")
				: std::exception(), error_message(message) {}
			const char *what() const noexcept
			{
				return error_message.c_str();
			}
		private:
			std::string error_message;
	};

	// Not defined in windows version for some reason...
#ifdef _WIN32
	typedef const char* zipcharpc;
	typedef struct tm_unz_s
	{
	    uInt tm_sec;            /* seconds after the minute - [0,59] */
	    uInt tm_min;            /* minutes after the hour - [0,59] */
	    uInt tm_hour;           /* hours since midnight - [0,23] */
	    uInt tm_mday;           /* day of the month - [1,31] */
	    uInt tm_mon;            /* months since January - [0,11] */
	    uInt tm_year;           /* years - [1980..2044] */
	} tm_unz;
#endif

	class ZipEntry {
	public:
		inline ZipEntry(const std::string & _name, const std::string & _comment, size_t _compressed_size,
				size_t _uncompressed_size, size_t _dosDate, size_t _compression_method,
				size_t _version, size_t _version_needed, tm_unz _tmu_date,
				bool _isDirectory, size_t _file_offset)
			: name(_name), timestamp(), comment(_comment), compressed_size(_compressed_size),
			  uncompressed_size(_uncompressed_size), dosDate(_dosDate),
			  compression_method(_compression_method), version(_version),
			  version_needed(_version_needed), tmu_date(_tmu_date),
			  isDirectory(_isDirectory), file_offset(_file_offset)
		{
			// timestamp YYYY-MM-DD HH:MM:SS
			timestamp = std::to_string(tmu_date.tm_year) + "-"
						+ std::to_string(tmu_date.tm_mon+1) + "-" + std::to_string(tmu_date.tm_mday) + " " +
						std::to_string(tmu_date.tm_hour) + ":" + std::to_string(tmu_date.tm_min) + ":" +
						std::to_string(tmu_date.tm_sec);
		}

		inline bool valid() { return !name.empty(); }
		inline bool isCompressed() { return this->compression_method != 0; }

		std::string name, timestamp, comment;
		size_t compressed_size, uncompressed_size;
		size_t dosDate;
		/**
		 *  0 - The file is stored (no compression)
		 *  1 - The file is Shrunk
		 *  2 - The file is Reduced with compression factor 1
		 *  3 - The file is Reduced with compression factor 2
		 *  4 - The file is Reduced with compression factor 3
		 *  5 - The file is Reduced with compression factor 4
		 *  6 - The file is Imploded
		 *  7 - Reserved for Tokenizing compression algorithm
		 *  8 - The file is Deflated
		 *  9 - Enhanced Deflating using Deflate64(tm)
		 * 10 - PKWARE Data Compression Library Imploding (old IBM TERSE)
		 * 11 - Reserved by PKWARE
		 * 12 - File is compressed using BZIP2 algorithm
		 * 13 - Reserved by PKWARE
		 * 14 - LZMA (EFS)
		 * 15 - Reserved by PKWARE
		 * 16 - Reserved by PKWARE
		 * 17 - Reserved by PKWARE
		 * 18 - File is compressed using IBM TERSE (new)
		 * 19 - IBM LZ77 z Architecture (PFS)
		 * 97 - WavPack compressed data
		 * 98 - PPMd version I, Rev 1
		 */
		size_t compression_method;
		size_t version;// version made by
		size_t version_needed;// version needed to extract
		tm_unz tmu_date;
		bool isDirectory;
		size_t file_offset;
		// Zip extra field is skipped because I don't need it
	};

	class Unzipper {
	public:
		explicit Unzipper(const std::string & filename);
		~Unzipper();
		bool hasEntry(const std::string& name);
		void extractEntryToMemory(const std::string& name, std::vector<uint8_t>& vec);
		std::vector<ZipEntry> getEntries();
		inline uint64_t getNoEntries() { return this->number_entry; }
		inline std::string getGlobalComment() { return this->globalComment; }
		void close();

		Unzipper(const Unzipper &rhs) = delete; //-Weffc++
		Unzipper(Unzipper &&rhs);//= default;
		Unzipper &operator=(const Unzipper &rhs) = delete; //-Weffc++
	private:
		std::vector<uint8_t> zippedBuffer;
		unzFile zipfile;
		uint64_t number_entry;
		std::string filename;
		std::string globalComment;
	};

	class Zipper {
	public:
		Zipper(const std::string & filename, const std::string & comment);
		~Zipper();
		// Z_BEST_COMPRESSION
		void add(const std::string& name, const std::string& comment, int compressionLevel, std::string& s);
		void add(const std::string& name, const std::string& comment, int compressionLevel, std::vector<uint8_t>& vec);
		void add(const std::string& name, const std::string& comment, int compressionLevel, const void* buf, size_t bufLen);
		void close();

		Zipper(const Zipper &rhs) = delete; //-Weffc++
		Zipper(Zipper &&rhs);//= default;
		Zipper &operator=(const Zipper &rhs) = delete; //-Weffc++
	private:
		std::string filename;
		std::string global_comment;
		zipFile zipfile;
	};

	uint32_t dostime(int year, int month, int day, int hour, int minute, int second);
	uint32_t unix2dostime(time_t unix_time);
	std::time_t dos2unixtime(uint32_t dostime);


} // minizip

#endif // MINIZIP_WRAPPER_HPP
