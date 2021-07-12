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
ENV SCIP_VERSION 7.0.2
ENV SCIP_DIR /app/scip-${SCIP_VERSION}
ENV SCIP_FILENAME scip-${SCIP_VERSION}.tgz
ENV SCIP_URL https\://www.scipopt.org/download/release

# SOPLEX variables
ENV SOPLEX_VERSION 5.0.2
ENV SOPLEX_DIR /app/soplex-${SOPLEX_VERSION}
ENV SOPLEX_FILENAME soplex-${SOPLEX_VERSION}.tgz
ENV SOPLEX_URL https://soplex.zib.de/download/release

# download and extract boost, soplex and scip
ADD ${BOOST_URL} ${BOOST_FILENAME}
ADD ${SOPLEX_URL}/${SOPLEX_FILENAME} ${SOPLEX_FILENAME}
ADD ${SCIP_URL}/${SCIP_FILENAME} ${SCIP_FILENAME}
RUN tar --bzip2 -xf ${BOOST_FILENAME}
RUN tar zxvf ${SOPLEX_FILENAME}
RUN tar zxvf ${SCIP_FILENAME}

# install build dependencies with pip
RUN pip3 install cmake ninja

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

# install location of SCIP
ENV SCIP_ROOT /usr/local
