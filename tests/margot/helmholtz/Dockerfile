FROM margotpolimi/margot:1.0

COPY . /usr/src/app
WORKDIR /usr/src/app

RUN cp config/AGORA.txt CMakeLists.txt \
  mkdir build \
  && cd build \
  && cmake .. \
  && make -j

WORKDIR /usr/src/app/build
CMD ["./helmholtz"]
