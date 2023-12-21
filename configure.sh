#! /bin/bash

mkdir -p build; cmake -DINSTALL_GTEST=OFF -DBUILD_GMOCK=OFF -DDEBUG=ON -S . -B build