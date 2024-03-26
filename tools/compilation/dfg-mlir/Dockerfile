FROM ubuntu:jammy

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get full-upgrade -y
RUN apt-get install -y llvm clang ninja-build cmake wget less bat vim sed python3 python3-pip # curl htop

ADD dfg-mlir /opt/dfg-mlir
WORKDIR /opt/dfg-mlir
RUN ./build_mlir.sh

ENV PATH="/opt/dfg-mlir/libs/circt_build/bin:/opt/dfg-mlir/libs/llvm_build/bin:${PATH}"

WORKDIR /opt/dfg-mlir
RUN cmake -S . -B build \
    -G Ninja \
    -DLLVM_DIR=/opt/dfg-mlir/libs/llvm_build/lib/cmake/llvm \
    -DMLIR_DIR=/opt/dfg-mlir/libs/llvm_build/lib/cmake/mlir \
    -DCIRCT_DIR=/opt/dfg-mlir/libs/circt_build/lib/cmake/circt \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DLLVM_USE_LINKER=lld
RUN cmake --build build
RUN cmake --build build --target install

WORKDIR /
#RUN rm -rf /opt/condrust

ENTRYPOINT ["bash"]
