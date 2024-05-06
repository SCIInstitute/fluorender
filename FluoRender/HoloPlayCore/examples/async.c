#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <HoloPlayCore.h>

volatile bool done = false;

void cb(hpc_obj obj, hpc_client_error clierr, void *context)
{
    char buf[1500];
    size_t d = hpc_ObjAsJson(obj, buf, 1500);
    printf("%s \nerror: %d\n*context: %d\n", buf, clierr, *(int *)context);
    done = true;
}

int main(int argc, char **argv)
{
    printf("start\n");
    
    hpc_client_error c = hpc_InitializeApp("async", hpc_LICENSE_NONCOMMERCIAL);
    if (c)
    {
        printf("client error %d\n", c);
    } else{
        hpc_obj *info_req = hpc_MakeObject("{\"info\":{}}", 0, NULL);
        int d = 123;
        hpc_client_error c = hpc_SendCallback(info_req, cb, &d);
        if (c)
        {
            printf("client error %d\n", c);
        }
        else
        {
            // wait for a while before exiting, defeating the entire purpose
            // of an async request -- oh well
            unsigned long i = 0;
            while (!done && i < 10000000)
            {
                i++;
            }
            printf("done.\n");
        }
        hpc_CloseApp();
    }
    return 0;
}
