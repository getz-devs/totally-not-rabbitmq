FROM ubuntu:latest AS build

# Install necessary packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    unzip \
    wget

WORKDIR /usr/src/rabbit

# Download source code
COPY . /usr/src/rabbit

# Install Boost
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.bz2 --no-check-certificate && \
    tar -xf boost_1_84_0.tar.bz2 && \
    mv boost_1_84_0 boost && \
    cd boost && \
    ./bootstrap.sh && \
    ./b2 install --prefix=/usr/local

# Build the project
RUN mkdir build && \
    cd build && \
    cmake .. && \
    make -j$(nproc) rabbitServerL3cmd

# Runtime stage
FROM ubuntu:latest

# Install necessary packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    libboost-all-dev

# Copy the built executable from the build stage
COPY --from=build /usr/src/rabbit/build/rabbitServerL3cmd /usr/local/bin/

# Expose the server port
EXPOSE 3030

# Run the server
CMD ["rabbitServerL3cmd", "-p", "3030"]