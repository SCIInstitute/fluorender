/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
/**
 * This file is used for compatibility across windows and mac/linux platforms.
 * This is specific to FLuoRender Code.
 * @author Brig Bagley
 * @version 4 March 2014
 */
#ifndef __COMPATIBILITY_H__
#define __COMPATIBILITY_H__

#include <string>
#include <cstring>
#include <cwchar>
#include <fstream>
#include <iostream>
#include <vector>
#include <cctype>
#include <cmath>
#include <regex>
#include <filesystem>
#include <chrono>

/**
	* This method swaps the byte order of a short.
	* @param num The short to swap byte order.
	* @return The short with bytes swapped.
	*/
inline uint16_t SwapShort(uint16_t num) {
	return ((num & 0x00FF) << 8) | ((num & 0xFF00) >> 8);
}

/**
	* This method swaps the byte order of a word.
	* @param num The word to swap byte order.
	* @return The word with bytes swapped.
	*/
inline uint32_t SwapWord(uint32_t num) {
	return ((num & 0x000000FF) << 24) | ((num & 0xFF000000) >> 24) |
		((num & 0x0000FF00) << 8) | ((num & 0x00FF0000) >> 8);
}

/**
	* This method swaps the byte order of a 8byte number.
	* @param num The 8byte to swap byte order.
	* @return The 8byte with bytes swapped.
	*/
inline uint64_t SwapLong(uint64_t num) {
	return
		((num & 0x00000000000000FF) << 56) |
		((num & 0xFF00000000000000) >> 56) |
		((num & 0x000000000000FF00) << 40) |
		((num & 0x00FF000000000000) >> 40) |
		((num & 0x0000000000FF0000) << 24) |
		((num & 0x0000FF0000000000) >> 24) |
		((num & 0x00000000FF000000) << 8) |
		((num & 0x000000FF00000000) >> 8);
}

inline std::regex REGEX(const std::string& wildcard, bool caseSensitive = true)
{
	// Note It is possible to automate checking if filesystem is case sensitive or not (e.g. by performing a test first time this function is ran)
	std::string regexString{ wildcard };
	// Escape all regex special chars:
	regexString = std::regex_replace(regexString, std::regex("\\\\"), "\\\\");
	regexString = std::regex_replace(regexString, std::regex("\\^"), "\\^");
	regexString = std::regex_replace(regexString, std::regex("\\."), "\\.");
	regexString = std::regex_replace(regexString, std::regex("\\$"), "\\$");
	regexString = std::regex_replace(regexString, std::regex("\\|"), "\\|");
	regexString = std::regex_replace(regexString, std::regex("\\("), "\\(");
	regexString = std::regex_replace(regexString, std::regex("\\)"), "\\)");
	regexString = std::regex_replace(regexString, std::regex("\\{"), "\\{");
	regexString = std::regex_replace(regexString, std::regex("\\{"), "\\}");
	regexString = std::regex_replace(regexString, std::regex("\\["), "\\[");
	regexString = std::regex_replace(regexString, std::regex("\\]"), "\\]");
	regexString = std::regex_replace(regexString, std::regex("\\+"), "\\+");
	regexString = std::regex_replace(regexString, std::regex("\\/"), "\\/");
	// Convert wildcard specific chars '*?' to their regex equivalents:
	regexString = std::regex_replace(regexString, std::regex("\\?"), ".");
	regexString = std::regex_replace(regexString, std::regex("\\*"), ".*");

	return std::regex(regexString, caseSensitive ? std::regex_constants::ECMAScript : std::regex_constants::icase);
}

