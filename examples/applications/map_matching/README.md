Map Matching Example
=================================

In this folder, we prepared an example on how to use the EVEREST SDK to accelerate a sub-kernel of a map matching application.

## Building and testing the software version

Make sure that the etl submodule has been initialized, then compile the application using the provided CMake build:

```
mkdir build
cd build
cmake ..
make
```

Run the application from the build directory with the provided test data:

```
./mma_xample -root .. -map data/map_o3 -fcd data/fcd_test1.csv -out output.csv
```

## Adapting the code for Bambu synthesis

You can inspect the code to understand how the original C++ application has been adapted in preparation for High-Level Synthesis (HLS) with the Bambu tool in the EVEREST SDK. In particular:
- The `mma_static` executable replaces STL containers with statically-sized ETL containters.
- Floating-point computation has been replaced where possible with fixed-point computation through the Bambu ac_types library.
- Interface pragmas have been added to the function that will be synthesized to request an AXI4 interface.
- Before calling the kernel, a function call is added to allocate the correct amount of space to arguments that are passed by reference (required for co-simulation).

## HLS, co-simulation, and implementation of the kernel

To synthesize the `kernel_projection` function into a Verilog accelerator, run Bambu:

```
cd synthesis/
bash bambu.sh
```

(The script assumes that Bambu is available on `$PATH`.)

The co-simulation process allows to verify that the generated Verilog code produces the same results of the original software application. If you want to sun co-simulation as well, add the `--simulate` option to the Bambu command in the script (requires a simulator to be installed). After co-simulation, the results written in `output.csv` are the ones coming from the Verilog simulation, and they can be compared with the output produced by the software.

If you also want to obtain area consumption metrics, you can run logic synthesis and implementation by replicing `--simulate` with `--evaluation` (requires backend Xilinx tools installed, if they are not in /opt/Xilinx you also have to add the `--xilinx-root=<path`> option).

Note: both co-simulation and implementation can take a significant amount of time. To reduce the number of times the kernel is simulated you can change `MAX_MATCH` to a lower number in `mma_xample.cpp`.