#!/bin/bash
# Build script for Garvan C++ app -> bin/app.bin
# Mirrors CMakeLists.txt so you can build without cmake.

set -e

# Clean & recreate output dir
rm -rf bin
mkdir -p bin

g++ -std=c++20 -O2 -pthread \
    main.cpp \
    routes/*.cpp \
    app/controllers/*.cpp \
    app/controllers/api/*.cpp \
    app/models/*.cpp \
    app/services/*.cpp \
    -I. \
    -Iapp/models \
    -Iapp/controllers \
    -Ivendors/Garvan \
    -Ivendors/Garvan/include \
    -Ivendors/Garvan/db \
    -Ivendors/Garvan/model \
    -Ivendors/Garvan/orm \
    -Ivendors/Garvan/orm/connection \
    -Ivendors/Garvan/orm/grammar \
    -Ivendors/Garvan/service \
    -Ivendors/Garvan/tools \
    -I/usr/include/monetdb \
    $(pkg-config --cflags libmongocxx libbsoncxx) \
    vendors/Garvan/libgarvan.a \
    -L/usr/lib/x86_64-linux-gnu \
    -lpqxx -lpq \
    -lmysqlcppconn \
    -lmapi \
    -lsqlite3 \
    $(pkg-config --libs libmongocxx libbsoncxx) \
    -o bin/app.bin

echo "Build OK -> bin/app.bin"
