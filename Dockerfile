FROM ubuntu:14.04

RUN mkdir /tissuestack
WORKDIR /tissuestack

RUN apt-get update
RUN apt-get -y install make
RUN apt-get -y install g++
RUN apt-get -y install libgraphicsmagick1-dev
RUN apt-get -y install uuid-dev
RUN apt-get -y install libzip-dev
RUN apt-get -y install libnifti-dev
RUN apt-get -y install libminc-dev
RUN apt-get -y install libpqxx-dev
RUN apt-get -y install libdcmtk-dev

RUN mkdir -p /tissuestack/src
COPY ./src/c++ /tissuestack/src/c++

WORKDIR /tissuestack/src/c++

RUN make && make compile && make install

WORKDIR /usr/local/tissuestack/2.3/bin

CMD /usr/local/tissuestack/2.3/bin/TissueStackServer
