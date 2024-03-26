#!/bin/bash

$EVEREST_PWD/tools/compilation/bambu/bambu \
    compute_duration.cpp --top-fname=kernel_compute_duration \
    --compiler=I386_CLANG12 --std=c++14 -O3 \
    --channels-type=MEM_ACC_NN --channels-number=2 --generate-interface=INFER \
    --memory-allocation-policy=ALL_BRAM -s \
    --device-name=xcu280-2Lfsvh2892-VVD --clock-period=5 -m64 \
    "$@" |& tee log.txt
