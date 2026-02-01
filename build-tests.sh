#!/bin/bash

mkdir build
cd build

cmake .. -DBUILD_TEST=ON
make -j4
