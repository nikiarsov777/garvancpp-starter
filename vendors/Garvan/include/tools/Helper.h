#ifndef HELPER_H
#define HELPER_H

#pragma once

#include <pqxx/pqxx>
#include "JsonValue.h"

using json = JsonValue;
using namespace std;

namespace Garvan
{
    class Helper
    {
    public:
        // Helper();
        // ~Helper();
        static json printToJason(pqxx::result result, vector<string> columns);
        static string view(string html, json data);
        static std::vector<std::string> split(std::string s, std::string delimiter);
        static string getenv(string param);
        
    private:
        
    };
}

#endif