inline std::wregex REGEX(const std::wstring& wildcard, bool caseSensitive = true)
{
	// Note It is possible to automate checking if filesystem is case sensitive or not (e.g. by performing a test first time this function is ran)
	std::wstring regexString{ wildcard };
	// Escape all regex special chars:
	regexString = std::regex_replace(regexString, std::wregex(L"\\\\"), L"\\\\");
	regexString = std::regex_replace(regexString, std::wregex(L"\\^"), L"\\^");
	regexString = std::regex_replace(regexString, std::wregex(L"\\."), L"\\.");
	regexString = std::regex_replace(regexString, std::wregex(L"\\$"), L"\\$");
	regexString = std::regex_replace(regexString, std::wregex(L"\\|"), L"\\|");
	regexString = std::regex_replace(regexString, std::wregex(L"\\("), L"\\(");
	regexString = std::regex_replace(regexString, std::wregex(L"\\)"), L"\\)");
	regexString = std::regex_replace(regexString, std::wregex(L"\\{"), L"\\{");
	regexString = std::regex_replace(regexString, std::wregex(L"\\{"), L"\\}");
	regexString = std::regex_replace(regexString, std::wregex(L"\\["), L"\\[");
	regexString = std::regex_replace(regexString, std::wregex(L"\\]"), L"\\]");
	regexString = std::regex_replace(regexString, std::wregex(L"\\+"), L"\\+");
	regexString = std::regex_replace(regexString, std::wregex(L"\\/"), L"\\/");
	// Convert wildcard specific chars '*?' to their regex equivalents:
	regexString = std::regex_replace(regexString, std::wregex(L"\\?"), L".");
	regexString = std::regex_replace(regexString, std::wregex(L"\\*"), L".*");

	return std::wregex(regexString, caseSensitive ? std::regex_constants::ECMAScript : std::regex_constants::icase);
}

inline std::string REM_EXT(const std::string& str)
{
	size_t pos = str.rfind('.');
	if (pos != std::string::npos)
		return str.substr(0, pos);
	return str;
}

inline std::string REM_NUM(const std::string& str)
{
	std::string tmp = REM_EXT(str);
	while (!tmp.empty() && std::isdigit(tmp.back()))
	{
		tmp.pop_back();
	}
	return tmp;
}

inline std::wstring REM_EXT(const std::wstring& str)
{
	size_t pos = str.rfind(L'.');
	if (pos != std::wstring::npos)
		return str.substr(0, pos);
	return str;
}

inline std::wstring REM_NUM(const std::wstring& str)
{
	std::wstring tmp = REM_EXT(str);
	while (!tmp.empty() && std::isdigit(tmp.back()))
	{
		tmp.pop_back();
	}
	return tmp;
}

inline std::string INC_NUM(const std::string& str)
{
	size_t pos = str.rfind('.');
	std::string ext;
	if (pos != std::string::npos) {
		ext = str.substr(pos);
	}
	std::string tmp = str.substr(0, pos);
	std::string digits;
	while (!tmp.empty() && std::isdigit(tmp.back())) {
		digits.insert(digits.begin(), tmp.back());
		tmp.pop_back();
	}
	size_t len = digits.length();
	if (len == 0) {
		return tmp + "01" + ext;
	}
	int num = std::stoi(digits);
	num++;
	std::ostringstream oss;
	oss << std::setw(len) << std::setfill('0') << num;
	digits = oss.str();
	return tmp + digits + ext;
}

inline std::wstring INC_NUM(const std::wstring& str)
{
	size_t pos = str.rfind('.');
	std::wstring ext;
	if (pos != std::wstring::npos) {
		ext = str.substr(pos);
	}
	std::wstring tmp = str.substr(0, pos);
	std::wstring digits;
	while (!tmp.empty() && std::isdigit(tmp.back())) {
		digits.insert(digits.begin(), tmp.back());
		tmp.pop_back();
	}
	size_t len = digits.length();
	if (len == 0) {
		return tmp + L"01" + ext;
	}
	int num = std::stoi(digits);
	num++;
	std::wostringstream oss;
	oss << std::setw(len) << std::setfill(L'0') << num;
	digits = oss.str();
	return tmp + digits + ext;
}

inline std::string INC_NUM_EXIST(const std::string& str)
{
	std::string str2 = str;
	while (std::filesystem::exists(str2))
		str2 = INC_NUM(str2);
	return str2;
}

inline std::wstring INC_NUM_EXIST(const std::wstring& str)
{
	std::wstring str2 = str;
	while (std::filesystem::exists(str2))
		str2 = INC_NUM(str2);
	return str2;
}

inline std::string MAKE_NUM(int num, int len)
{
	std::ostringstream oss;
	oss << std::setw(len) << std::setfill('0') << num;
	return oss.str();
}

inline std::wstring MAKE_NUMW(int num, int len)
{
	std::wostringstream oss;
	oss << std::setw(len) << std::setfill(L'0') << num;
	return oss.str();
}

#define PI_2 1.5707963267948966192313216916398
#define PI 3.1415926535897932384626433832795
#define EPS 1e-6

inline double d2r(double d)
{
	return d * 0.017453292519943295769236907684886;
}

inline double r2d(double r)
{
	return r * 57.295779513082320876798154814105;
}

inline void sinCos(double* returnSin, double* returnCos, double theta)
{
	// For simplicity, we'll just use the normal trig functions.
	// Note that on some platforms we may be able to do better

	*returnSin = sin(theta);
	*returnCos = cos(theta);
}

static const unsigned char BitReverseTable256[] =
{
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

inline unsigned int bit_reverse(unsigned int v)
{
	unsigned int c; // reverse 32-bit value, 8 bits at time 

	// Option 1: 
	c = (BitReverseTable256[v & 0xff] << 24) |
		(BitReverseTable256[(v >> 8) & 0xff] << 16) |
		(BitReverseTable256[(v >> 16) & 0xff] << 8) |
		(BitReverseTable256[(v >> 24) & 0xff]);

	return c;
}

inline unsigned int reverse_bit(unsigned int val, unsigned int len)
{
	unsigned int res = val;
	int s = len - 1;

	for (val >>= 1; val; val >>= 1)
	{
		res <<= 1;
		res |= val & 1;
		s--;
	}
	res <<= s;
	res <<= 32 - len;
	res >>= 32 - len;
	return res;
}

inline float nCr(int n, int r)
{
	if (n < r || n < 0 || r < 0)
		return 0;
	float result = 1;
	for (int i = 0; i < r; i++)
		result *= float(n - i) / float(r - i);

	return result;
}

inline std::string split_host_name_and_port(const std::string& address, uint16_t& port)
{
	static std::basic_regex<char> addressMatcher("(?:(\\[.*\\])|([^:]*))(?:[:](\\d+))?");
	std::match_results<typename std::string::const_iterator> results;
	if (std::regex_match(address, results, addressMatcher)) {
		if (results[3].matched) {
			std::string portStr = results[3].str();
			port = static_cast<uint16_t>(std::strtol(portStr.c_str(), nullptr, 10));
		}

		return (results[1].matched) ? results[1].str() : results[2].str();
	}
	else {
		return address;
	}
}

inline std::wstring GET_SUFFIX(const std::wstring& pathname)
{
	std::wstring extension = std::filesystem::path(pathname).extension().wstring();
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	return extension;
}

inline std::string GET_SUFFIX(const std::string& pathname)
{
	std::string extension = std::filesystem::path(pathname).extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	return extension;
}

inline unsigned long long TIME()
{
	using namespace std::chrono;
	return time_point_cast<seconds>(system_clock::now()).time_since_epoch().count();
}

inline unsigned long long GET_TICK_COUNT()
{
	using namespace std::chrono;
	return time_point_cast<milliseconds>(steady_clock::now()).time_since_epoch().count();
}

inline std::string STR_DIR_SEP(const std::string& pathname)
{
	std::filesystem::path path(pathname);
	return path.make_preferred().string();
}

inline std::wstring STR_DIR_SEP(const std::wstring& pathname)
{
	std::filesystem::path path(pathname);
	return path.make_preferred().wstring();
}

inline std::wstring NORM_PATH(const std::wstring& path)
{
	std::wstring normalizedPath = path;
	std::replace(normalizedPath.begin(), normalizedPath.end(), L'\\', L'/');
	return normalizedPath;
}

inline std::wstring ESCAPE_REGEX(const std::wstring& input)
{
	static const std::wregex special_chars(L"([.^$|()\\[\\]{}*+?\\\\])");
	return std::regex_replace(input, special_chars, L"\\$1");
}

inline bool FIND_FILES_4D(const std::wstring& path_name, const std::wstring& id, std::vector<std::wstring>& batch_list, int& cur_batch)
{
	std::wstring filename = std::filesystem::path(path_name).filename().wstring();

	// Find the position of the ID in the filename
	size_t begin = filename.rfind(id);
	if (begin == std::wstring::npos)
		return false;

	size_t id_len = id.length();

	// Extract digits immediately after the ID
	std::wstring index_digits;
	size_t k = begin + id_len;
	while (k < filename.length() && iswdigit(filename[k])) {
		index_digits.push_back(filename[k]);
		++k;
	}

	if (index_digits.empty())
		return false;

	// Build regex pattern: match files with same prefix, ID, and digits
	std::wstring prefix = filename.substr(0, begin);
	std::wstring suffix = filename.substr(k); // anything after the digits
	std::wstringstream pattern;
	pattern << L"^" << ESCAPE_REGEX(prefix)
		<< ESCAPE_REGEX(id)
		<< L"(\\d+)"  // capture digits
		<< ESCAPE_REGEX(suffix) << L"$";

	std::wregex file_regex(pattern.str());

	// Search in the parent directory
	std::filesystem::path dir = std::filesystem::path(path_name).parent_path();
	std::vector<std::pair<int, std::wstring>> indexed_files;
	int input_index = std::stoi(index_digits);

	for (const auto& entry : std::filesystem::directory_iterator(dir))
	{
		if (!entry.is_regular_file())
			continue;

		std::wstring fname = entry.path().filename().wstring();
		std::wsmatch match;
		if (std::regex_match(fname, match, file_regex))
		{
			int index = std::stoi(match[1].str());
			indexed_files.emplace_back(index, entry.path().wstring());
		}
	}

	if (indexed_files.empty())
		return false;

	std::sort(indexed_files.begin(), indexed_files.end());

	batch_list.clear();
	cur_batch = -1;
	for (size_t i = 0; i < indexed_files.size(); ++i)
	{
		batch_list.push_back(indexed_files[i].second);
		if (indexed_files[i].first == input_index)
			cur_batch = static_cast<int>(i);
	}

	return true;
}

inline void FIND_FILES_BATCH(const std::wstring& path_name,
	const std::wstring& search_mask,
	std::vector<std::wstring>& batch_list,
	int& cur_batch)
{
	std::filesystem::path p(path_name);
	p = std::filesystem::absolute(p).parent_path();
	std::wstring search_path = p.wstring();
	std::wstring full_search_mask = L".*" + search_mask;

	std::wregex regex(full_search_mask, std::regex::icase);
	batch_list.clear();
	int cnt = 0;

	for (const auto& entry : std::filesystem::directory_iterator(search_path))
	{
		if (!entry.is_regular_file())
			continue;
		std::wstring str = entry.path().filename().wstring();
		if (std::regex_match(str, regex))
		{
			std::wstring name = entry.path().wstring();
			batch_list.push_back(name);
			if (name == path_name)
				cur_batch = cnt;
			cnt++;
		}
	}
}

inline void FIND_FILES(const std::wstring& path_name,
	const std::wstring& search_mask,
	std::vector<std::wstring>& batch_list,
	int& cur_batch)
{
	std::filesystem::path p(path_name);
	p = p.parent_path();
	std::wstring search_path = p.wstring();

	std::wregex regex(search_mask);
	batch_list.clear();
	int cnt = 0;

	for (const auto& entry : std::filesystem::directory_iterator(search_path))
	{
		if (!entry.is_regular_file())
			continue;
		std::wstring str = entry.path().filename().wstring();
		if (std::regex_match(str, regex))
		{
			std::filesystem::path full_path = p / entry.path().filename();
			std::wstring name = full_path.wstring();
			batch_list.push_back(name);
			if (name == path_name)
				cur_batch = cnt;
			cnt++;
		}
	}
}

inline bool IS_NUMBER(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(int(c)); }) == s.end();
}

