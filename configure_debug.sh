#! /bin/bash

mkdir -p build; cmake -DINSTALL_GTEST=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build