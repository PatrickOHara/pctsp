# Build from official python image
FROM python:3.8

# Set working directory
WORKDIR /app

# Boost variables
ENV BOOST_MAJOR 1
ENV BOOST_MINOR 74
ENV BOOST_PATCH 0
ENV BOOST_VERSION "${BOOST_MAJOR}.${BOOST_MINOR}.${BOOST_PATCH}"
ENV BOOST_SRC_DIR "/app/boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}"
ENV BOOST_FILENAME "boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}.tar.bz2"
ENV BOOST_URL "https://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/${BOOST_FILENAME}/download"

# SCIP variables
ENV SCIP_VERSION 8.0.0
ENV SCIP_SRC_DIR /app/scipoptsuite-${SCIP_VERSION}
ENV SCIP_FILENAME scipoptsuite-${SCIP_VERSION}.tgz
ENV SCIP_URL https\://www.scipopt.org/download/release

# TBB variables
ENV TBB_SRC_DIR /app/oneTBB
ENV TBB_URL https://github.com/oneapi-src/oneTBB.git

# yaml-cpp variables
ENV YAML_CPP_SRC_DIR /app/yaml-cpp
ENV YAML_CPP_URL https://github.com/jbeder/yaml-cpp.git

# install locations
ENV LOCAL_INSTALL_PREFIX /usr/local/
ENV TBB_ROOT ${LOCAL_INSTALL_PREFIX}
ENV SCIP_ROOT ${LOCAL_INSTALL_PREFIX}
ENV YAML_CPP_ROOT ${LOCAL_INSTALL_PREFIX}

# export location of local install for linker
ENV LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${LOCAL_INSTALL_PREFIX}/lib"

# download and extract boost, scip, TBB and yaml-cpp
ADD ${BOOST_URL} ${BOOST_FILENAME}
ADD ${SCIP_URL}/${SCIP_FILENAME} ${SCIP_FILENAME}
ADD https://api.github.com/repos/oneapi-src/oneTBB/compare/master...HEAD /dev/null
ADD https://api.github.com/repos/jbeder/yaml-cpp/compare/master...HEAD /dev/null
RUN tar --bzip2 -xf ${BOOST_FILENAME}
RUN tar zxvf ${SCIP_FILENAME}
RUN git clone ${TBB_URL} ${TBB_SRC_DIR}
RUN git clone ${YAML_CPP_URL} ${YAML_CPP_SRC_DIR}

# install build dependencies with pip
RUN pip3 install cmake ninja scikit-build

# build and install Boost
WORKDIR ${BOOST_SRC_DIR}
RUN ./bootstrap.sh
RUN ./b2 install
RUN rm -r ${BOOST_SRC_DIR}

# build and install TBB
RUN mkdir ${TBB_SRC_DIR}/build
WORKDIR ${TBB_SRC_DIR}/build
RUN cmake -GNinja -DCMAKE_INSTALL_PREFIX=${TBB_ROOT} -DTBB_TEST=OFF ..
RUN cmake --build .
RUN cmake --install .
RUN rm -r ${TBB_SRC_DIR}

# build and install SCIP
RUN mkdir ${SCIP_SRC_DIR}/build
WORKDIR ${SCIP_SRC_DIR}/build
RUN cmake -GNinja -DCMAKE_INSTALL_PREFIX=${SCIP_ROOT} -DIPOPT=false ..
RUN cmake --build .
RUN cmake --install .
RUN rm -r ${SCIP_SRC_DIR}

# download, build and install yaml-cpp
RUN mkdir ${YAML_CPP_SRC_DIR}/build
WORKDIR ${YAML_CPP_SRC_DIR}/build
RUN cmake -GNinja -DYAML_BUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DYAML_CPP_BUILD_TESTS=0 -DCMAKE_INSTALL_PREFIX=${YAML_CPP_ROOT} ..
RUN cmake --build .
RUN cmake --install .
RUN rm -r ${YAML_CPP_SRC_DIR}

# set so that pyscipopt can find the scip install
ENV SCIPOPTDIR=${SCIP_ROOT}
RUN pip install pyscipopt
