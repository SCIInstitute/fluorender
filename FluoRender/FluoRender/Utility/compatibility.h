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

#include <boost/locale.hpp>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <tiffio.h>
#include <cctype>
#include <cmath>
#include <regex>
#include <filesystem>

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
	int len = digits.length();
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

inline std::string INC_NUM_EXIST(const std::string& str)
{
	std::string str2 = str;
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

inline std::wstring GET_SUFFIX(const std::wstring &pathname)
{
	std::wstring extension = std::filesystem::path(pathname).extension().wstring();
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	return extension;
}

inline std::string GET_SUFFIX(const std::string &pathname)
{
	std::string extension = std::filesystem::path(pathname).extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	return extension;
}


#ifdef _WIN32 //WINDOWS ONLY

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <chrono>
#include <sys/types.h>
#include <ctype.h>
#include <direct.h>

#define GETCURRENTDIR _getcwd

#define FSEEK64     _fseeki64
#define SSCANF    sscanf

inline wchar_t GETSLASH() { return L'\\'; }
inline wchar_t GETSLASHALT() { return L'/'; }
inline char GETSLASHA() { return '\\'; }
inline char GETSLASHALTA() { return '/'; }

inline std::wstring GET_NAME(std::wstring &pathname)
{
	size_t pos1 = pathname.find_last_of(GETSLASH());
	size_t pos2 = pathname.find_last_of(GETSLASHALT());
	if (pos1 != std::wstring::npos &&
		pos2 != std::wstring::npos)
		return pathname.substr((pos1 > pos2 ? pos1 : pos2) + 1);
	else if (pos1 != std::wstring::npos)
		return pathname.substr(pos1 + 1);
	else if (pos2 != std::wstring::npos)
		return pathname.substr(pos2 + 1);
	else
		return pathname;
}

inline std::string GET_NAMEA(std::string &pathname)
{
	size_t pos1 = pathname.find_last_of(GETSLASHA());
	size_t pos2 = pathname.find_last_of(GETSLASHALTA());
	if (pos1 != std::string::npos &&
		pos2 != std::string::npos)
		return pathname.substr((pos1 > pos2 ? pos1 : pos2) + 1);
	else if (pos1 != std::string::npos)
		return pathname.substr(pos1 + 1);
	else if (pos2 != std::string::npos)
		return pathname.substr(pos2 + 1);
	else
		return pathname;
}

inline std::wstring GET_PATH(std::wstring &pathname)
{
	size_t pos1 = pathname.find_last_of(GETSLASH());
	size_t pos2 = pathname.find_last_of(GETSLASHALT());
	if (pos1 != std::wstring::npos &&
		pos2 != std::wstring::npos)
		return pathname.substr(0, (pos1 > pos2 ? pos1 : pos2) + 1);
	else if (pos1 != std::wstring::npos)
		return pathname.substr(0, pos1 + 1);
	else if (pos2 != std::wstring::npos)
		return pathname.substr(0, pos2 + 1);
	else
		return pathname;
}

inline std::string GET_PATHA(std::string &pathname)
{
	size_t pos1 = pathname.find_last_of(GETSLASHA());
	size_t pos2 = pathname.find_last_of(GETSLASHALTA());
	if (pos1 != std::string::npos &&
		pos2 != std::string::npos)
		return pathname.substr(0, (pos1 > pos2 ? pos1 : pos2) + 1);
	else if (pos1 != std::string::npos)
		return pathname.substr(0, pos1 + 1);
	else if (pos2 != std::string::npos)
		return pathname.substr(0, pos2 + 1);
	else
		return pathname;
}

inline bool SEP_PATH_NAME(std::wstring &pathname, std::wstring &path, std::wstring &name)
{
	size_t pos1 = pathname.find_last_of(GETSLASH());
	size_t pos2 = pathname.find_last_of(GETSLASHALT());
	if (pos1 != std::wstring::npos &&
		pos2 != std::wstring::npos)
	{
		path = pathname.substr(0, (pos1 > pos2 ? pos1 : pos2) + 1);
		name = pathname.substr((pos1 > pos2 ? pos1 : pos2) + 1);
		return true;
	}
	else if (pos1 != std::wstring::npos)
	{
		path = pathname.substr(0, pos1 + 1);
		name = pathname.substr(pos1 + 1);
		return true;
	}
	else if (pos2 != std::wstring::npos)
	{
		path = pathname.substr(0, pos2 + 1);
		name = pathname.substr(pos2 + 1);
		return true;
	}
	else
		return false;
}

inline bool SEP_PATH_NAMEA(std::string &pathname, std::string &path, std::string &name)
{
	size_t pos1 = pathname.find_last_of(GETSLASHA());
	size_t pos2 = pathname.find_last_of(GETSLASHALTA());
	if (pos1 != std::string::npos &&
		pos2 != std::string::npos)
	{
		path = pathname.substr(0, (pos1 > pos2 ? pos1 : pos2) + 1);
		name = pathname.substr((pos1 > pos2 ? pos1 : pos2) + 1);
		return true;
	}
	else if (pos1 != std::string::npos)
	{
		path = pathname.substr(0, pos1 + 1);
		name = pathname.substr(pos1 + 1);
		return true;
	}
	else if (pos2 != std::string::npos)
	{
		path = pathname.substr(0, pos2 + 1);
		name = pathname.substr(pos2 + 1);
		return true;
	}
	else
		return false;
}

inline std::wstring s2ws(const std::string& s)
{
	return boost::locale::conv::utf_to_utf<wchar_t>(s);
}

inline std::string ws2s(const std::wstring& ws)
{
	return boost::locale::conv::utf_to_utf<char>(ws);
}

inline TIFF* TIFFOpenW(std::wstring fname, const char* opt)
{
	fname = L"\x5c\x5c\x3f\x5c" + fname;
	return TIFFOpenW(fname.c_str(), opt);
}

inline FILE* FOPEN(FILE** fp, const char *fname, const char* mode) {
	fopen_s(fp, fname, mode);
	return *fp;
}

inline FILE* WFOPEN(FILE** fp, const wchar_t* fname, const wchar_t* mode) {
	_wfopen_s(fp, fname, mode);
	return *fp;
}

inline void OutputStreamOpen(std::ofstream &os, std::string fname)
{
	fname = "\x5c\x5c\x3f\x5c" + fname;
	os.open(fname);
}

inline void OutputStreamOpenW(std::wofstream &os, std::wstring fname)
{
	fname = L"\x5c\x5c\x3f\x5c" + fname;
	os.open(fname);
}

inline int MkDir(std::string dirname)
{
	dirname = "\x5c\x5c\x3f\x5c" + dirname;
	return _mkdir(dirname.c_str());
}

inline int MkDirW(std::wstring dirname)
{
	dirname = L"\x5c\x5c\x3f\x5c" + dirname;
	return _wmkdir(dirname.c_str());
}

inline errno_t STRCPY(char* d, size_t n, const char* s) { return strcpy_s(d, n, s); }

inline errno_t STRNCPY(char* d, size_t n, const char* s, size_t x) {
	return strncpy_s(d, n, s, x);
}

inline errno_t STRCAT(char * d, size_t n, const char* s) {
	return strcat_s(d, n, s);
}

inline char* STRDUP(const char* s) { return _strdup(s); }

inline int SPRINTF(char* buf, size_t n, const char * fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int r = vsprintf_s(buf, n, fmt, args);
	va_end(args);
	return r;
}

inline int WSTOI(std::wstring s) { return _wtoi(s.c_str()); }

inline double WSTOD(std::wstring s) { return _wtof(s.c_str()); }

inline int STOI(const char * s) { return (s ? atoi(s) : 0); }

inline double STOD(const char * s) { return (s ? atof(s) : 0.0); }

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

inline std::string STR_DIR_SEP(const std::string pathname)
{
	std::string result = pathname;
	size_t pos = 0;
	while ((pos = result.find("/", pos)) != std::string::npos)
	{
		result.replace(pos, 1, "\\");
		pos++;
	}
	return result;
}

inline bool FIND_FILES_4D(std::wstring path_name,
	std::wstring id, std::vector<std::wstring> &batch_list,
	int &cur_batch)
{
	size_t begin = path_name.rfind(id);
	size_t id_len = id.length();
	if (begin == -1)
		return false;
	else
	{
		std::wstring searchstr = path_name.substr(0, begin);
		searchstr.push_back(L'*');
		std::wstring t_num;
		size_t k;
		bool end_digits = false;
		for (k = begin+id_len; k < path_name.length(); ++k)
		{
			wchar_t c = path_name[k];
			if (iswdigit(c))
			{
				if (end_digits)
					searchstr.push_back(c);
				else
					t_num.push_back(c);
			}
			else if (k == begin + id_len)
				return false;
			else
			{
				end_digits = true;
				searchstr.push_back(c);
			}
		}
		if (t_num.length() == 0)
			return false;
		
		std::wstring search_path = path_name.substr(0,
			path_name.find_last_of(L'\\')) + L'\\';
		WIN32_FIND_DATAW FindFileData;
		HANDLE hFind;
		hFind = FindFirstFileW(searchstr.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			int cnt = 0;
			batch_list.clear();
			std::wstring name = search_path + FindFileData.cFileName;
			batch_list.push_back(name);
			if (name == path_name)
				cur_batch = cnt;
			cnt++;

			while (FindNextFileW(hFind, &FindFileData) != 0)
			{
				name = search_path + FindFileData.cFileName;
				batch_list.push_back(name);
				if (name == path_name)
					cur_batch = cnt;
				cnt++;
			}
		}
		FindClose(hFind);

		return true;
	}
}

inline void FIND_FILES(std::wstring m_path_name,
	std::wstring search_mask,
	std::vector<std::wstring> &m_batch_list,
	int &m_cur_batch)
{
	std::wstring search_path = m_path_name.substr(0,
		m_path_name.find_last_of(L'\\')) + L'\\';
	if (std::string::npos == search_mask.find(m_path_name))
		search_mask = search_path + search_mask;
	WIN32_FIND_DATAW FindFileData;
	HANDLE hFind;
	hFind = FindFirstFileW(search_mask.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		int cnt = 0;
		m_batch_list.clear();
		std::wstring name = search_path + FindFileData.cFileName;
		m_batch_list.push_back(name);
		if (name == m_path_name)
			m_cur_batch = cnt;
		cnt++;

		while (FindNextFileW(hFind, &FindFileData) != 0)
		{
			name = search_path + FindFileData.cFileName;
			m_batch_list.push_back(name);
			if (name == m_path_name)
				m_cur_batch = cnt;
			cnt++;
		}
	}
	FindClose(hFind);
}

inline void SaveConfig(wxFileConfig &file, wxString str)
{
	str = "\x5c\x5c\x3f\x5c" + str;
	wxFileOutputStream os(str);
	file.Save(os);
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

#else // MAC OSX or LINUX

#include <unistd.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/stat.h>

#define GETCURRENTDIR getcwd

#define FSEEK64     fseek

inline wchar_t GETSLASH() { return L'/'; }
inline char GETSLASHA() { return '/'; }

inline bool str_mat(std::wstring &s1, size_t p1, std::wstring &s2, size_t p2)
{
	// If we reach at the end of both strings, we are done
	if (s1[p1] == L'\0' && s2[p2] == L'\0')
		return true;

	// Make sure that the characters after '*' are present 
	// in second string. This function assumes that the first 
	// string will not contain two consecutive '*' 
	if (s1[p1] == L'*' && s1[p1+1] != L'\0' && s2[p2] == L'\0')
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

inline bool STR_MATCH(std::wstring &pattern, std::wstring &search)
{
	return str_mat(pattern, 0, search, 0);
}

inline std::wstring GET_NAME(std::wstring &pathname)
{
	int64_t pos = pathname.find_last_of(GETSLASH());
	if (pos != std::wstring::npos)
		return pathname.substr(pos + 1);
	else
		return pathname;
}

inline std::string GET_NAMEA(std::string &pathname)
{
	int64_t pos = pathname.find_last_of(GETSLASHA());
	if (pos != std::string::npos)
		return pathname.substr(pos + 1);
	else
		return pathname;
}

inline std::wstring GET_PATH(std::wstring &pathname)
{
	int64_t pos = pathname.find_last_of(GETSLASH());
	if (pos != std::wstring::npos)
		return pathname.substr(0, pos + 1);
	else
		return pathname;
}

inline std::string GET_PATHA(std::string &pathname)
{
	int64_t pos = pathname.find_last_of(GETSLASHA());
	if (pos != std::string::npos)
		return pathname.substr(0, pos + 1);
	else
		return pathname;
}

inline bool SEP_PATH_NAME(std::wstring &pathname, std::wstring &path, std::wstring &name)
{
	size_t pos = pathname.find_last_of(GETSLASH());
	if (pos != std::wstring::npos)
	{
		path = pathname.substr(0, pos + 1);
		name = pathname.substr(pos + 1);
		return true;
	}
	else
		return false;
}

inline bool SEP_PATH_NAMEA(std::string &pathname, std::string &path, std::string &name)
{
	size_t pos = pathname.find_last_of(GETSLASHA());
	if (pos != std::string::npos)
	{
		path = pathname.substr(0, pos + 1);
		name = pathname.substr(pos + 1);
		return true;
	}
	else
		return false;
}

inline std::wstring s2ws(const std::string& utf8) {
	//    return std::wstring( str.begin(), str.end() );
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	return converter.from_bytes(utf8);
}

inline std::string ws2s(const std::wstring& utf16) {
	//    return std::string( str.begin(), str.end() );
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	return converter.to_bytes(utf16);
}

inline int SSCANF(const char* buf, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int r = vsscanf(buf, fmt, args);
	va_end(args);
	return r;
}

inline int swprintf_s(wchar_t *buf, size_t n, const wchar_t* fmt, ...) {
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

inline char* STRCAT(char * d, size_t n, const char* s) {
	return strncat(d, s, n - strlen(d) - 1);
}

inline char* STRDUP(const char* s) { return strdup(s); }

inline TIFF* TIFFOpenW(std::wstring fname, const char* opt) {
	return TIFFOpen(ws2s(fname).c_str(), opt);
}

inline int SPRINTF(char* buf, size_t n, const char * fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int r = vsprintf(buf, fmt, args);
	va_end(args);
	return r;
}

inline int WSTOI(std::wstring s) { return atoi(ws2s(s).c_str()); }

inline double WSTOD(std::wstring s) { return atof(ws2s(s).c_str()); }

inline int STOI(const char * s) { return (s ? atoi(s) : 0); }

inline double STOD(const char * s) { return (s ? atof(s) : 0.0); }

inline time_t TIME() { return time(NULL); }

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
} LARGE_INTEGER, *PLARGE_INTEGER;

inline std::string STR_DIR_SEP(const std::string pathname)
{
	std::string result = pathname;
	size_t pos = 0;
	while ((pos = result.find("\\", pos)) != std::string::npos)
	{
		result.replace(pos, 1, "/");
		pos++;
	}
	return result;
}

inline bool FIND_FILES_4D(std::wstring path_name,
	std::wstring id, std::vector<std::wstring> &batch_list,
	int &cur_batch)
{
	int64_t begin = path_name.find(id);
	size_t id_len = id.length();
	if (begin == -1)
		return false;
	else
	{
		std::wstring searchstr = path_name.substr(0, begin);
		std::wstring searchstr2;
		std::wstring t_num;
		size_t k;
		bool end_digits = false;
		for (k = begin + id_len; k < path_name.length(); ++k)
		{
			wchar_t c = path_name[k];
			if (iswdigit(c))
			{
				if (end_digits)
					searchstr.push_back(c);
				else
					t_num.push_back(c);
			}
			else if (k == begin + id_len)
				return false;
			else
			{
				end_digits = true;
				searchstr2.push_back(c);
			}
		}
		if (t_num.length() == 0)
			return false;

		std::wstring search_path = path_name.substr(0,
			path_name.find_last_of(L'/')) + L'/';
		DIR* dir;
		struct dirent *ent;
		if ((dir = opendir(ws2s(search_path).c_str())) != NULL)
		{
			int cnt = 0;
			batch_list.clear();

			while ((ent = readdir(dir)) != NULL)
			{
				std::string file(ent->d_name);
				std::wstring wfile = search_path + s2ws(file);
				//check if it contains the string.
				if (ent->d_name[0] != '.' &&
					wfile.find(searchstr) != std::string::npos &&
					wfile.find(searchstr2) != std::string::npos) {
					std::string ss = ent->d_name;
					std::wstring f = s2ws(ss);
					std::wstring name;
					if (f.find(search_path) == std::string::npos)
						name = search_path + f;
					else
						name = f;
					batch_list.push_back(name);
					if (name == path_name)
						cur_batch = cnt;
					cnt++;
				}
			}
		}
		return true;
	}
}

inline void FIND_FILES(std::wstring m_path_name,
	std::wstring search_mask,
	std::vector<std::wstring> &m_batch_list,
	int &m_cur_batch)
{
	std::wstring search_path = m_path_name.substr(0,
		m_path_name.find_last_of(L'/')) + L'/';
	std::string sspath = ws2s(search_path.c_str());
	DIR* dir = opendir(sspath.c_str());
	if (!dir)
		return;
	int cnt = 0;
	m_batch_list.clear();
	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL)
	{
		std::string file(ent->d_name);
		std::wstring wfile = s2ws(file);
		if (file[0] != '.' &&
			STR_MATCH(search_mask, wfile))
		{
			std::wstring name = search_path + wfile;
			m_batch_list.push_back(name);
			if (name == m_path_name)
				m_cur_batch = cnt;
			cnt++;
		}
	}
	closedir(dir);
}

inline FILE* WFOPEN(FILE ** fp, const wchar_t* filename, const wchar_t* mode) {
	*fp = fopen(ws2s(std::wstring(filename)).c_str(),
		ws2s(std::wstring(mode)).c_str());
	return *fp;
}

inline FILE* FOPEN(FILE ** fp, const char* filename, const char* mode) {
	*fp = fopen(filename, mode);
	return *fp;
}

inline void OutputStreamOpen(std::ofstream &os, std::string fname)
{
	os.open(fname);
}

inline void OutputStreamOpenW(std::wofstream &os, std::wstring fname)
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

inline uint32_t GET_TICK_COUNT() {
	struct timeval ts;
	gettimeofday(&ts, NULL);
	return ts.tv_sec * 1000 + ts.tv_usec / 1000;
}

inline void SaveConfig(wxFileConfig &file, wxString str)
{
	wxFileOutputStream os(str);
	file.Save(os);
}

inline bool IS_NUMBER(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(int(c)); }) == s.end();
}

inline void INC_NUMBER(std::string& s)
{
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

//LINUX SPECIFIC
#ifdef _LINUX
#endif
//MAC OSX SPECIFIC
#ifdef _DARWIN
#include <dlfcn.h>
#endif

#endif //END_IF_DEF__WIN32__

#endif //END__COMPATIBILITY_H__
