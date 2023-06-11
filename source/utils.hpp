/**
 * HF Workshop - Utility functions
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#ifdef __GNUC__
	#define QUOTE(s) #s
	#define DIAGNOSTIC_PUSH() _Pragma("GCC diagnostic push")
	#define DIAGNOSTIC_IGNORE(warning) _Pragma(QUOTE(GCC diagnostic ignored warning))
	#define DIAGNOSTIC_POP() _Pragma("GCC diagnostic pop")
#else
	#define DIAGNOSTIC_PUSH()
	#define DIAGNOSTIC_IGNORE(warning)
	#define DIAGNOSTIC_POP()
#endif

#include <string>     // wstring
#include <vector>     // vector
#include <algorithm>  // sort, unique
#include <sstream>    // stringstream
#include <iomanip>    // setfill, setw
#include <ios>        // hex


/**
 * String operations
 */
/// trim from start
std::string &ltrim(std::string &s);
/// trim from end
std::string &rtrim(std::string &s);
/// trim from both ends
std::string &trim(std::string &s);

/**
 * Converts the string to lower case
 */
std::string to_lowercase(const std::string &str);

/**
 * Converts the string to upper case
 */
std::string to_uppercase(const std::string &str);

/**
 * Compares two strings case insensitive
 */
bool equalsIgnoreCase(std::string str1, std::string str2);

/**
 * Checks if string ends with a given suffix, case sensitive
 */
bool endsWith(const std::string& str, const std::string& suffix);

/**
 * Checks if string starts with a given prefix, case sensitive
 */
bool startsWith(const std::string& str, const std::string& prefix);

/**
 * Replaces a substring in a string.
 */
bool replace(std::string& str, const std::string& from, const std::string& to);

/// remove quotes from start and end of string
std::string &removequotes(std::string &s);

/**
 * Returns filename from path
 */
std::string getFilenameFromPath(const std::string& path);

/**
 * Replaces all occurrences of a substring in a string
 */
void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr);

/**
 * Converts a string to a wstring
 */
std::wstring s2ws(const std::string& str);

/**
 * Converts a wstring to a string
 */
std::string ws2s(const std::wstring& wstr);

/**
 * Extracts an integer from a string
 */
int extractIntFromStr(const std::string &str);


template<class T>
std::vector<T> getVectorOfNumbers(std::string &ids) {
	std::vector<T> ids_v;

	// Replace commas in string with spaces
	std::replace(ids.begin(), ids.end(), ',', ' ');

	std::stringstream ss(ids);
	T i;
	while (ss >> i) {
		ids_v.emplace_back(i);
	}

	// return empty vector on malformed input
	if (!ss.eof()) {
		return {};
	}

	// Remove duplicates
	// https://stackoverflow.com/questions/1041620/whats-the-most-efficient-way-to-erase-duplicates-and-sort-a-vector
	std::sort( ids_v.begin(), ids_v.end() );
	ids_v.erase( std::unique( ids_v.begin(), ids_v.end() ), ids_v.end() );

	return ids_v;
}

// Taken from: https://stackoverflow.com/a/48643043/3049315
/// Convert integer value `val` to text in hexadecimal format.
/// The minimum width is padded with leading zeros; if not
/// specified, this `width` is derived from the type of the
/// argument. Function suitable from char to long long.
/// Pointers, floating point values, etc. are not supported;
/// passing them will result in an (intentional!) compiler error.
/// Basics from: http://stackoverflow.com/a/5100745/2932052
template <typename T>
inline std::string int_to_hex(T val, int width = sizeof(T) * 2) {
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(width) << std::hex << (val | 0);
	return ss.str();
}

#endif // UTILS_HPP
