#!/bin/bash

find ./include ./src ./test -type f -name "*.[ch]pp" > ./format-files.txt

clang-format-14 --files=format-files.txt --style=file -i

rm -rf ./format-files.txt
