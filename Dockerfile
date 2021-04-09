# Build from official python image
FROM python:3.8

# Set working directory
WORKDIR /app

# Copy the source code to the image
COPY setup.py setup.py
COPY template template

# Install package
RUN pip install .
