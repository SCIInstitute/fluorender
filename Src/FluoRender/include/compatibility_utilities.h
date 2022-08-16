/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#ifndef __COMPATIBILITY_UTILITIES_H__
#define __COMPATIBILITY_UTILITIES_H__

#include <string>
#include <cstring>
#include <fstream>
#include <locale>
#include <vector>
#include <codecvt>

#include <tiffio.h>
#include <dirent.h>

#ifdef _WIN32 // WINDOWS ONLY

#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <cctype>
#include <cwctype>
#include <ctype.h>
#include <sys/types.h>

#define GETCURRENTDIR _getcwd

#define FSEEK64     _fseeki64
#define SSCANF    sscanf

inline char GETSLASHA() { return '\\'; }
inline char GETSLASHALTA() { return '/'; }
inline wchar_t GETSLASH() { return L'\\'; }
inline wchar_t GETSLASHALT() { return L'/'; }

inline std::string GET_SUFFIX(const std::string &pathname)
{
	int64_t pos = pathname.find_last_of('.');
	if (pos != std::string::npos)
		return pathname.substr(pos);
	else
		return "";
}

inline std::string GET_NAME(const std::string &pathname)
{
	int64_t pos1 = pathname.find_last_of(GETSLASHA());
	int64_t pos2 = pathname.find_last_of(GETSLASHALTA());
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

inline std::string GET_PATH(const std::string &pathname)
{
	int64_t pos1 = pathname.find_last_of(GETSLASHA());
	int64_t pos2 = pathname.find_last_of(GETSLASHALTA());
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

inline std::wstring GET_SUFFIX(const std::wstring &pathname)
{
	int64_t pos = pathname.find_last_of(L'.');
	if (pos != std::wstring::npos)
		return pathname.substr(pos);
	else
		return L"";
}

inline std::wstring GET_NAME(const std::wstring &pathname)
{
	int64_t pos1 = pathname.find_last_of(GETSLASH());
	int64_t pos2 = pathname.find_last_of(GETSLASHALT());
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

inline std::wstring GET_PATH(const std::wstring &pathname)
{
	int64_t pos1 = pathname.find_last_of(GETSLASH());
	int64_t pos2 = pathname.find_last_of(GETSLASHALT());
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

inline bool SEP_PATH_NAME(const std::wstring &pathname, std::wstring &path, std::wstring &name)
{
	int64_t pos1 = pathname.find_last_of(GETSLASH());
	int64_t pos2 = pathname.find_last_of(GETSLASHALT());
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
		search_mask = m_path_name + search_mask;
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

inline bool FILE_EXISTS(const std::string& name)
{
	if (FILE *file = fopen(name.c_str(), "r"))
	{
		fclose(file);
		return true;
	}
	else
		return false;
}

inline bool FILE_EXISTS(const std::wstring& name)
{
	if (FILE *file = _wfopen(name.c_str(), L"r"))
	{
		fclose(file);
		return true;
	}
	else
		return false;
}

inline bool DIR_EXISTS(const std::string& name)
{
	const std::filesystem::path p(name);
	return std::filesystem::exists(p);
}

inline bool DIR_EXISTS(const std::wstring& name)
{
	const std::filesystem::path p(name);
	return std::filesystem::exists(p);
}

inline void SLEEP(unsigned long t)
{
	Sleep(t);
}

#else // MAC OSX or LINUX
#include <iostream>
#include <filesystem>

#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>

#define GETCURRENTDIR getcwd

#define FSEEK64     fseek

inline char GETSLASHA() { return '/'; }
inline wchar_t GETSLASH() { return L'/'; }

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

inline std::wstring GET_SUFFIX(const std::wstring &pathname)
{
	int64_t pos = pathname.find_last_of(L'.');
	if (pos != std::wstring::npos)
		return pathname.substr(pos);
	else
		return L"";
}

inline std::string GET_SUFFIX(const std::string &pathname)
{
    int64_t pos = pathname.find_last_of('.');
    if (pos != std::string::npos)
        return pathname.substr(pos);
    else
        return "";
}

inline std::wstring GET_NAME(std::wstring &pathname)
{
	int64_t pos = pathname.find_last_of(GETSLASH());
	if (pos != std::wstring::npos)
		return pathname.substr(pos + 1);
	else
		return pathname;
}

inline std::wstring GET_PATH(const std::wstring &pathname)
{
	int64_t pos = pathname.find_last_of(GETSLASH());
	if (pos != std::wstring::npos)
		return pathname.substr(0, pos + 1);
	else
		return pathname;
}

inline bool SEP_PATH_NAME(std::wstring &pathname, std::wstring &path, std::wstring &name)
{
	int64_t pos = pathname.find_last_of(GETSLASH());
	if (pos != std::wstring::npos)
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

inline bool FILE_EXISTS(const std::string& name)
{
	const std::filesystem::path p(name);
	return std::filesystem::exists(p);
}

inline bool FILE_EXISTS(const std::wstring& name)
{
	const std::filesystem::path p(name);
	return std::filesystem::exists(p);
}

inline bool DIR_EXISTS(const std::string& name)
{
	const std::filesystem::path p(name);
	return std::filesystem::exists(p);
}

inline bool DIR_EXISTS(const std::wstring& name)
{
	const std::filesystem::path p(name);
	return std::filesystem::exists(p);
}

inline void SLEEP(unsigned long t)
{
	usleep(t * 1000);
}

// LINUX SPECIFIC
#ifdef _LINUX
#endif
// MAC OSX SPECIFIC
#ifdef __APPLE__
#include <dlfcn.h>
#endif

#endif // end _WIN32

#endif // end __COMPATIBILITY_UTILITIES_H__
