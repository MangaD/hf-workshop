/**
 * HF Workshop - IO Wrapper
 */


#include "io_wrapper.hpp"

#include <cstdio>       // fprintf, printf, fopen, fseek...
#include <cstring>      // strerror
#include <cstdarg>      // va_list, va_start, va_end
#include <cerrno>       // errno
#include <system_error> // generic_category
#include <memory>       // unique_ptr
#include "utils.hpp"    // s2ws, ws2s
#include "swf_utils.hpp"    // trim

#include <rlutil/rlutil.h>

#ifdef _WIN32
	#include <Windows.h> // GetCurrentConsoleFontEx, SetCurrentConsoleFontEx
#endif

#ifndef NO_READLINE
/**
 * CAREFUL: GNU Readline uses the GPLv3 license! This has implications in
 *          the rights you have over the code you link it against.
 * 
 * MinGW:
 *
 * 32-bit:
 *   https://packages.msys2.org/package/mingw-w64-i686-readline
 *   https://packages.msys2.org/package/mingw-w64-i686-termcap
 *
 * 64-bit:
 *   https://packages.msys2.org/package/mingw-w64-x86_64-readline
 *   https://packages.msys2.org/package/mingw-w64-x86_64-termcap
 */
/*
#include <readline/readline.h> // readline
#include <readline/history.h> // add_history
*/

/**
 * MinGW:
 *
 * 32-bit:
 *   https://packages.msys2.org/package/mingw-w64-i686-wineditline
 *   
 * 64-bit:
 *   https://packages.msys2.org/package/mingw-w64-x86_64-wineditline
 */
#include <editline/readline.h>
#endif

using namespace std;

namespace l18n {

#ifdef _WIN32
	void localization::saveWinConsoleFont() {
	#if WINVER >= _WIN32_WINNT_VISTA
		GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &this->savedFont);
	#endif
	}
	void localization::changeWinConsoleFont(const wchar_t * wstr, SHORT size) {
	#if WINVER >= _WIN32_WINNT_VISTA
		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize = sizeof(cfi);
		cfi.nFont = 0;
		cfi.dwFontSize.X = 0; // Width of each character in the font
		cfi.dwFontSize.Y = size;// Height
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = FW_NORMAL;
		std::wcscpy(cfi.FaceName, wstr); // Choose your font
		SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
	#endif
	}
	void localization::restoreWinConsoleFont() {
	#if WINVER >= _WIN32_WINNT_VISTA
		SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &this->savedFont);
	#endif
	}
