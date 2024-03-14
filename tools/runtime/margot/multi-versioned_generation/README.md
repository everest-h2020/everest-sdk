# Multi-versioned kernel generation

This flow structures the generation of a C++ library featuring mARGOt able to choose the best version of the kernel among the ones provided. The flow consists of three scripts that have to be run in order.

The first script pre-generates the file needed and the packer/unpacker functions given the name of the version and the problem.json. It pre-generates also the cmake needed for the compilation. In case of FPGA versions, it generates also part of the executor to handle the interaction with the accelerator.

```
python3 uva.py <problem.json> <output dir>
```

The second script collects all the version.json specified in the different versions into a single one (it checks all the subdirectories of the current path).

```
python3 collector.py <default_version>
```

The third generator script takes the collected version.json file, the problem.json file and it builds the margot configuration file, margot executables and cmake files (it will take them from the current directory). If AGORA support is needed, it requires also a json with the agora configuration.

```
python3 generator.py [-- agora <AGORA_json>] <problem_json>
```