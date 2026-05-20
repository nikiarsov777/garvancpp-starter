#include "UserMongoService.h"

#include "vendors/Garvan/crow.h"
#include "vendors/Garvan/include/tools/Logger.h"

#include "../models/UserMongo.h"

using namespace AppModels;
using namespace AppServices;

MongoUserService::MongoUserService() {}

json AppServices::MongoUserService::index()
{
    cout << "MongoUserService" << endl;
    json result = UserMongo::where<UserMongo>("id", ">" , "0")->limit(1000)->get();

    cout << "result" << endl;
    return result;
}

json MongoUserService::create(const crow::request &req)
{
    auto data = crow::json::load(req.body);

    static Logger myLogger;
    crow::logger::setHandler(&myLogger);

    if (!data) {
        myLogger.log("Грешка: Невалиден JSON в тялото!", crow::LogLevel::Warning);
        return JsonValue{}.operator[]("error") = "Invalid JSON";
    }

    UserMongo *user = new UserMongo();
    // user->where("id", to_string(id));



    myLogger.log("=============", crow::LogLevel::Info);

    if (data.has("id")) {
        int id = data["id"].d(); // .s() превръща в string
        user->set("id", id);
        myLogger.log("Намерен id: " + id, crow::LogLevel::Info);
    }
    if (data.has("email")) {
        std::string emailValue = data["email"].s(); // .s() превръща в string
        myLogger.log("Намерен имейл: " + emailValue, crow::LogLevel::Info);
        user->set("email", emailValue);
    }    else {
        myLogger.log("Липсва 'email' в JSON обекта", crow::LogLevel::Warning);
    }
    if (data.has("name")) {
        std::string nameValue = data["name"].s(); // .s() превръща в string
        myLogger.log("Намерен name: " + nameValue, crow::LogLevel::Info);
        user->set("name", nameValue);
    } else {
        myLogger.log("Липсва 'name' в JSON обекта", crow::LogLevel::Warning);
    }

    if (data.has("password")) {
        std::string passwordValue = data["password"].s(); // .s() превръща в string
        myLogger.log("Намерен password: " + passwordValue, crow::LogLevel::Info);
        user->set("password", passwordValue);
    } else {
        myLogger.log("Липсва 'password' в JSON обекта", crow::LogLevel::Warning);
    }

    user->save();

    json res = json::Object();
    res["status"] = 200;
    return res;
}
