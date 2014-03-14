#!/bin/bash

FILE=.gitignore

while read CMD; do
   if [ "$CMD" != "*.svn" -a "$CMD" != ".git" ]; then
      echo "rm -rf $CMD"
      rm -rf $CMD
   fi
done < "$FILE"
