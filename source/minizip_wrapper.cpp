/**
 * C++ Wrapper to minizip library
 */

#include "minizip_wrapper.hpp"

#include <ctime>      // time_t
#include <cstring>    // memset
#include <stdexcept>
#include <algorithm>  // std::equal

#include <minizip/zip.h>
#include <minizip/unzip.h>

#ifdef _WIN32
#include <minizip/iowin32.h>
#endif

#include <array>
#include <string>      // wstring
#include "utils.hpp"   // s2ws

#ifdef ZIP_DEBUG_BUILD
#include <iostream>
#define ZIP_DEBUG(x) do { std::cout << x << std::endl; } while (0)
#define ZIP_DEBUG_NNL(x) do { std::cout << x; } while (0)
#else
#define ZIP_DEBUG(x) do { } while (0)
#define ZIP_DEBUG_NNL(x) do { } while (0)
#endif

namespace minizip {

	Unzipper::Unzipper(const std::string &_filename)
			: zippedBuffer(), zipfile(nullptr),
			  number_entry(), filename(_filename),
			  globalComment() {
		ZIP_DEBUG("Name: " << this->filename);

		// Open file
	#if defined(_WIN32) && defined(UNICODE)
		std::wstring wfilename = s2ws(filename);
		zlib_filefunc64_def ffunc;
		fill_win32_filefunc64W(&ffunc);
		this->zipfile = unzOpen2_64(wfilename.c_str(), &ffunc);
	#else
		this->zipfile = unzOpen64(filename.c_str());
	#endif
		if (this->zipfile == nullptr) {
			throw minizip_exception("Could not open file '" + filename + "'.");
		}

		// Get zip info
		unz_global_info64 global_info;
		int ret = unzGetGlobalInfo64(this->zipfile, &global_info);
		if (ret != UNZ_OK) {
			this->close();
			throw minizip_exception("Failed to read global info for '" + filename + "'. Error: " + std::to_string(ret));
		}
		this->number_entry = global_info.number_entry;
		uLong size_comment = global_info.size_comment;
		ZIP_DEBUG("\tNo. entries: " << this->getNoEntries());
		ZIP_DEBUG("\tSize of global comment: " << size_comment);
		if (size_comment > 0) {
			std::string s(size_comment, '\0');
			this->globalComment = s;
			ret = unzGetGlobalComment(this->zipfile, &this->globalComment[0],
				static_cast<uint16_t>(this->globalComment.size() + 1));
			if (ret < 0) {
				this->close();
				throw minizip_exception("Failed to read global comment for '" +
					filename + "'. Error: " + std::to_string(ret));
			}
			ZIP_DEBUG("\tGlobal comment: " << this->getGlobalComment());
		}
	}

	Unzipper::~Unzipper() {
		this->close();
	}

	void Unzipper::close() {
		if (this->zipfile != nullptr) {
			unzClose(this->zipfile);
			this->zipfile = nullptr;
		}
	}

