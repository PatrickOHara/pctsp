# Build from official python image
FROM patrickohara/tspwplib:latest

# Set working directory
WORKDIR /app

# Boost variables
ENV BOOST_MAJOR 1
ENV BOOST_MINOR 74
ENV BOOST_PATCH 0
ENV BOOST_VERSION "${BOOST_MAJOR}.${BOOST_MINOR}.${BOOST_PATCH}"
ENV BOOST_DIR "/app/boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}"
ENV BOOST_FILENAME "boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}.tar.bz2"
ENV BOOST_URL "https://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/${BOOST_FILENAME}/download"

# SCIP variables
ENV SCIP_VERSION 7.0.3
ENV SCIP_DIR /app/scip-${SCIP_VERSION}
ENV SCIP_FILENAME scip-${SCIP_VERSION}.tgz
ENV SCIP_URL https\://www.scipopt.org/download/release

# SOPLEX variables
ENV SOPLEX_VERSION 5.0.2
ENV SOPLEX_DIR /app/soplex-${SOPLEX_VERSION}
ENV SOPLEX_FILENAME soplex-${SOPLEX_VERSION}.tgz
ENV SOPLEX_URL https://soplex.zib.de/download/release

# yaml-cpp variables
ENV YAML_CPP_DIR /app/yaml-cpp
ENV YAML_CPP_URL https://github.com/jbeder/yaml-cpp.git

# download and extract boost, soplex, scip and yaml-cpp
ADD ${BOOST_URL} ${BOOST_FILENAME}
ADD ${SOPLEX_URL}/${SOPLEX_FILENAME} ${SOPLEX_FILENAME}
ADD ${SCIP_URL}/${SCIP_FILENAME} ${SCIP_FILENAME}
ADD https://api.github.com/repos/jbeder/yaml-cpp/compare/master...HEAD /dev/null
RUN tar --bzip2 -xf ${BOOST_FILENAME}
RUN tar zxvf ${SOPLEX_FILENAME}
RUN tar zxvf ${SCIP_FILENAME}
RUN git clone ${YAML_CPP_URL} ${YAML_CPP_DIR}

# install build dependencies with pip
RUN pip3 install cmake ninja scikit-build

# build Boost
WORKDIR ${BOOST_DIR}
RUN ./bootstrap.sh
RUN ./b2 install

# build SOPLEX
RUN mkdir ${SOPLEX_DIR}/build
WORKDIR ${SOPLEX_DIR}/build
RUN cmake -GNinja ..
RUN cmake --build .
RUN cmake --install .

# build SCIP
RUN mkdir ${SCIP_DIR}/build
WORKDIR ${SCIP_DIR}/build
RUN cmake -GNinja .. -DSOPLEX_DIR=${SOPLEX_DIR}/build
RUN cmake --build .
RUN cmake --install .

# download, build and install yaml-cpp
RUN mkdir ${YAML_CPP_DIR}/build
WORKDIR ${YAML_CPP_DIR}/build
RUN cmake -GNinja -DYAML_BUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DYAML_CPP_BUILD_TESTS=0 ..
RUN cmake --build .
RUN cmake --install .

# install location of SCIP
ENV SCIP_ROOT /usr/local

# set so that pyscipopt can find the scip install
ENV SCIPOPTDIR=${SCIP_ROOT}
RUN pip install pyscipopt

# export location of local install for linker
ENV LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib"