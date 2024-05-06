#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <HoloPlayCore.h>

int main(int argc, char **argv)
{
    hpc_client_error errco;
    if (!(errco = hpc_InitializeApp("minimal.c", hpc_LICENSE_NONCOMMERCIAL)))
    {
        char buf[1500];
        size_t d = hpc_GetStateAsJSON(buf, 1500);
        // if we haven't allocated enough space for the string, try again
        if (d)
        {
            char *buf2 = (char *)malloc(d);
            size_t c = hpc_GetStateAsJSON(buf2, d);
            printf("%s\n", buf2);
            free(buf2);
        }
        else
        {
            printf("%s\n", buf);
        }
        hpc_GetHoloPlayServiceVersion(buf, 1500);
        printf("Version %s\n", buf);
        printf("%d devices.\n", hpc_GetNumDevices());
    }
    else
    {
        printf("Error connecting to HoloPlay Service (code %d).\n", errco);
    }
    hpc_CloseApp();
    return 0;
}
