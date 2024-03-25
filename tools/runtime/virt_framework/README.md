# Virtualization framework repository
## Structure
- Folder **esfm** contains the Everest SR-IOV FPGA Manager, which is the program the user can use to control the whole virtualization framework
- Folder **qdma-drivers** contains the modified qdma drivers and their wrapper API to be installed and used by the application developer
- Folder **qemu**: contains the patch to be applied to qemu (full project available on gitlab, at this [address](https://github.com/qemu/qemu))

## Requirements
- Git 2+
- Virsh 8.0.0+
- QEMU 7.1.0+
- GCC 8.5.5
- Vivado 2022.1
- Red Hat Enterprise Linux 8.7 (Ootpa)
- Linux 4.18.0-348.7.1.el8_5.x86_64

More information about how to create Virtual Machines with *Virsh* can be found [here](https://medium.com/@art.vasilyev/use-ubuntu-cloud-image-with-kvm-1f28c19f82f8).

## Installation settings
D5.6 and the README of each component describe the in depth installation procedure and requirements (permissions, dependencies).
Here below we briefly detail the installation settings for each component of the Everest virtualization framework.

### ESFM
ESFM is a bash script, hence it does not require any particular installation process.
Given that requirements are fulfilled, the esfm can just be executed.

### QDMA-DRIVERS
The installation process of these drivers is simple and similar to any other driver installation. As mentioned, more details on the related [README](https://github.com/virtualopensystems/qdma_drivers://github.com/virtualopensystems/qdma_drivers#).

## Funding
> This research was supported by the Horizon 2020 EU Research & Innovation programme under GA No 957269 (EVEREST project).
