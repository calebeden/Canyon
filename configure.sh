#! /bin/bash

mkdir -p build; cmake -DINSTALL_GTEST=OFF -DBUILD_GMOCK=OFF -S . -B build