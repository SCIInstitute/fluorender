#!/bin/bash

for file in *.png 
do
   ./png2c.py ${file} > ${file:0:${#file}-4}.h 
   #rm ${file:0:${#file}-4}
   #echo "${file} : ${file:0:${#file}-4} "
done