inline void INC_NUMBER(std::string& s)
{
	if (s.empty())
	{
		s = "1";
		return;
	}
	if (std::isdigit(s.back()))
	{
		size_t p = s.find_last_not_of("0123456789") + 1;
		std::string sn = s.substr(p);
		s = s.substr(0, p);
		s += std::to_string(std::stoi(sn) + 1);
	}
	else
	{
		s += "1";
	}
}

inline void INC_NUMBER(std::wstring& s)
{
	if (s.empty())
	{
		s = L"1";
		return;
	}
	if (std::isdigit(s.back()))
	{
		size_t p = s.find_last_not_of(L"0123456789") + 1;
		std::wstring sn = s.substr(p);
		s = s.substr(0, p);
		s += std::to_wstring(std::stoi(sn) + 1);
	}
	else
	{
		s += L"1";
	}
}

inline std::wstring s2ws(const std::string& s)
{
	std::mbstate_t state = std::mbstate_t();
	const char* src = s.data();
	size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
	if (len == static_cast<size_t>(-1))
		return L"";
	std::wstring dest(len, L'\0');
	std::mbsrtowcs(&dest[0], &src, len, &state);
	return dest;
}

inline std::string ws2s(const std::wstring& ws)
{
	std::mbstate_t state = std::mbstate_t();
	const wchar_t* src = ws.data();
	size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
	if (len == static_cast<size_t>(-1))
		return "";
	std::string dest(len, '\0');
	std::wcsrtombs(&dest[0], &src, len, &state);
	return dest;
}

