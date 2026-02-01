#!/bin/bash

mkdir build
cd build

cmake .. -DBUILD_TEST=OFF
make -j4
