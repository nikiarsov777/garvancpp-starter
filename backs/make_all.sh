#!/bin/bash
# g++ main.cpp -o app.bin -I controllers/ -I db/ -I models/ -lpthread
# g++ main.cpp -o app.bin app/controllers/*.cpp  app/db/*.cpp app/models/*.cpp -lpthread
# g++ main.cpp -o app.bin -I app/controllers/  -I app/db/ -I app/models/  app/controllers/*.cpp  app/db/*.cpp app/models/*.cpp -lpthread

if [ ! -d "bin" ]; then
  mkdir bin
fi

#g++ -std=c++20 \
#main.cpp -o bin/app.bin  routes/*.cpp app/controllers/*.cpp app/controllers/api/*.cpp \
#app/models/*.cpp app/services/*.cpp vendors/Garvan/orm/*.cpp vendors/Garvan/model/*.cpp \
#vendors/Garvan/tools/*.cpp vendors/Garvan/controller/*.cpp vendors/Garvan/db/*.cpp vendors/Garvan/service/*.cpp \
#-lpthread -lpqxx -lmysqlcppconn -I/usr/include/mongoc-2.2.2 -I/usr/include/bson-2.2.2 \
#-I/usr/include/bsoncxx/v_noabi -I/usr/include/mongocxx/v_noabi \
#-lmongoc2 -lbson2 -lmongocxx -lbsoncxx

g++ -std=c++20 \
main.cpp routes/*.cpp app/controllers/*.cpp app/controllers/api/*.cpp \
app/models/*.cpp app/services/*.cpp vendors/Garvan/orm/*.cpp vendors/Garvan/model/*.cpp \
vendors/Garvan/tools/*.cpp vendors/Garvan/controller/*.cpp vendors/Garvan/db/*.cpp vendors/Garvan/service/*.cpp \
-o bin/app.bin \
-I/usr/include/mongoc-2.2.2 -I/usr/include/bson-2.2.2 \
$(pkg-config --cflags --libs libmongocxx libbsoncxx) \
-lmongoc2 -lbson2 -lpthread -lpqxx -lmysqlcppconn

