# mARGOt framework repository

The mARGOt autotuning framework provides to the EVEREST runtime toolkit a mechanism to introduce application adaptability. Through the usage of the mARGOt framework components, applications can select the best configuration for some runtime parameters during the entire execution, adapting to changes in the hardware available and in the behaviour of the system.

## Structure

- Folder __core__ contains the mARGOt library implementation and the AGORA server application. It requires _Boost_, _paho.mqtt.c_ and _CMake_ (3.10 minimum) to build. Additional informaion can be found within the documentation and the READMEs in the folder.
- Folder __theo__ contains the Theo runtime observer sources. All the requirements are specified in the _requirements.txt_ file. Check the README inside the folder for additional details about the execution.
- Folder __adam__ contains the mARGOt interface for other programming languages (different from C++). Additional details can be found within the README in the folder.
- Folder __brian__ contains the broker that automatically creates adam instances for non-C++ applications. Additional details can be found within the README inside the folder.
- Folder __dependencies__ contains the script needed to build the dependencies of mARGOt.
- Folder __multi-versioned\_generation__ contains the scripts needed to manage the generation of the multi-versioned kernels.

## Requirements

The framework has been tested on Ubuntu 20.04 LTS and compiled with gcc 9.4.0. It requires CMake (version 3.10 minimum) for its compilation. The AGORA module requires also Python 3.8 to work.
We also provided Docker containers for the deployment of the components of the mARGOt framework, in particular:

- __margotpolimi/margot:1.0__ is a Docker image that contains a pre_compiled version of mARGOt. It can serve as a starting point for the development of an application using the mARGOt library.
- __margotpolimi/agora:1.0__ is a Docker image that runs the AGORA component. It requires access to the chosen MQTT broker (defaults to "127.0.0.1" but can be changed by modifying the entrypoint).
- __margotpolimi/theo:1.0__ is a Docker image that runs the Theo component. It also requires access to the chosen MQTT broker (defaults to "127.0.0.1" but can be changed by modifying the entrypoint).
- __margotpolimi/brian:1.0__ is a Docker image that runs the Brian broker. It also requires access to the chosen MQTT broker (defaults to "127.0.0.1" but can be changed by modifying the entrypoint).

The Dockerfiles used to generate those files is present in the folders _core/containers_ and _theo_ and _brian_. 

## Installation settings

It is recommended to install the dependencies first with the following commands:

```
cd dependencies
. get_dependencies.sh
source set_env.sh
cd ..
source set_env.sh
```

It is also recommended to use the _compile\_margot.sh_ script for the compilation and installation of mARGOt.
It is recommended to add the _${OPTIONS\_CMAKE}_ parameter when running the cmake commands for compilation. For example, instead of running _cmake .._ execute _cmake ${OPTIONS\_CMAKE} .._. 