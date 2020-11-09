# Use the latest stable Ubuntu version (currently 18.04)
FROM ubuntu:18.04

WORKDIR /home


ENV TBB_RELEASE 2018_U2
ENV TBB_VERSION 2018_20171205
ENV TBB_DOWNLOAD_URL https://github.com/01org/tbb/releases/download/${TBB_RELEASE}/tbb${TBB_VERSION}oss_lin.tgz
ENV TBB_INSTALL_DIR /opt
ENV CLANG_VERSION 9.0.0

# Make sure the image is updated, install some prerequisites,  Download the latest version of Clang (official binary) for Ubuntu
# Extract the archive and add Clang to the PATH
RUN apt-get update -y && apt-get install -y \
  xz-utils \
  build-essential \
  curl \
  wget \
  && rm -rf /var/lib/apt/lists/* \
  && curl -SL http://releases.llvm.org/${CLANG_VERSION}/clang+llvm-${CLANG_VERSION}-x86_64-linux-gnu-ubuntu-14.04.tar.xz \
  | tar -xJC . && mv clang+llvm-${CLANG_VERSION}-x86_64-linux-gnu-ubuntu-14.04 clang_${CLANG_VERSION} && \
  echo 'export PATH=/clang_${CLANG_VERSION}/bin:$PATH' >> ~/.bashrc && \
  echo 'export LD_LIBRARY_PATH=/clang_${CLANG_VERSION}/lib:LD_LIBRARY_PATH' >> ~/.bashrc

# Download and install TBB
RUN wget ${TBB_DOWNLOAD_URL} && \
	tar -C ${TBB_INSTALL_DIR} -xf tbb${TBB_VERSION}oss_lin.tgz && \
	rm tbb${TBB_VERSION}oss_lin.tgz && \
	sed -i "s%SUBSTITUTE_INSTALL_DIR_HERE%${TBB_INSTALL_DIR}/tbb${TBB_VERSION}oss%" ${TBB_INSTALL_DIR}/tbb${TBB_VERSION}oss/bin/tbbvars.* && \
	echo "source ${TBB_INSTALL_DIR}/tbb${TBB_VERSION}oss/bin/tbbvars.sh intel64" >> ~/.bashrc

# fetch recent cmake & install
ENV CMAKE_VER_MAJOR=3
ENV CMAKE_VER_MINOR=16
ENV CMAKE_VER_PATCH=4
ENV CMAKE_VER="${CMAKE_VER_MAJOR}.${CMAKE_VER_MINOR}"
ENV CMAKE_VERSION="${CMAKE_VER}.${CMAKE_VER_PATCH}"
RUN mkdir -p /tmp/build && cd /tmp/build && \
  curl -sSL https://cmake.org/files/v${CMAKE_VER}/cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz >cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz && \
  tar -v -zxf cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz && \
  rm -f cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz && \
  cd cmake-${CMAKE_VERSION}-Linux-x86_64 && \
  cp -rp bin/* /usr/local/bin/ && \
  cp -rp share/* /usr/local/share/ && \
  cd / && rm -rf /tmp/build

RUN apt-get update && apt-get install -y git libpthread-stubs0-dev automake

# fetch and install boost
RUN wget https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.gz && tar xf boost_1_74_0.tar.gz && cd boost_1_74_0 && ./bootstrap.sh --prefix=/usr/local && ./b2 install

# require papi libs from http://icl.utk.edu/papi/software/
# -> goto src, run ./configure, make install
RUN git clone https://bitbucket.org/icl/papi.git && cd papi/src && ./configure && make install

# clone grizzly repo
RUN cd /home && git clone https://github.com/LeonhardFS/grizzly-prototype.git

# Start from a Bash prompt
CMD [ "/bin/bash" ]
