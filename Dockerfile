FROM patrickohara/scip:latest

ARG SETUPTOOLS_SCM_PRETEND_VERSION_FOR_PCTSP=0.2.7

WORKDIR /app

# Download data for oplib
ADD https://api.github.com/repos/bcamath-ds/OPLib/compare/master...HEAD /dev/null
ENV OPLIB_ROOT /app/OPLib
RUN git clone https://github.com/bcamath-ds/OPLib.git ${OPLIB_ROOT}

# Download data for tspwplib
ADD https://api.github.com/repos/rhgrant10/tsplib95/compare/master...HEAD /dev/null
ENV TSPLIB_ROOT /app/tsplib95/archives/problems/tsp
RUN git clone https://github.com/rhgrant10/tsplib95.git /app/tsplib95

# Copy the source code to the image
COPY . /app/pctsp

# Install package
RUN pip3 install pybind11
RUN --mount=source=.git,target=.git,type=bind \
    pip3 install --no-cache-dir /app/pctsp 