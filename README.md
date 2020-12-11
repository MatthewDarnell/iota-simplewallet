# iota-simplewallet
A Simple and Secure IOTA wallet written in C

# Build on linux

Requirements:
- Ubuntu 18.04
- Qt, min version 5.9
- CMake, min version 15.6

## Installing required dependencies

```
apt-get install build-essential git libsodium-dev
```

Iota-simplewallet depends on Qt, you can use default version which is shipped with Ubuntu 18.04 or you use another one version

In case of using default qt:
```
apt-get install qt5-default
```

CMake 15.6 is required, the easiset way is to install it:
```
wget https://github.com/Kitware/CMake/releases/download/v3.18.5/cmake-3.18.5-Linux-x86_64.sh
chmod +x cmake-3.18.5-Linux-x86_64.sh
sudo ./cmake-3.18.5-Linux-x86_64.sh --skip-license --prefix=/usr
```

## Building the project

To build the project execute:

```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=output ../
cmake --build .
```

This will put `iota-qt` into `build/output/appdir/usr/bin`

If you are using custom qt, you will need to pass extra parameter to cmake.

```
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=output -DCMAKE_PREFIX_PATH=path_to_qt_kit ../
```

# Build on Windows

Requirements:
- MinGW 9
- Qt, min version 5.9
- CMake, min version 15.6

## Installing required dependencies

This project can be built only with MinGW. Installing MinGW is a complicated process, you can use one that is deployed with Qt native installation or use something like msys2.

## Building the project

To build the project form mingw shell execute:

```
mkdir build && cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=output ../
cmake --build .
```

This will put `iota-qt` into `build/output`

If you are using custom Qt installation without msys2 but with MinGW use Qt Creator IDE to build the project, tweak `CMAKE_INSTALL_PREFIX` to place executable in desired location.