inline std::wstring GET_STEM(const std::wstring& pathname)
{
	std::filesystem::path path(pathname);
	return path.stem().wstring();
}

inline std::string GET_STEM(const std::string& pathname)
{
	std::filesystem::path path(pathname);
	return path.stem().string();
}

inline std::wstring GET_NAME(const std::wstring& pathname)
{
	std::filesystem::path path(pathname);
	return path.filename().wstring();
}

inline std::string GET_NAME(const std::string& pathname)
{
	std::filesystem::path path(pathname);
	return path.filename().string();
}

inline std::wstring GET_PATH(const std::wstring& pathname)
{
	namespace fs = std::filesystem;
	fs::path result;
	fs::path path(pathname);

	if (fs::exists(path) && fs::is_directory(path)) {
		// It's a valid directory → use it as-is
		result = path;
	}
	else {
		// It's likely a file → use its parent path
		result = path.parent_path();
	}

	// Ensure trailing separator
	std::wstring result_str = result.wstring();
	if (!result_str.empty() && result_str.back() != fs::path::preferred_separator) {
		result_str += fs::path::preferred_separator;
	}

	return result_str;
}

inline std::string GET_PATH(const std::string& pathname)
{
	namespace fs = std::filesystem;
	fs::path result;
	fs::path path(pathname);

	if (fs::exists(path) && fs::is_directory(path)) {
		// It's a valid directory → use it as-is
		result = path;
	}
	else {
		// It's likely a file → use its parent path
		result = path.parent_path();
	}

	// Ensure trailing separator
	std::string result_str = result.string();
	if (!result_str.empty() && result_str.back() != fs::path::preferred_separator) {
		result_str += fs::path::preferred_separator;
	}

	return result_str;
}

inline bool SEP_PATH_NAME(const std::wstring& pathname, std::wstring& path, std::wstring& name)
{
	std::filesystem::path fs_path(pathname);

	// Check if the path exists and is a file
	if (!std::filesystem::exists(fs_path) || !std::filesystem::is_regular_file(fs_path)) {
		return false;
	}

	std::filesystem::path p = fs_path.parent_path();
	p /= "";
	path = p.wstring();
	name = fs_path.filename().wstring();
	return true;
}

inline bool SEP_PATH_NAME(const std::string& pathname, std::string& path, std::string& name)
{
	std::filesystem::path fs_path(pathname);

	// Check if the path exists and is a file
	if (!std::filesystem::exists(fs_path) || !std::filesystem::is_regular_file(fs_path)) {
		return false;
	}

	std::filesystem::path p = fs_path.parent_path();
	p /= "";
	path = p.string();
	name = fs_path.filename().string();
	return true;
}

inline void CHECK_TRAILING_SLASH(std::wstring& str)
{
	std::filesystem::path path(str);
	if (!path.empty() && path.native().back() != std::filesystem::path::preferred_separator) {
		path += std::filesystem::path::preferred_separator;
	}
	str = path.wstring();
}

inline void CHECK_TRAILING_SLASH(std::string& str)
{
	std::filesystem::path path(str);
	if (!path.empty() && path.native().back() != std::filesystem::path::preferred_separator) {
		path += std::filesystem::path::preferred_separator;
	}
	str = path.string();
}

