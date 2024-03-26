#!/bin/bash

$EVEREST_PWD/tools/compilation/bambu/bambu --compiler=I386_CLANG12 helmholtz.ll \
        --device-name=xcu280-2Lfsvh2892-VVD --clock-period=5 \
        --top-fname=kernel --channels-type=MEM_ACC_11
