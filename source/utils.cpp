/**
 * HF Workshop - Utility functions
 */

#include <string>     // string, wstring
#include <cctype>     // isspace, isdigit
#include <cerrno>     // errno
#include <system_error> // generic_category
#include "utils.hpp"

#include <cstdlib> // mbstowcs
#include <cstring> // strlen
#include <cwchar>  // vwprintf, wchar_t

using namespace std;


/**
 * String operations
 */
 // trim from start
std::string &ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c);}));
	return s;
}
// trim from end
std::string &rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {return !std::isspace(c);}).base(), s.end());
	return s;
}
// trim from both ends
std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}
string to_lowercase(const string &str) {
	string s = str;
	std::transform(str.begin(), str.end(), s.begin(), ::tolower);
	return s;
}
string to_uppercase(const string &str) {
	string s = str;
	std::transform(str.begin(), str.end(), s.begin(), ::toupper);
	return s;
}
bool equalsIgnoreCase(std::string str1, std::string str2) {
	str1 = to_lowercase(str1);
	str2 = to_lowercase(str2);
	return str1 == str2;
}
bool startsWith(const std::string& str, const std::string& prefix) {
	return str.size() >= prefix.size() &&
	    str.compare(0, prefix.size(), prefix) == 0;
}

bool endsWith(const std::string& str, const std::string& suffix) {
	return str.size() >= suffix.size() &&
	    str.compare(str.size()-suffix.size(), suffix.size(), suffix) == 0;
}
bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if(start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}
// remove quotes from start and end of string
std::string &removequotes(std::string &s) {
	if (s[0] == '"' || s[0] == '\'') {
		s.erase(s.begin());
	}
	if (s[s.size()-1] == '"' || s[s.size()-1] == '\'') {
		s.erase(s.begin()+s.size()-1);
	}
	return s;
}

std::string getFilenameFromPath(const std::string& path) {
	size_t pathPos = path.find_last_of("/\\");
	return (pathPos != string::npos ? path.substr(pathPos + 1) : path);
}

void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr) {
	// Get the first occurrence
	size_t pos = data.find(toSearch);

	// Repeat till end is reached
	while( pos != std::string::npos)
	{
		// Replace this occurrence of Sub String
		data.replace(pos, toSearch.size(), replaceStr);
		// Get the next occurrence from the current position
		pos =data.find(toSearch, pos + replaceStr.size());
	}
}

// https://stackoverflow.com/questions/8032080/how-to-convert-char-to-wchar-t
// https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string
std::wstring s2ws(const std::string& str) {
	std::mbstate_t state = std::mbstate_t();
	const char *c = str.c_str();
	std::size_t len = 1 + std::mbsrtowcs(nullptr, &c, 0, &state);
	if(len == 0) {
		throw std::system_error(errno, std::generic_category());
	}
	std::wstring wstr(len-1, L'\0');
	// For some reason 'mbsrtowcs' changes 'c' when
	// is isn't supposed to when dst is null.
	c = str.c_str();
	std::mbsrtowcs(&wstr[0], &c, wstr.size(), &state);
	return wstr;
}
std::string ws2s(const std::wstring& wstr) {
	std::mbstate_t state = std::mbstate_t();
	const wchar_t *c = wstr.c_str();
	std::size_t len = 1 + std::wcsrtombs(nullptr, &c, 0, &state);
	if(len == 0) {
		throw std::system_error(errno, std::generic_category());
	}
	std::string str(len-1, L'\0');
	// For some reason 'wcsrtombs' changes 'c' when
	// is isn't supposed to when dst is null.
	c = wstr.c_str();
	std::wcsrtombs(&str[0], &c, str.size(), &state);
	return str;
}
int extractIntFromStr(const std::string &str) {
	size_t i = 0;
	for (; i < str.length(); i++ ){ if ( isdigit(str[i]) ) break; }
	return std::stoi(string{str.begin()+i, str.end()});
}
