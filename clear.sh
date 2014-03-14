#!/bin/bash

rm -rf *.cmake */*.cmake */*/*.cmake */*/*/*.cmake */*/*/*/*.cmake
rm -rf Makefile */Makefile */*/Makefile */*/*/Makefile */*/*/*/Makefile
rm -rf CMakeFiles */CMakeFiles */*/CMakeFiles */*/*/CMakeFiles */*/*/*/CMakeFiles
rm -rf CMakeCache.txt */CMakeCache.txt */*/CMakeCache.txt */*/*/CMakeCache.txt */*/*/*/CMakeCache.txt

