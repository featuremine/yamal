# Introduction

Yamal is a message-oriented transactional IPC protocol. Yamal can be used to seamlessly interchange between local and distributed deployment.

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
  -DBUILD_DOCUMENTATION=ON \
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
