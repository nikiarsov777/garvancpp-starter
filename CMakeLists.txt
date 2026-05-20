cmake_minimum_required(VERSION 3.20)
project(Garvan CXX)

# Конфигурация на компилатора
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Търсене на зависимости
find_package(PkgConfig REQUIRED)

# MongoDB C++ driver (libmongocxx + libbsoncxx)
pkg_check_modules(MONGOCXX REQUIRED IMPORTED_TARGET libmongocxx libbsoncxx)

# MonetDB MAPI - pkg-config is broken on this system, so we set it manually
set(MONETDB_INCLUDE_DIRS /usr/include/monetdb)
set(MONETDB_LIBRARIES mapi)

# Дефиниране на изходната директория
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 1. Събиране на всички .cpp файлове
file(GLOB_RECURSE SOURCES
    "main.cpp"
    "routes/*.cpp"
    "app/controllers/*.cpp"
    "app/controllers/api/*.cpp"
    "app/models/*.cpp"
    "app/services/*.cpp"
)

# 2. Събиране на всички .h файлове (за да се виждат в Qt Creator)
file(GLOB_RECURSE HEADERS
    "*.h"
    "*.hpp"
    "routes/*.h"
    "app/controllers/*.h"
    "app/models/*.h"
    "app/services/*.h"
    "vendors/Dbmate/*.h"
    "vendors/Garvan/*.h"
    "vendors/Garvan/controller*.h"
    "vendors/Garvan/crow*.h"
    "vendors/Garvan/db*.h"
    "vendors/Garvan/model*.h"
    "vendors/Garvan/orm*.h"
    "vendors/Garvan/orm/connection/*.h"
    "vendors/Garvan/orm/grammar/*.h"
    "vendors/Garvan/service*.h"
    "vendors/Garvan/tools*.h"
)

# 3. Добавяне на SOURCES и HEADERS в изпълнимия файл
add_executable(app_bin ${SOURCES} ${HEADERS})

# Инклуд директории
target_include_directories(app_bin PRIVATE
    .
    app/models
    app/controllers

    vendors/Garvan
    vendors/Garvan/include
    vendors/Garvan/db
    vendors/Garvan/model
    vendors/Garvan/orm
    vendors/Garvan/orm/connection
    vendors/Garvan/orm/grammar
    vendors/Garvan/service
    vendors/Garvan/tools

    ${MONGOCXX_INCLUDE_DIRS}
    ${MONETDB_INCLUDE_DIRS}
)

link_directories(/usr/lib/x86_64-linux-gnu)

# Линкиране на библиотеки
target_link_libraries(app_bin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vendors/Garvan/libgarvan.a")
target_link_libraries(app_bin PRIVATE pqxx pq mysqlcppconn mapi sqlite3)
target_link_libraries(app_bin PRIVATE ${MONGOCXX_LIBRARIES} ${MONETDB_LIBRARIES})

# Име на изходния файл
set_target_properties(app_bin PROPERTIES OUTPUT_NAME app.bin)
