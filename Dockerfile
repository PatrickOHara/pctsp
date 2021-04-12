from patrickohara/scip:latest

# Copy the source code to the image
COPY . /app/pctsp

# Install package
RUN pip3 install /app/pctsp