	std::vector<ZipEntry> Unzipper::getEntries() {

		std::vector<ZipEntry> entries;

		for (size_t i = 0; i < this->number_entry; ++i) {

			// Get some info about current file.
			unz_file_info64 file_info;
			auto ret = unzGetCurrentFileInfo64(this->zipfile, &file_info, nullptr,
							  0, nullptr, 0, nullptr, 0);
			if (ret != UNZ_OK) {
				this->close();
				throw minizip_exception("Failed to read file info for '" +
					this->filename + "'. Entry no " + std::to_string(i) +
					". Error: " + std::to_string(ret));
			}

			// Get file name and comment of current file.
			std::string ze_name(file_info.size_filename, '\0');
			std::string ze_comment(file_info.size_file_comment, '\0');
			ret = unzGetCurrentFileInfo64(this->zipfile, nullptr, &ze_name[0],
					static_cast<uint16_t>(ze_name.size() + 1),
					nullptr, 0,
					&ze_comment[0],
					static_cast<uint16_t>(ze_comment.size() + 1));
			if (ret != UNZ_OK) {
				this->close();
				throw minizip_exception("Failed to read file info for '" +
					this->filename + "'. Entry no " + std::to_string(i) +
					". Error: " + std::to_string(ret));
			}

			// Get file offset
			size_t file_offset = static_cast<size_t>(unzGetOffset64(this->zipfile));

		#ifdef _WIN32
			auto dosDate = file_info.dosDate;
			time_t unixTime = dos2unixtime(dosDate);
			tm_unz tmu_date;
			if (unixTime >= 0) {
				tm *unixTm = localtime(&unixTime);
				tmu_date.tm_sec = static_cast<uInt>(unixTm->tm_sec);
				tmu_date.tm_min = static_cast<uInt>(unixTm->tm_min);
				tmu_date.tm_hour = static_cast<uInt>(unixTm->tm_hour);
				tmu_date.tm_mday = static_cast<uInt>(unixTm->tm_mday);
				tmu_date.tm_mon = static_cast<uInt>(unixTm->tm_mon);
				tmu_date.tm_year = static_cast<uInt>(unixTm->tm_year)+1900;
			} else {
				tmu_date.tm_sec = static_cast<uInt>(-1);
				tmu_date.tm_min = static_cast<uInt>(-1);
				tmu_date.tm_hour = static_cast<uInt>(-1);
				tmu_date.tm_mday = static_cast<uInt>(-1);
				tmu_date.tm_mon = static_cast<uInt>(-1);
				tmu_date.tm_year = static_cast<uInt>(-1);
			}
		#else
			auto dosDate = file_info.dosDate;
			tm_unz tmu_date = file_info.tmu_date;
		#endif

			entries.emplace_back(ze_name, ze_comment, file_info.compressed_size,
				 file_info.uncompressed_size, dosDate,
				 file_info.compression_method, file_info.version,
				 file_info.version_needed, tmu_date,
				 (ze_name.back() == '/'), file_offset);

			ZIP_DEBUG("\tEntry name: " << entries.back().name);
			ZIP_DEBUG("\tEntry name size: " << file_info.size_filename);
			ZIP_DEBUG("\tEntry comment: " << entries.back().comment);
			ZIP_DEBUG("\tEntry comment size: " << file_info.size_file_comment);
			ZIP_DEBUG("\tCompressed size: " << entries.back().compressed_size / 1024 << " KB");
			ZIP_DEBUG("\tUncompressed size: " << entries.back().uncompressed_size / 1024 << " KB");
			ZIP_DEBUG("\tDOS Date: " << entries.back().dosDate);
			ZIP_DEBUG("\tCompression method: " << entries.back().compression_method);
			ZIP_DEBUG("\tVersion (made by): " << entries.back().version);
			ZIP_DEBUG("\tVersion (needed): " << entries.back().version_needed);
			ZIP_DEBUG("\tTimestamp: " << entries.back().timestamp);
			ZIP_DEBUG("\tIs directory: " << std::to_string(entries.back().isDirectory));

			if ((i + 1) < this->number_entry) {
				ret = unzGoToNextFile(this->zipfile);
				if (ret != UNZ_OK) {
					this->close();
					throw minizip_exception("Failed to go to next file on '" +
						this->filename + "'. Entry no " + std::to_string(i) +
						". Error: " + std::to_string(ret));
				}
			}
		}

		// Place cursor on first file so we can call this function again.
		int ret = unzGoToFirstFile(this->zipfile);
		if (ret != UNZ_OK) {
			this->close();
			throw minizip_exception("Failed to go to first file on '" +
				this->filename + "'. Error: " + std::to_string(ret));
		}

		return entries;
	}

	bool Unzipper::hasEntry(const std::string& name) {
		/*
		 * Try locate the file szFileName in the zipfile.
		 * For the iCaseSensitivity signification, see unzStringFileNameCompare
		 * return value :
		 * UNZ_OK if the file is found. It becomes the current file.
		 * UNZ_END_OF_LIST_OF_FILE if the file is not found
		 */
		int ret = unzLocateFile(this->zipfile, name.c_str(), 1);
		return ret == UNZ_OK;
	}

