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
#define WSTOD(s)                                _wtof(s)
#define WSTOI(s)                                _wtoi(s)
#define STOD(s)                                 _atof(s)
#define STOI(s)                                 _atoi(s)
#define SPRINTF                                 sprintf_s
#define SSCANF                                  scanf_s
#define STRDUP                                  _strdup
#define STRCPY(a,b,c)                           strcpy_s(a,b,c)
#define STRCAT(a,b,c)                           strcat_s(a,b,c)
#define STRNCPY(a,b,c,d)                        strncpy_s(a,b,c,d)
#define ws2s
#define FOPEN                                   fopen_s
#define WFOPEN                                  _wfopen_s

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

#define SetDoubleBuffered(t)
#define SPRINTF                                    sprintf
#define SSCANF                                     scanf
#define STRDUP                                     strdup
#define STRCPY(a,b,c)                              strcpy(a,c)
#define STRCAT(a,b,c)                              strcat(a,c)
#define STRNCPY(a,b,c,d)                           strncpy(a,c,d)
#define TIFFOpenW(a,b)                             TIFFOpen(ws2s(a).c_str(),b)
#define swprintf_s                                 swprintf
#define WSTOD(s)                                   atof(ws2s(s).c_str())
#define WSTOI(s)                                   atoi(ws2s(s).c_str())
#define STOD(s)                                    atof(s)
#define STOI(s)                                    atoi(s)

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
   *fp = fopen(ws2s(std::wstring(filename)).c_str(),ws2s(std::wstring(mode)).c_str());
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
