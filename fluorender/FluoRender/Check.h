#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <string>
#include <sstream>
//#include <windows.h>

using namespace std;

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

string GetAddress()
{
   string result = "";

   PIP_ADAPTER_INFO pAdapterInfo;
   PIP_ADAPTER_INFO pAdapter = NULL;
   DWORD dwRetVal = 0;

   ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
   pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
   if (pAdapterInfo == NULL)
      return result;

   if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
   {
      FREE(pAdapterInfo);
      pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
      if (pAdapterInfo == NULL)
         return result;
   }

   stringstream ss;

   if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
   {
      pAdapter = pAdapterInfo;
      while (pAdapter)
      {
         ss << pAdapter->Description << "\n";
         for (UINT i = 0; i < pAdapter->AddressLength; i++)
            ss << (int)pAdapter->Address[i];
         ss << "\n";
         pAdapter = pAdapter->Next;
      }
   }

   if (pAdapterInfo)
      FREE(pAdapterInfo);

   result = ss.str();

   return result;
}