	void Unzipper::extractEntryToMemory(const std::string &name, std::vector<uint8_t> &vec) {

		int ret;
		if (!this->hasEntry(name)) {
			throw minizip_exception("File '" + name + "' not found inside '" +
				this->filename + "'.");
		}

		// Get some info about current file.
		unz_file_info64 file_info;
		ret = unzGetCurrentFileInfo64(this->zipfile, &file_info, nullptr,
			0, nullptr, 0, nullptr, 0);
		if (ret != UNZ_OK) {
			this->close();
			throw minizip_exception("Failed to read file info of '" + name + "' for '" +
				this->filename + "'. Error: " + std::to_string(ret));
		}

		ret = unzOpenCurrentFile(zipfile);
		if (ret != UNZ_OK) {
			this->close();
			throw minizip_exception("Failed to open file '" + name +
				"' inside '" + this->filename +
				"'. Error: " + std::to_string(ret));
		}

		vec.resize(static_cast<unsigned int>(file_info.uncompressed_size));
		ret = unzReadCurrentFile(zipfile, vec.data(), static_cast<uInt>(vec.size()));
		if (ret < 0) {
			unzCloseCurrentFile(zipfile);
			this->close();
			throw minizip_exception("Failed to read file '" + name + "' with error " +
			                         std::to_string(ret) + ". Error: " + std::to_string(ret));
		}

		ret = unzCloseCurrentFile(zipfile);
		if (ret == UNZ_CRCERROR) {
			this->close();
			throw minizip_exception("File '" + name + "' was read but the CRC is not good.");
		}
	}

	Zipper::Zipper(const std::string &_filename, const std::string &comment)
			: filename(_filename), global_comment(comment), zipfile() {

		if (filename.empty()) {
			throw minizip_exception("Name of zip file must not be empty.");
		}

		zipcharpc comm = comment.empty() ? comment.c_str() : nullptr;

	#if defined(_WIN32) && defined(UNICODE)
		std::wstring wfilename = s2ws(filename);
		zlib_filefunc64_def ffunc;
		fill_win32_filefunc64W(&ffunc);
		this->zipfile = zipOpen2_64(wfilename.c_str(), APPEND_STATUS_CREATE,
		                            &comm,
		                            &ffunc);
	#else
		this->zipfile = zipOpen2_64(filename.c_str(), APPEND_STATUS_CREATE,
		                            &comm,
		                            nullptr);
	#endif
		if (this->zipfile == nullptr) {
			throw minizip_exception("Could not create '" + filename + "'.");
		}
	}

	void Zipper::add(const std::string &name, const std::string &comment,
			int compressionLevel, std::string &s) {
		this->add(name, comment, compressionLevel, s.data(), s.size());
	}

	void Zipper::add(const std::string &name, const std::string &comment,
			int compressionLevel, std::vector<uint8_t> &vec) {
		this->add(name, comment, compressionLevel, vec.data(), vec.size());
	}

	void Zipper::add(const std::string &name, const std::string &comment,
			int compressionLevel, const void *buf, size_t bufLen) {
		if (name.empty()) {
			throw minizip_exception("Name of file to add in zip '" +
				this->filename + "' must not be empty.");
		}

		time_t now = time(nullptr);

		zip_fileinfo zfi;
	#ifndef _WIN32
		tm *ltm = localtime(&now);
		// range values - https://github.com/danieleggert/minizip/blob/master/minizip/zip.h#L79
		zfi.tmz_date.tm_sec = ltm->tm_sec;
		zfi.tmz_date.tm_min = ltm->tm_min;
		zfi.tmz_date.tm_hour = ltm->tm_hour;
		zfi.tmz_date.tm_mday = ltm->tm_mday;
		zfi.tmz_date.tm_mon = ltm->tm_mon;
		zfi.tmz_date.tm_year = ltm->tm_year;
	#endif

		zfi.dosDate = unix2dostime(now);
		zfi.internal_fa = 0;
		zfi.external_fa = 0;

		int ret = zipOpenNewFileInZip64(this->zipfile, name.c_str(), &zfi,
		                                nullptr, 0, nullptr, 0,
		                                (comment.empty() ? comment.c_str() : nullptr),
		                                Z_DEFLATED, compressionLevel, (bufLen >= 0xffffffff));
		if (ret != ZIP_OK) {
			this->close();
			throw minizip_exception("Failed to open file '" + name +
				"' inside '" + this->filename +
				"'. Error: " + std::to_string(ret));
		}

		ret = zipWriteInFileInZip(this->zipfile, buf, static_cast<uInt>(bufLen));
		if (ret != ZIP_OK) {
			zipCloseFileInZip(this->zipfile);
			this->close();
			throw minizip_exception("Failed to write file '" + name +
				"' inside '" + this->filename +
				"'. Error: " + std::to_string(ret));
		}

		ret = zipCloseFileInZip(this->zipfile);
		if (ret != ZIP_OK) {
			this->close();
			throw minizip_exception("Failed to close file '" + name +
				"' inside '" + this->filename +
				"'. Error: " + std::to_string(ret));
		}
	}

