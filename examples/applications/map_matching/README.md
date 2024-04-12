Map Matching Example
=================================

In this folder, we prepared an example on how to use the EVEREST SDK to accelerate a sub-kernel of a map matching application.

### Building and testing the software version

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