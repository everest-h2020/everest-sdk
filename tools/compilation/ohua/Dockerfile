FROM ubuntu:jammy

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get full-upgrade -y
RUN apt-get install -y llvm clang less bat vim sed python3 python3-pip haskell-stack # curl htop

ADD condrust /opt/condrust
WORKDIR /opt/condrust
RUN stack upgrade --binary-only
RUN stack install --no-nix --local-bin-path /usr/bin
WORKDIR /
RUN rm -rf /opt/condrust

ENTRYPOINT ["bash"]
