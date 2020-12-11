# iota-simplewallet
A Simple and Secure IOTA wallet written in C

# Build on linux

## Install required depedencies

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

### Build the project

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


