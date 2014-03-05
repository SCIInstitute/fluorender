/**
 * This file is used for compatibility across windows and mac/linux platforms.
 * @author Brig Bagley
 * @version 4 March 2014
 */
#ifdef _WIN32
#include <windows.h>
#include <ole2.h>
#define WSTOD(s)                                _wtof(s)
#define WSTOI(s)                                _wtoi(s)
#define CREATE_DIR(f)                           CreateDirectory(f, NULL)
#define SSCANF(a,b,c,d,e,f)                     sscanf_s(a,b,c,d,e,f)
//#define GET_WM_ACTIVATE_STATE(wp, lp)           LOWORD(wp)
//#define GET_WM_COMMAND_ID(wp, lp)               LOWORD(wp)
//#define GET_WM_COMMAND_HWND(wp, lp)             (HWND)(lp)
//#define GET_WM_COMMAND_CMD(wp, lp)              HIWORD(wp)
//#define FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, fn) \
//   (void)(fn)((hwnd), WM_COMMAND, MAKEWPARAM((UINT)(id),(UINT)(codeNotify)), (LPARAM)(HWND)(hwndCtl))
/* -------------------------------------------------------------------------- */
#else
#include <cstdlib>
#define WSTOD(s)                                   atof(s)
#define WSTOI(s)                                   atoi(s)
#define DWORD                                      unsigned int
#define CREATE_DIR(f)                              mkdir(f, S_IRWXU | S_IRGRP | S_IXGRP)
#define SSCANF(a,b,c,d,e,f)                        sscanf(a,b,c,d,e,f)
//#define GET_WM_ACTIVATE_STATE(wp, lp)               (wp)
//#define GET_WM_COMMAND_ID(wp, lp)                   (wp)
//#define GET_WM_COMMAND_HWND(wp, lp)                 (HWND)LOWORD(lp)
//#define GET_WM_COMMAND_CMD(wp, lp)                  HIWORD(lp)
//#define FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, fn) \
//   (void)(fn)((hwnd), WM_COMMAND, (WPARAM)(int)(id), MAKELPARAM((UINT)(hwndCtl), (codeNotify)))
/* -------------------------------------------------------------------------- */
#endif
