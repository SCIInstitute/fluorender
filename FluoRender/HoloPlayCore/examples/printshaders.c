#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "HoloPlayShaders.h"

int main(int argc, char **argv)
{
    printf("========Vertex shader=======\n");
    printf("%s\n", hpc_LightfieldVertShaderGLSL);
    printf("=======Fragment shader=======\n");
    printf("%s\n", hpc_LightfieldFragShaderGLSL);
    return 0;
}
