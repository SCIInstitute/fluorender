/**
 * This file is used for compatibility across windows and mac/linux platforms.
 * This is specific to FLuoRender Code.
 * @author Brig Bagley
 * @version 4 March 2014
 */
#ifndef __COMPATIBILITY_H__
#define __COMPATIBILITY_H__

#ifdef _WIN32 //WINDOWS ONLY

#include <windows.h>
#include <ole2.h>
#include <wintab.h>
#include <pktdef.h>
#include "WacUtils/WacUtils.h"
#include <stdarg.h>

inline std::wstring ws2s(std::wstring s) { return s; }

inline int SSCANF(const char* buf, const*fmt, ...){
   va_list args;
   va_start(args,fmt);
   int r = vsscanf_s(buf,fmt,args);
   va_end(args);
   return r;
}

inline errno_t FOPEN(FILE** fp, const char *fname, const char* mode) {
   fopen_s(fp,fname,mode);
}

inline errno_t WFOPEN(FILE** fp, const wchar_t* fname, const wchar_t* mode) {
   _wfopen_s(fp,fname,mode);
}

inline errno_t STRCPY(char* d, size_t n, const char* s) { return strcpy_s(d,n,s); }

inline errno_t STRNCPY(char* d, size_t n, const char* s, size_t x) { 
   return strncpy_s(d,n,s,x); 
}

inline errno_t STRCAT(char * d, size_t n, const char* s) { 
   return strcat(d,n,s);
}

inline char* STRDUP(const char* s) { return _strdup(s); }

inline int SPRINTF(char* buf, size_t n, const char * fmt, ...) {
   va_list args;
   va_start(args,fmt);
   int r = vsprintf_s(buf,n,fmt,args);
   va_end(args);
   return r;
}

inline int WSTOI(std::wstring s) { return _wtoi(s.c_str()); }

inline double WSTOD(std::wstring s) { return _wtof(s.c_str()); }

inline int STOI(const char * s) { return _atoi(s); }

inline double STOD(const char * s) { return _atof(s); }

inline time_t TIME(time_t* n) { return _time32(n); }

inline int CREATE_DIR(const char *f) { return CreateDirectory(f,NULL); }

inline uint32_t GET_TICK_COUNT() { return GetTickCount(); }

inline void FIND_FILES(std::wstring m_path_name,
      std::wstring search_ext,
      std::vector<std::wstring> &m_batch_list,
      int &m_cur_batch, std::wstring regex = L"") {
   std::wstring search_path = m_path_name.substr(0,
         m_path_name.find_last_of(L'\\')) + L'\\';
   std::wstring search_str = regex + L"*" + search_ext;
   WIN32_FIND_DATAW FindFileData;
   HANDLE hFind;
   hFind = FindFirstFileW(search_str.c_str(), &FindFileData);
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

#else // MAC OSX or LINUX

#include <vector>
#include <codecvt>
#include <iostream>
#include <dirent.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <../LibTiff/tiffio.h>

inline std::wstring s2ws(const std::string& str) {
   typedef std::codecvt_utf8<wchar_t> convert_typeX;
   std::wstring_convert<convert_typeX, wchar_t> converterX;
   return converterX.from_bytes(str);
}

inline std::string ws2s(const std::wstring& str) {
   typedef std::codecvt_utf8<wchar_t> convert_typeX;
   std::wstring_convert<convert_typeX, wchar_t> converterX;
   return converterX.to_bytes(str);
}

inline int SSCANF(const char* buf, const char* fmt, ...){
   va_list args;
   va_start(args,fmt);
   int r = vsscanf(buf,fmt,args);
   va_end(args);
   return r;
}

inline int swprintf_s(wchar_t *buf, size_t n, const wchar_t* fmt, ...) {
   va_list args;
   va_start(args,fmt);
   int r = vswprintf(buf,n,fmt,args);
   va_end(args);
   return r;
}

inline void SetDoubleBuffered(bool) {};

inline char* STRCPY(char* d, size_t n, const char* s) { return strncpy(d,s,n-1); }

inline char* STRNCPY(char* d, size_t n, const char* s, size_t x) { 
   return strncpy(d,s,n-1); 
}

inline char* STRCAT(char * d, size_t n, const char* s) { 
   return strncat(d,s,n-strlen(d)-1);
}

inline char* STRDUP(const char* s) { return strdup(s); }

inline TIFF* TIFFOpenW(std::wstring fname, const char* opt) {
   return TIFFOpen(ws2s(fname).c_str(),opt);
}

inline int SPRINTF(char* buf, size_t n, const char * fmt, ...) {
   va_list args;
   va_start(args,fmt);
   int r = vsprintf(buf,fmt,args);
   va_end(args);
   return r;
}

inline int WSTOI(std::wstring s) { return atoi(ws2s(s).c_str()); }

inline double WSTOD(std::wstring s) { return atof(ws2s(s).c_str()); }

inline int STOI(const char * s) { return atoi(s); }

inline double STOD(const char * s) { return atof(s); }

inline time_t TIME(time_t* n) { return time(n); }

inline int CREATE_DIR(const char *f) { return mkdir(f, S_IRWXU | S_IRGRP | S_IXGRP); }

typedef union _LARGE_INTEGER {
   struct {
      unsigned int LowPart;
      long HighPart;
   };
   struct {
      unsigned int LowPart;
      long HighPart;
   } u;
   long long QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

inline void FIND_FILES(std::wstring m_path_name,
      std::wstring search_ext,
      std::vector<std::wstring> &m_batch_list,
      int &m_cur_batch, std::wstring regex = L"") {
   std::wstring search_path = m_path_name.substr(0,m_path_name.find_last_of(L'/')) + L'/';
   std::wstring search_str(L".txt");
   DIR* dir;
   struct dirent *ent;
   if ((dir = opendir(ws2s(search_path).c_str())) != NULL) {
      int cnt = 0;
      m_batch_list.clear();
      while((ent = readdir(dir)) != NULL) {
         //check if it contains the string.
         if (strstr(ent->d_name,ws2s(search_str).c_str()) &&
               strstr(ent->d_name,ws2s(regex).c_str())) {
            std::string ss = ent->d_name;
            std::wstring f = s2ws(ss);
            std::wstring name = search_path + f;
            m_batch_list.push_back(name);
            if (name == m_path_name)
               m_cur_batch = cnt;
            cnt++;
         }
      }
   }
}

inline FILE* WFOPEN(FILE ** fp, const wchar_t* filename, const wchar_t* mode) {
   *fp = fopen(ws2s(std::wstring(filename)).c_str(),
         ws2s(std::wstring(mode)).c_str());
   return *fp;
}

inline FILE* FOPEN(FILE ** fp, const char* filename, const char* mode) {
   *fp = fopen(filename,mode);
   return *fp;
}

inline uint32_t GET_TICK_COUNT() {
   struct timeval ts;
   gettimeofday(&ts, NULL);
   return ts.tv_sec * 1000 + ts.tv_usec / 1000;
}

//LINUX SPECIFIC
#ifdef _LINUX
#endif
//MAC OSX SPECIFIC
#ifdef _DARWIN
#endif

#endif //END_IF_DEF__WIN32__

#endif //END__COMPATIBILITY_H__
