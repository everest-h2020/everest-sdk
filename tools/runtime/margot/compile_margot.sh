#!/bin/bash

cd core
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${PWD}/../install \
  ${OPTIONS_CMAKE} ..
make
make install