inline std::wstring APPEND_QUILT_INFO(const std::wstring& file, int vx, int vy, double aspect)
{
	std::filesystem::path old_path(file);
	std::filesystem::path base = old_path;
	base.replace_extension();

	std::wstringstream suffix;
	suffix << L"_qs" << vx << L"x" << vy
		<< L"a" << std::fixed << std::setprecision(2) << aspect;

	std::wstring new_path = base.wstring() + suffix.str() + old_path.extension().wstring();
	return new_path;
}

extern "C" {
	typedef struct tiff TIFF;
	TIFF* TIFFOpenW(const wchar_t* name, const char* mode);
}

inline char GETSLASHA() { return std::filesystem::path::preferred_separator; }
inline wchar_t GETSLASH() { return static_cast<wchar_t>(GETSLASHA()); }

inline int STOI(const char* s) { return (s ? atoi(s) : 0); }
inline int STOI(const std::string& s, int def = 0) { try { return std::stoi(s); } catch (...) { return def; } }
inline double STOD(const char* s) { return (s ? atof(s) : 0.0); }
inline double STOD(const std::string& s, double def = 0.0) { try { return std::stod(s); } catch (...) { return def; } }
inline long STOL(const std::string& s, long def = 0) { try { return std::stol(s); } catch (...) { return def; } }
inline unsigned long STOUL(const std::string& s, unsigned long def = 0) { try { return std::stoul(s); } catch (...) { return def; } }
inline unsigned long long STOULL(const std::string& s, unsigned long long def = 0) { try { return std::stoull(s); } catch (...) { return def; } }
inline float STOF(const std::string& s, float def = 0.0f) { try { return std::stof(s); } catch (...) { return def; } }
inline int WSTOI(const std::wstring& s, int def = 0) { try { return std::stoi(s); } catch (...) { return def; } }
inline double WSTOD(const std::wstring& s, double def = 0.0) { try { return std::stod(s); } catch (...) { return def; } }


template<typename T>
typename std::vector<std::weak_ptr<T>>::iterator FIND_PTR(
	std::vector<std::weak_ptr<T>>& vec, const std::shared_ptr<T>& target)
{
	return std::find_if(vec.begin(), vec.end(),
		[&target](const std::weak_ptr<T>& wp) {
		auto sp = wp.lock();
		return sp && sp == target;
	});
}

#ifdef _WIN32 //WINDOWS ONLY

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <sys/types.h>
#include <ctype.h>
#include <direct.h>

#define FSEEK64     _fseeki64
#define SSCANF    sscanf

inline wchar_t GETSLASHALT() { return L'/'; }
inline char GETSLASHALTA() { return '/'; }

inline TIFF* TIFFOpenW(const std::wstring& fname, const char* opt)
{
	std::wstring path = L"\x5c\x5c\x3f\x5c" + fname;
	return TIFFOpenW(path.c_str(), opt);
}

inline FILE* FOPEN(FILE** fp, const std::string& fname, const char* mode)
{
	std::string path = "\x5c\x5c\x3f\x5c" + fname;
	fopen_s(fp, path.c_str(), mode);
	return *fp;
}

inline FILE* WFOPEN(FILE** fp, const std::wstring& fname, const wchar_t* mode)
{
	std::wstring path = L"\x5c\x5c\x3f\x5c" + fname;
	_wfopen_s(fp, path.c_str(), mode);
	return *fp;
}

inline void OutputStreamOpen(std::ofstream& os, const std::string& fname)
{
	std::string path = "\x5c\x5c\x3f\x5c" + fname;
	os.open(path);
}

inline void OutputStreamOpenW(std::wofstream& os, const std::wstring& fname)
{
	std::wstring path = L"\x5c\x5c\x3f\x5c" + fname;
	os.open(path);
}

inline int MkDir(const std::string& dirname)
{
	std::string path = "\x5c\x5c\x3f\x5c" + dirname;
	return _mkdir(path.c_str());
}

inline int MkDirW(const std::wstring& dirname)
{
	std::wstring path = L"\x5c\x5c\x3f\x5c" + dirname;
	return _wmkdir(path.c_str());
}

