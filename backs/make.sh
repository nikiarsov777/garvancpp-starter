#!/bin/bash

rm -rf bin
if [ ! -d "bin" ]; then
  mkdir bin
fi

g++ -std=c++20 \
main.cpp  \
routes/*.cpp app/controllers/*.cpp app/controllers/api/*.cpp \
app/models/*.cpp app/services/*.cpp \
-I. \
-Iapp/models \
-Iapp/controllers \
-Ivendors/Garvan \
-Ivendors/Garvan/db \
-Ivendors/Garvan/model \
-Ivendors/Garvan/orm \
-Ivendors/Garvan/service \
-Ivendors/Garvan/tools \
vendors/Garvan/libgarvan.a \
-lpqxx -lpq -lmysqlcppconn \
$(pkg-config --cflags --libs libmongocxx libbsoncxx) \
-o bin/app.bin
