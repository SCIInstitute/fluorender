/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

