# -*- coding: utf-8 -*-
FROM ubuntu:18.04 AS build-env
MAINTAINER Takahiro INOUE <takahiro.inoue@aist.go.jp>

ENV JOBS 4

WORKDIR /tmp

####
## requirement for docker build
####

RUN apt-get update        && \
    apt-get -y install       \
      build-essential        \
      cmake                  \
      git                    \
      qt5-default

RUN git clone https://github.com/eigenteam/eigen-git-mirror.git && \
    mkdir build-eigen                                           && \
    cd    build-eigen                                           && \
    cmake ../eigen-git-mirror                                   && \
    make -j ${JOBS} install                                     && \
    cd ..                                                       && \
    ldconfig || true

####
## application deploy
####

ADD . .

RUN mkdir build     && \
    cd build        && \
    cmake ..        && \
    make -j ${JOBS}

FROM ubuntu:18.04
MAINTAINER Takahiro INOUE <takahiro.inoue@aist.go.jp>

COPY --from=build-env /tmp/build/unblending-cli/unblending-cli /usr/local/bin/unblending-cli

RUN apt-get update      && \
    apt-get -y install     \
      qt5-default

ENTRYPOINT [ "unblending-cli" ]
