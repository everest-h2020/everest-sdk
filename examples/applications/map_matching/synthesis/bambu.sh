#!/bin/bash
script_dir="$(dirname $(readlink -e $0))"

rm -rf kernel_projection.{v,vhd} webtalk* x* .Xil HLS_output panda-temp results.txt \
    simulate_* synthesize_* log.txt array* *.mem BitHeap_* FloPoCo_common.vhdl output.c \
    modelsim.ini transcript temp_verilog* *.nym

bambu \
    ${script_dir}/../mma/mma_xample.cpp \
    ${script_dir}/../mma/component_projection.cpp \
    --top-fname=kernel_projection \
    --compiler=I386_CLANG16 -lm --std=c++14 -O3 \
    -I${script_dir}/../etl/include --generate-interface=INFER \
    --channels-type=MEM_ACC_NN --channels-number=2 \
    --memory-allocation-policy=ALL_BRAM -s -v4 -m64 \
    --device-name=xcu55c-2Lfsvh2892-VVD --clock-period=3 \
    --generate-tb=${script_dir}/../mma/roadnetwork_osm.cpp \
    --generate-tb=${script_dir}/../mma/fcd.cpp \
    --generate-tb=${script_dir}/../mma/component_projection.cpp \
    --generate-tb=${script_dir}/../mma/mma_xample.cpp \
    --simulator=MODELSIM -DCUSTOM_VERIFICATION \
    --tb-arg="-root" --tb-arg=".." --tb-arg="-map" --tb-arg="data/map_o3" \
    --tb-arg="-fcd" --tb-arg="data/fcd_test1.csv" --tb-arg="-out" --tb-arg="output.csv" "$@" |& tee log.txt
