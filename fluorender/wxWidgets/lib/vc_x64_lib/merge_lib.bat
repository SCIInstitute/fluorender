@ECHO OFF
REM bash equivalent for windows script
REM #!/bin/bash
REM 
REM if [ ! -f wxmsw30ud_core.lib ];
REM then
REM 
REM   cat core1.libpart > wxmsw30ud_core.lib
REM   cat core2.libpart >> wxmsw30ud_core.lib
REM   cat core3.libpart >> wxmsw30ud_core.lib
REM fi

COPY core*.libpart /B wxmsw30ud_core.lib /B
