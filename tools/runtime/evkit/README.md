# evkit
Distributed runtime for the Everest project.

## Installation
Clone this repository recursively:
```bash
$ git clone --recursive https://code.it4i.cz/everest/evkit.git
```

Install required third-party dependencies:
- CMake
- Rust compiler (Cargo): https://www.rust-lang.org/tools/install
- HDF5 (C++): for alternatives
- OpenCL: for PTDR on FPGA

CMake variables for the `binding` project:

| **Variable**            | **Default** | **Description**                                  |
|-------------------------|-------------|--------------------------------------------------|
| `-DPTDR_KERNEL`         | `OFF`       | Build PTDR kernel library.                       |
| `-DALTERNATIVES_KERNEL` | `OFF`       | Build Alternatives kernel library.               |
| `-DPTDR_CPU_BENCHMARK`  | `OFF`       | Build PTDR CPU benchmark.                        |
| `-DENERGY_PROFILING`    | `OFF`       | Measure energy usage for the PTDR CPU benchmark. |
| `-DPTDR_FPGA_BENCHMARK` | `OFF`       | Build PTDR FPGA benchmark.                       |

## Usage
1. Build the project. By default, both alternatives and PTDR is built.
    ```bash
    $ cargo build --release
    ```
    If you want to build only alternatives or only PTDR, you can pass the `alternatives` or `ptdr` features:
    ```bash
    $ cargo build --release --features alternatives
    ```

2. Start the worker
    ```bash
    $ cargo run --release -- run <compute-port> <management-port>
    ```
    Compute and management ports should be chosen by the traffic simulator.
    The available kernels of the worker are decided by the enabled Cargo features.

### PTDR
1. Compile/run the binary with `--features ptdr`.
2. Pass the path to a MsgPack PTDR file. If you want to run on an FPGA, also pass the path to a FPGA bitstream.
   ```bash
   $ cargo run --release --features ptdr -- run <compute-address:port> <management-address:port> --ptdr-profiles <msgpack-profiles-path> \
      [--ptdr-bitstream <fpga-bitstream-path>]
   ```
