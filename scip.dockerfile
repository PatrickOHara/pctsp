# Build from official python image
FROM patrickohara/tspwplib:latest

# install cmake and boost
RUN apt -y update --fix-missing \
    && apt -y upgrade \
    && apt install -y cmake libboost-all-dev

# Set working directory
WORKDIR /app

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

# download and extract soplex and scip
ADD ${SOPLEX_URL}/${SOPLEX_FILENAME} ${SOPLEX_FILENAME}
ADD ${SCIP_URL}/${SCIP_FILENAME} ${SCIP_FILENAME}
RUN tar zxvf ${SOPLEX_FILENAME}
RUN tar zxvf ${SCIP_FILENAME}

# build SOPLEX
RUN mkdir ${SOPLEX_DIR}/build
WORKDIR ${SOPLEX_DIR}/build
RUN cmake ..
RUN cmake --build .
RUN make install

# build SCIP
RUN mkdir ${SCIP_DIR}/build
WORKDIR ${SCIP_DIR}/build
RUN cmake .. -DSOPLEX_DIR=${SOPLEX_DIR}/build
RUN cmake --build .
RUN make install

# install location of SCIP
ENV SCIP_ROOT /usr/local