#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "HoloPlayCore.h"

int main(int argc, char **argv)
{
	hpc_client_error errco = hpc_InitializeApp("HoloPlayInfo.c", hpc_LICENSE_NONCOMMERCIAL);
	if (errco)
	{
		char *errstr;
		switch (errco)
		{
		case hpc_CLIERR_NOSERVICE:
			errstr = "HoloPlay Service not running";
			break;
		case hpc_CLIERR_SERIALIZEERR:
			errstr = "Client message could not be serialized";
			break;
		case hpc_CLIERR_VERSIONERR:
			errstr = "Incompatible version of HoloPlay Service";
			break;
		case hpc_CLIERR_PIPEERROR:
			errstr = "Interprocess pipe broken";
			break;
		case hpc_CLIERR_SENDTIMEOUT:
			errstr = "Interprocess pipe send timeout";
			break;
		case hpc_CLIERR_RECVTIMEOUT:
			errstr = "Interprocess pipe receive timeout";
			break;
		default:
			errstr = "Unknown error";
			break;
		}
		printf("Client access error (code = %d): %s!\n", errco, errstr);
	}
	else
	{
		char buf[1000];
		hpc_GetHoloPlayCoreVersion(buf, 1000);
		printf("HoloPlay Core version %s.\n", buf);
		hpc_GetHoloPlayServiceVersion(buf, 1000);
		printf("HoloPlay Service version %s.\n", buf);
		int num_displays = hpc_GetNumDevices();
		printf("%d device%s connected.\n", num_displays, (num_displays == 1 ? "" : "s"));
		for (int i = 0; i < num_displays; ++i)
		{
			printf("Device information for display %d:\n", i);
			hpc_GetDeviceHDMIName(i, buf, 1000);
			printf(" Device name: %s\n", buf);
			hpc_GetDeviceType(i, buf, 1000);
			printf(" Device type: %s\n", buf);
			int b0 = hpc_GetDevicePropertyInt(i, "/buttons/0");
			int b1 = hpc_GetDevicePropertyInt(i, "/buttons/1");
			int b2 = hpc_GetDevicePropertyInt(i, "/buttons/2");
			int b3 = hpc_GetDevicePropertyInt(i, "/buttons/3");
			printf(" Button status: %d %d %d %d", b0, b1, b2, b3);
			printf("\nWindow parameters for display %d:\n", i);
			printf(" Position: (%d, %d)\n", hpc_GetDevicePropertyWinX(i), hpc_GetDevicePropertyWinY(i));
			printf(" Size: (%d, %d)\n", hpc_GetDevicePropertyScreenW(i), hpc_GetDevicePropertyScreenH(i));
			printf(" Aspect ratio: %f\n", hpc_GetDevicePropertyDisplayAspect(i));
			printf("Shader uniforms for display %d:\n", i);
			printf(" pitch: %.9f\n", hpc_GetDevicePropertyPitch(i));
			printf(" tilt: %.9f\n", hpc_GetDevicePropertyTilt(i));
			printf(" center: %.9f\n", hpc_GetDevicePropertyCenter(i));
			printf(" subp: %.9f\n", hpc_GetDevicePropertySubp(i));
			printf(" fringe: %.1f\n", hpc_GetDevicePropertyFringe(i));
			printf(" RI: %d\n BI: %d\n invView: %d\n", hpc_GetDevicePropertyRi(i), hpc_GetDevicePropertyBi(i), hpc_GetDevicePropertyInvView(i));
		}
	}
	hpc_CloseApp();
	return 0;
}
