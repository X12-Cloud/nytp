#!/bin/bash

echo "Removing existing build directory"
rm -rf build

echo "Performing a clean build of nytp"
cmake -B build
cmake --build build
