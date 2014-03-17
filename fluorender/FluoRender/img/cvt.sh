#!/bin/bash

for file in *.png 
do
   ~/Downloads/wxWidgets-3.0.0/misc/scripts/png2c.py ${file} > ${file:0:${#file}-4}.h 
   #rm ${file:0:${#file}-4}
   #echo "${file} : ${file:0:${#file}-4} "
done
