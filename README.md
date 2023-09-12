# Introduction

Yamal is an open source library for transactional low-latency IPC and data capture. It is used to build systems where data is communicated and captured between different processes very quickly, with an emphasis on ensuring the consistency and reliability of that data. This is especially important in environments where fast, reliable data transmission and storage are essential, such as financial trading platforms or real-time analytics systems.

# How to build

## Requirements

* Cmake
* Git
* C/C++ compiler
* Python >=3.6

```bash
apt-get install -y cmake git build-essential python3
```

## Clone and build

```bash
git clone --recurse-submodules https://github.com/featuremine/yamal.git && \
mkdir yamal/build && \
cd yamal/build && \
cmake .. \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_TESTING=ON \
  -DBUILD_TOOLS=ON \
  -DBUILD_WHEEL=ON \
  -DTEST_EXTENSIONS=ON \
  -DBUILD_DOCUMENTATION=OFF \
  -DCMAKE_BUILD_TYPE=Release && \
make -j 3
```

# Testing

## Run using ctest

```bash
ctest --extra-verbose
```

# Usage instructions

- [Yamal Documentation](docs/README.md)
