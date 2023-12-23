#! /bin/bash

mkdir -p build; cmake -DINSTALL_GTEST=OFF -DCMAKE_BUILD_TYPE=Debug -S . -B build