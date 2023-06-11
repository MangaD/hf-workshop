/**
 * HF Workshop - IO Wrapper
 */

#ifndef IO_WRAPPER_HPP
#define IO_WRAPPER_HPP

#ifdef _WIN32
	#include <Windows.h>
#endif

#ifndef NO_GETTEXT
	/**
	 * MinGW:
	 *
	 * 32-bit:
	 *   https://packages.msys2.org/package/mingw-w64-i686-gettext
	 *
	 * 64-bit:
	 *   https://packages.msys2.org/package/mingw-w64-x86_64-gettext
	 */
	#include <libintl.h> // gettext
	#include <clocale> // setlocale
#endif // NO_GETTEXT

#include <string>
#include <vector>
#include <cstdint> // uint8_t
#include <cstring> // size_t
#include <memory>  // unique_ptr

namespace l18n {

	class localization {

	public:

		inline localization()
	#ifdef _WIN32
		: savedFont()
	#endif
		{
	#ifdef _WIN32
			saveWinConsoleFont();
	#endif
			initLocale();
		}

		inline ~localization() {
			#ifdef _WIN32
			restoreWinConsoleFont();
			#endif
		}

		/**
		 * i18n l10n
		 */
		void initLocale();

		inline const char * getText(const char * msgid) {
		#ifndef NO_GETTEXT
			/**
			 * BUGS
			 * The return type ought to be const char *, but is char * to avoid
			 * warnings in C code predating ANSI C.
			 *
			 * When an empty string is used for msgid, the functions may return a
			 * nonempty string.
			 *
			 * http://man7.org/linux/man-pages/man3/gettext.3.html
			 */
			return static_cast<const char *>(gettext(msgid));
		#else
			return msgid;
		#endif // NO_GETTEXT
		}

	private:

		#ifdef _WIN32
		CONSOLE_FONT_INFOEX savedFont;
		static constexpr int stdoutBuffSize = 1000;

		void saveWinConsoleFont();
		void restoreWinConsoleFont();
		void changeWinConsoleFont(const wchar_t * wstr, SHORT size);
		#endif

	};

	/**
	 * Print functions
	 */
	int printf_error (const char* str, ...);
	int printf_colored (int color, const char* str, ...);
	int printf_normal (const char* str, ...);
	int print2Colors(const char *s, size_t breakAt, int colorA, int colorB);
	int io_vfprintf(FILE *stream, const char* format, va_list arg);

	/**
	 * Read functions
	 */
	void readLine(std::string &out, const std::string &prompt = "");

	/**
	 * File IO
	 */
	struct file_deleter {
		void operator()(std::FILE* File) noexcept {
			std::fclose(File);
		}
	};
	using unique_file = std::unique_ptr<std::FILE,file_deleter>;
	unique_file make_fopen(const char* filename, const char* mode);
	size_t readBinaryFile(const std::string &filename, std::vector<uint8_t> &outBuffer);
	size_t writeBinaryFile(const std::string &filename, const std::vector<uint8_t> &inBuffer);


} // l18n

#endif //IO_WRAPPER_HPP