	Zipper::~Zipper() {
		this->close();
	}

	void Zipper::close() {
		if (this->zipfile != nullptr) {
			zipClose(this->zipfile, global_comment.c_str());
			this->zipfile = nullptr;
		}
	}

	// taken from: https://zipios.sourceforge.io/zipios-v2.0/dostime_8c_source.html
	uint32_t dostime(int year, int month, int day, int hour, int minute, int second) {
		if (year < 1980 || year > 2107 || month < 1 || month > 12 ||
			day < 1 || day > 31 || hour < 0 || hour > 23 ||
			minute < 0 || minute > 59 || second < 0 || second > 59) {
			return 0;
		}

		return ((static_cast<uint32_t>(year) - 1980) << 25) | ((static_cast<uint32_t>(month)) << 21) |
			((static_cast<uint32_t>(day)) << 16) | ((static_cast<uint32_t>(hour)) << 11) |
			((static_cast<uint32_t>(minute)) << 5) | ((static_cast<uint32_t>(second)) >> 1); // 1 every other second
	}

	// taken from: https://zipios.sourceforge.io/zipios-v2.0/dostime_8c_source.html
	uint32_t unix2dostime(time_t unix_time) {
		time_t even_time;
		struct tm *s;

		even_time = (unix_time + 1) & ~1; /* Round up to even seconds. */
		s = localtime(&even_time);	/* Use local time since MSDOS does. */
		return dostime(s->tm_year + 1900, s->tm_mon + 1, s->tm_mday,
			s->tm_hour, s->tm_min, s->tm_sec);
	}

	// taken from: https://zipios.sourceforge.io/zipios-v2.0/dostime_8c_source.html
	time_t dos2unixtime(uint32_t dostime) {
		struct tm t; /* argument for mktime() */

		std::memset(&t, 0, sizeof(t));

		t.tm_isdst = -1; /* let mktime() determine if DST is in effect */
		/* Convert DOS time to UNIX time_t format */
		t.tm_sec = ((static_cast<int>(dostime) << 1) & 0x3E);
		t.tm_min = ((static_cast<int>(dostime) >> 5) & 0x3F);
		t.tm_hour = ((static_cast<int>(dostime) >> 11) & 0x1F);
		t.tm_mday = ((static_cast<int>(dostime) >> 16) & 0x1F);
		t.tm_mon = ((static_cast<int>(dostime) >> 21) & 0x0F) - 1;
		t.tm_year = ((static_cast<int>(dostime) >> 25) & 0x7F) + 80;

		if (t.tm_year < 80 || t.tm_year > 207 || t.tm_mon < 0 ||
			t.tm_mon > 11 || t.tm_mday < 1 || t.tm_mday > 31 ||
			t.tm_hour < 0 || t.tm_hour > 23 || t.tm_min < 0 ||
			t.tm_min > 59 || t.tm_sec < 0 || t.tm_sec > 59) {
			return -1;
		}

		// A full round trip between Unix date to DOS and back to Unix works
		// as is (without worry about the current timezone) because the DOS
		// format makes use of localdate() and that's 1 to 1 compatible with
		// mktime() which expects a local date too.
		return mktime(&t);
	}


} // namespace minizip
