# Multi-versioned Helmholtz integrated with mARGOt

This folder contains an example of a multi-versioned library featuring mARGOt generated using the scripts provided in the flow folder.
It requires __OpenCL__, __XRT__ and __Vivado__ to build (due to the fpga version, the naive one does not require additional components).
To build and install it, run the following commands, after changing the files _fpga/fpga.hpp_, _naive/naive.hpp_, and _helmholtz.h_ the path to the included header _inverse\_helmholtz.h_:

```
mkdir build
cd build
cmake ${OPTIONS_CMAKE} \
  -DCMAKE_INSTALL_PREFIX=../install \
  -DXILINX_XRT=<LOCATION_OF_XRT_INSTALLATION> \
  -DXILINX_VIVADO=<LOCATION_OF_VIVADO_INSTALLATION> \
  ..
make
make install
```

The code features two different versions:

- The _naive_ version executes on cpu and is the default one;
- The _fpga_ version runs on fpga (if present).

The File provided for the generation of all the sources are:

- _problem.json_, that specifies the interface the host application expects from the multi-versioned library;
- _version.json_, that describes the available versions.
- _agora.json_, that contains some additional configuration for the interaction with AGORA and Theo.
