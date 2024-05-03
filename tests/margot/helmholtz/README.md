# mARGOt integration

The integration with mARGOt introduced some changes in the sources. The main difference can be found in the operator file, which has been replaced by the operator+margot one.
The three source files of the alternative versions didn't need major changes.

## Installation

* Adjust the mARGOt configuration file to suit the code. We provide two working configuration files depending on the use of agora;
* Edit the line "margot_heel_generate_interface(\<Configurations\>)" in the CMakeLists.txt file by including the configuration files. If using mARGOt+agora, only one file is needed, while if using only margot the knowledge file must be specified too (an example knowledge file can be seen in the ops.json file);
* Run the following commands to build without Docker;

```
cp config/AGORA.txt CMakeLists.txt
mkdir build
cd build
cmake ${OPTIONS_CMAKE} ..
make
./helmholtz
```

* Alternatively, it is possible to build and run a Docker container with the application using the provided Dockerfile. The following commands create the Docker image and then launch a container.

```
docker build -t helmholtz_dock .
docker run -it --rm --name helmholtz --network host helmholtz_dock
```

#### Additional info

More info can be found in the documentations of mARGOt.
