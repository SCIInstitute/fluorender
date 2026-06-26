#include "Debug.h"

#ifdef _DEBUG

#include <string>
#include <filesystem>
#include <chrono>
#include <ctime>

#ifdef _WIN32

// ===================== WINDOWS =====================

static HANDLE g_hLogFile = NULL;
static CRITICAL_SECTION g_dbgLock;
static bool g_lockInitialized = false;
static unsigned int dbg_counter = 0;


// ---------- Path helpers ----------

static std::wstring GetLogDirectory()
{
    wchar_t path[MAX_PATH] = { 0 };

    DWORD len = GetEnvironmentVariableW(L"LOCALAPPDATA", path, MAX_PATH);
    if (len > 0)
    {
        return std::wstring(path) + L"\\FluoRender\\";
    }

    return L".\\";
}

static std::wstring GetLogFilePath()
{
    std::wstring dir = GetLogDirectory();
    std::filesystem::create_directories(dir);
    return dir + L"debug.log";
}


// ---------- Init / Shutdown ----------

BOOL DBGLOG_Init()
{
    if (!g_lockInitialized)
    {
        InitializeCriticalSection(&g_dbgLock);
        g_lockInitialized = true;
    }

    std::wstring path = GetLogFilePath();

    g_hLogFile = CreateFileW(
        path.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (g_hLogFile == INVALID_HANDLE_VALUE)
        return FALSE;

    SetFilePointer(g_hLogFile, 0, NULL, FILE_END);

    return TRUE;
}

void DBGLOG_Shutdown()
{
    if (g_hLogFile && g_hLogFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_hLogFile);
        g_hLogFile = NULL;
    }

    if (g_lockInitialized)
    {
        DeleteCriticalSection(&g_dbgLock);
        g_lockInitialized = false;
    }
}


// ---------- Core logging ----------

static void GetTimestamp(wchar_t* buffer, size_t size)
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    StringCchPrintfW(buffer, size,
        L"%04d-%02d-%02d %02d:%02d:%02d.%03d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}


VOID _DBGPRINT(
    LPCWSTR func,
    INT line,
    LPCWSTR fmt,
    ...
)
{
    if (!g_lockInitialized)
        DBGLOG_Init(); // auto-init

    EnterCriticalSection(&g_dbgLock);

    wchar_t timebuf[64];
    GetTimestamp(timebuf, 64);

    va_list args;
    va_start(args, fmt);

    int prefixLen = _scwprintf(
        L"%s [%07d] [%s:%d] ",
        timebuf, dbg_counter, func, line);

    int msgLen = _vscwprintf(fmt, args);

    size_t totalChars = prefixLen + msgLen + 2;

    wchar_t* buffer = (wchar_t*)_malloca(totalChars * sizeof(wchar_t));

    if (buffer)
    {
        swprintf(buffer, totalChars,
            L"%s [%07d] [%s:%d] ",
            timebuf, dbg_counter, func, line);

        size_t offset = wcslen(buffer);

        vswprintf(buffer + offset,
            totalChars - offset,
            fmt,
            args);

        // Debugger
        OutputDebugStringW(buffer);

        // File
        if (g_hLogFile && g_hLogFile != INVALID_HANDLE_VALUE)
        {
            DWORD written = 0;
            WriteFile(g_hLogFile,
                buffer,
                (DWORD)(wcslen(buffer) * sizeof(wchar_t)),
                &written,
                NULL);

            FlushFileBuffers(g_hLogFile);
        }

        _freea(buffer);
    }

    dbg_counter++;

    va_end(args);

    LeaveCriticalSection(&g_dbgLock);
}

#else

// ===================== LINUX / MAC =====================

#include <fstream>
#include <mutex>
#include <cstdarg>

static std::wofstream g_logFile;
static std::mutex g_logMutex;
static unsigned int dbg_counter = 0;


// ---------- Path helpers ----------

static std::wstring GetLogDirectory()
{
    const char* xdg = getenv("XDG_STATE_HOME");
    if (xdg && *xdg)
    {
        return std::filesystem::path(xdg) / "FluoRender";
    }

    const char* home = getenv("HOME");
    if (!home)
        return L"./";

#ifdef __APPLE__
    return std::filesystem::path(home) / "Library/Logs/FluoRender";
#else
    return std::filesystem::path(home) / ".local/state/FluoRender";
#endif
}

static std::wstring GetLogFilePath()
{
    std::filesystem::path dir = GetLogDirectory();
    std::filesystem::create_directories(dir);
    return (dir / "debug.log").wstring();
}


// ---------- Init / Shutdown ----------

bool DBGLOG_Init()
{
    std::wstring path = GetLogFilePath();
    g_logFile.open(std::filesystem::path(path), std::ios::app);
    return g_logFile.is_open();
}

void DBGLOG_Shutdown()
{
    if (g_logFile.is_open())
        g_logFile.close();
}


// ---------- Timestamp ----------

static void GetTimestamp(wchar_t* buffer, size_t size)
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm* tm_info = std::localtime(&t);
    wcsftime(buffer, size, L"%Y-%m-%d %H:%M:%S", tm_info);
}


// ---------- Core logging ----------

void DBGPRINT(const wchar_t* format, ...)
{
    std::lock_guard<std::mutex> lock(g_logMutex);

    wchar_t timebuf[64];
    GetTimestamp(timebuf, 64);

    wchar_t msg[2048];

    va_list args;
    va_start(args, format);
    vswprintf(msg, 2048, format, args);
    va_end(args);

    wchar_t finalMsg[2300];

    swprintf(finalMsg, 2300,
        L"%s [%07d] %s",
        timebuf,
        dbg_counter,
        msg);

    // Console
    wprintf(L"%s", finalMsg);

    // File
    if (g_logFile.is_open())
    {
        g_logFile << finalMsg;
        g_logFile.flush();
    }

    dbg_counter++;
}

#endif

#endif // _DEBUG