inline errno_t STRCPY(char* d, size_t n, const char* s) { return strcpy_s(d, n, s); }

inline errno_t STRNCPY(char* d, size_t n, const char* s, size_t x) {
	return strncpy_s(d, n, s, x);
}

inline errno_t STRCAT(char* d, size_t n, const char* s) {
	return strcat_s(d, n, s);
}

inline char* STRDUP(const char* s) { return _strdup(s); }

inline int SPRINTF(char* buf, size_t n, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int r = vsprintf_s(buf, n, fmt, args);
	va_end(args);
	return r;
}

#else // MAC OSX or LINUX

#include <unistd.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/stat.h>

#define FSEEK64     fseek

inline bool str_mat(std::wstring& s1, size_t p1, std::wstring& s2, size_t p2)
{
	// If we reach at the end of both strings, we are done
	if (s1[p1] == L'\0' && s2[p2] == L'\0')
		return true;

	// Make sure that the characters after '*' are present 
	// in second string. This function assumes that the first 
	// string will not contain two consecutive '*' 
	if (s1[p1] == L'*' && s1[p1 + 1] != L'\0' && s2[p2] == L'\0')
		return false;

	// If the first string contains '?', or current characters 
	// of both strings match 
	if (s1[p1] == L'?' || s1[p1] == s2[p2])
		return str_mat(s1, p1 + 1, s2, p2 + 1);

	// If there is *, then there are two possibilities 
	// a) We consider current character of second string 
	// b) We ignore current character of second string. 
	if (s1[p1] == L'*')
		return str_mat(s1, p1 + 1, s2, p2) || str_mat(s1, p1, s2, p2 + 1);
	return false;
}

inline int SSCANF(const char* buf, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int r = vsscanf(buf, fmt, args);
	va_end(args);
	return r;
}

inline int swprintf_s(wchar_t* buf, size_t n, const wchar_t* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int r = vswprintf(buf, n, fmt, args);
	va_end(args);
	return r;
}

inline void SetDoubleBuffered(bool) {};

inline char* STRCPY(char* d, size_t n, const char* s) { return strncpy(d, s, n - 1); }

inline char* STRNCPY(char* d, size_t n, const char* s, size_t x) {
	return strncpy(d, s, n - 1);
}

inline char* STRCAT(char* d, size_t n, const char* s) {
	return strncat(d, s, n - strlen(d) - 1);
}

inline char* STRDUP(const char* s) { return strdup(s); }

inline TIFF* TIFFOpenW(std::wstring fname, const char* opt) {
	return TIFFOpen(ws2s(fname).c_str(), opt);
}

inline int SPRINTF(char* buf, size_t n, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int r = vsprintf(buf, fmt, args);
	va_end(args);
	return r;
}

typedef union _LARGE_INTEGER {
	struct {
		unsigned int LowPart;
		long HighPart;
	} v;
	struct {
		unsigned int LowPart;
		long HighPart;
	} u;
	long long QuadPart;
} LARGE_INTEGER, * PLARGE_INTEGER;

inline FILE* WFOPEN(FILE** fp, std::wstring& filename, const wchar_t* mode) {
	*fp = fopen(ws2s(filename).c_str(),
		ws2s(std::wstring(mode)).c_str());
	return *fp;
}

inline FILE* FOPEN(FILE** fp, std::string& filename, const char* mode) {
	*fp = fopen(filename.c_str(), mode);
	return *fp;
}

inline void OutputStreamOpen(std::ofstream& os, std::string fname)
{
	os.open(fname);
}

inline void OutputStreamOpenW(std::wofstream& os, std::wstring fname)
{
	//os.open(L"test");//fname.c_str());
}

inline int MkDir(std::string dirname)
{
	return mkdir(dirname.c_str(), 0777);
}

inline int MkDirW(std::wstring dirname)
{
	return mkdir(ws2s(dirname).c_str(), 0777);
}

//LINUX SPECIFIC
#ifdef _LINUX
#endif
//MAC OSX SPECIFIC
#ifdef _DARWIN
#include <dlfcn.h>
#endif

#endif //END_IF_DEF__WIN32__

#endif //END__COMPATIBILITY_H__
