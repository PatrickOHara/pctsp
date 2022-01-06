FROM patrickohara/scip:latest

# Copy the source code to the image
COPY . /app/pctsp

# Install package
RUN --mount=source=.git,target=.git,type=bind \
    pip3 install --no-cache-dir /app/pctsp