#endif

	void localization::initLocale() {
	#ifndef NO_GETTEXT
		// Setting the i18n environment
		// https://www.labri.fr/perso/fleury/posts/programming/a-quick-gettext-tutorial.html
		// https://mohan43u.wordpress.com/2012/07/02/gnu-gettext-yet-another-tutorial/
		setlocale (LC_ALL, "");
		// Works with "." instead of getenv?
		bindtextdomain ("hfworkshop", "./locale");
		textdomain ("hfworkshop");

		/**
		 * How to blody get Chinese working on Windows
		 * without using wide characters.
		 * https://stackoverflow.com/a/45622802
		 * https://stackoverflow.com/a/35383318
		 *
		 * Note: Don't use `SetConsoleOutputCP(CP_UTF8);`
		 *       as suggested because it doesn't really work
		 *       with Chinese.
		 */
		#ifdef _WIN32
		// Enable buffering to prevent VS from chopping up UTF-8 byte sequences
		setvbuf(stdout, nullptr, _IOFBF, stdoutBuffSize); // XXX If string has more than 1000 bytes, this could go wrong
		setvbuf(stderr, nullptr, _IOFBF, stdoutBuffSize);
		
		// Lastly, the Windows console supports both raster fonts and TrueType fonts.
		// As pointed out by Paul, raster fonts will simply ignore the console's code page.
		// So non-ASCII Unicode characters will only work if the console is set to a
		// TrueType Font. Up until Windows 7, the default is a raster font, so the user
		// will have to change it manually. Luckily, Windows 10 changes the default font
		// to Consolas, so this part of the problem should solve itself with time.
		changeWinConsoleFont(L"Consolas", 16);
		#endif
	#endif // NO_GETTEXT
	}

	/**
	 * I/O operations
	 */
	int printf_error (const char* str, ...) {
		rlutil::saveDefaultColor();
		rlutil::setColor(rlutil::LIGHTRED, std::cerr);

		va_list args;
		va_start(args, str);
		int r = io_vfprintf(stderr, str, args);
		va_end(args);

		fflush(stderr);

		rlutil::resetColor(std::cerr);

		return r;
	}
	int printf_colored (int color, const char* str, ...) {
		rlutil::saveDefaultColor();
		rlutil::setColor(color);

		va_list args;
		va_start(args, str);
		int r = io_vfprintf(stdout, str, args);
		va_end(args);

		fflush(stdout);

		rlutil::resetColor();
		return r;
	}
	int printf_normal (const char* str, ...) {
		va_list args;
		va_start(args, str);
		int r = io_vfprintf(stdout, str, args);
		va_end(args);
		fflush(stdout);
		return r;
	}
	int print2Colors(const char *s, size_t breakAt, int colorA, int colorB) {
		rlutil::saveDefaultColor();
		int count = 0;
		int color = colorA;
		for(size_t i = 0, l = 0; i < strlen(s); ++i, ++l) {
			if(l == breakAt) {
				fflush(stdout); // Because of Windows workaround for Chinese
				color = colorB;
			}
			if (s[i] == '\n') {
				fflush(stdout); // Because of Windows workaround for Chinese
				color = colorA;
				l = 0;
			}
			rlutil::setColor(color);
			if (putchar(s[i]) == EOF) {
				throw std::system_error(ferror(stdout), std::generic_category());
			}
			++count;
		}
		rlutil::resetColor();
		fflush(stdout); // Because of Windows workaround for Chinese
		return count;
	}

	int io_vfprintf(FILE *stream, const char* format, va_list arg) {
	#ifdef _WIN32
		// Allow positional arguments
		#if WINVER >= _WIN32_WINNT_VISTA
		return _vfprintf_p(stream, format, arg);
		#else
		return vfprintf(stream, format, arg);
		#endif

		// Print wide-char
		//return _vfwprintf_p(stream, s2ws(s).c_str(), arg);
	#else
		return vfprintf(stream, format, arg);
	#endif
	}

	/**
	 * Tutorials:
	 * - https://eli.thegreenplace.net/2016/basics-of-using-the-readline-library/
	 * - https://gist.github.com/kristopherjohnson/b31bb38cf6485b83f654
	 * - https://stackoverflow.com/questions/31073999/text-auto-complete-using-tab-in-command-line
	 * - (nice one) https://thoughtbot.com/blog/tab-completion-in-gnu-readline
	 *
	 * Why use a prompt: https://askubuntu.com/questions/861602/readline-when-there-is-output-at-beginning-of-line
	 */
	void readLine(string &out, const string &prompt) {
	#ifndef NO_READLINE
		/* GNU Readline
		#ifdef _WIN32
			char* line;
			// There's a bug with win readline that strings get cut off when too big
			// XXX Doesn't autocomplete unicode filenames on Windows
			if (prompt.size() > 60) {
				printf_normal((prompt + "\n").c_str());
				line = readline("");
			} else {
				line = readline(prompt.c_str());
			}
		#else
		*/
			char* line = readline(prompt.c_str());
		//#endif
		if (line) {
			out = string(line);
			trim(out);
			add_history(line); // Add to history so up arrow can use what was already entered
			free((void *) line); // readline malloc's a new buffer every time.
		}
	#else
		printf_normal(prompt.c_str());
		getline(cin, out);
	#endif
	}


	/**
	 * File IO
	 */

	unique_file make_fopen(const char* filename, const char* mode) {
		FILE *fileHandle = nullptr;

	#ifdef _WIN32
		// Because of unicode file names
		fileHandle = _wfopen(s2ws(filename).c_str(), s2ws(string(mode)).c_str());
	#else
		fileHandle = fopen( filename, mode);
	#endif
		if (!fileHandle) {
			throw runtime_error(strerror(errno));
		}

		return unique_file(fileHandle);
	}

	size_t readBinaryFile(const string &filename, vector<uint8_t> &outBuffer) {

		unique_file fileHandle = make_fopen(filename.c_str(), "rb");
		size_t count = 0;

		/*
		 * Get file size, it is only an estimate, could be wrong
		 * if other program writes to it.
		 */
		if (fseek(fileHandle.get(), 0, SEEK_END) == 0) {
			long fileSize = ftell(fileHandle.get());
			rewind(fileHandle.get());
			// ftell can fail if reading from stdin for example
			if (fileSize > 0) {
				// Reserve alone wouldn't work because fread doesn't understand std::vector
				outBuffer.resize(fileSize);

				if ( (count = fread (&outBuffer[0], 1, fileSize, fileHandle.get())) != static_cast<size_t>(fileSize) ) {
					throw std::runtime_error("Error reading file: fread");
				}
			}
		} else {
			rewind(fileHandle.get());
		}

		int c;
		while ((c = fgetc (fileHandle.get())) != EOF) {
			outBuffer.emplace_back(c);
			++count;
		}
		if (ferror (fileHandle.get())) {
			throw std::runtime_error("Error reading file: fgetc");
		}

		return count;
	}

	size_t writeBinaryFile(const string &filename, const vector<uint8_t> &inBuffer) {
		unique_file fileHandle = make_fopen(filename.c_str(), "wb");
		size_t count = 0;
		count = fwrite (inBuffer.data() , 1, inBuffer.size(), fileHandle.get());
		if (ferror (fileHandle.get())) {
			throw std::runtime_error("Error writing file: fwrite");
		}
		return count;
	}

} // l18n
