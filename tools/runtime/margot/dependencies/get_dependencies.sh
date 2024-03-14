#!/bin/bash

echo $'\e[1;33m'Installing Boost...$'\e[0m'
wget http://downloads.sourceforge.net/project/boost/boost/1.60.0/boost_1_60_0.tar.gz
tar xfz boost_1_60_0.tar.gz
rm boost_1_60_0.tar.gz
cd boost_1_60_0
./bootstrap.sh --with-libraries=program_options --prefix="${PWD}/install"
./b2 install
cd ..

echo $'\e[1;33m'Installing paho.mqtt.c...$'\e[0m'
git clone https://github.com/eclipse/paho.mqtt.c
cd paho.mqtt.c
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${PWD}/../install ..
make install
cd ../..

echo $'\e[1;33m'Installing nlohmann_json...$'\e[0m'
git clone https://github.com/nlohmann/json
cd json
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${PWD}/../install ..
make install
cd ../..