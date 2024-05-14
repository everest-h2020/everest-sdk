# Everest SDK

## The EVEREST project

Scope, aim, results, links to website and publications

## The System Development Kit

General introduction

![SDK diagram](sdk.png)

## Structure of the repository

The SDK repository is structured as follows:

- examples: this folder contains application examples that showcase how to use different tools on real-world use cases;
- tests: this folder contains small tests that verify if a tool has been installed correctly;
- tools: this folder contains the SDK tools, further divided into compilation, runtime, services, and basecamp.


## Getting started

### General configuration

Make sure to include all submodules through `git clone --recursive` or `git submodule init` and `git submodule update`.

Run the command `. set_env.sh` before proceeding.

### Installation

Each tool provides dedicated installation instructions, and many of them also available as Docker containers.

The EVEREST SDK relies on a specific revision of the MLIR and CIRCT projects.
The easiest way to obtain and build these is by executing the script `build_mlir.sh`, which will fetch and build both projects.
Be advised that during the compilation process, up to 150GB of space are required temporarily for build artifacts.

### Basecamp

One way getting started is to use the [tool `basecamp`](https://github.com/everest-h2020/everest-basecamp), which is the facade for most tools of the EVEREST SDK. The SDK is modular in nature and basecamp is a unified interface for these modules. The modular structure could simplify the installation, depending on the use case. 
Basecamp and its setup instruction can be found under `tools/basecamp`. 

## Acknowledgements

![Acknowledgements](eu_banner.png)
