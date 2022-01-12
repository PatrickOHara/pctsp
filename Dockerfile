FROM patrickohara/scip:latest

WORKDIR /app

# Download data for oplib
ENV OPLIB_ROOT /app/OPLib
RUN git clone https://github.com/bcamath-ds/OPLib.git ${OPLIB_ROOT}

# Download data for tspwplib
ENV TSPLIB_ROOT /app/tsplib95/archives/problems/tsp
RUN git clone https://github.com/rhgrant10/tsplib95.git /app/tsplib95

# Copy the source code to the image
COPY . /app/pctsp

# Install package
RUN pip3 install /app/pctsp