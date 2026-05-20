#include "UserMonetService.h"

#include "vendors/Garvan/crow.h"
#include "vendors/Garvan/include/tools/Logger.h"

#include "../models/UserMonet.h"

using namespace AppModels;
using namespace AppServices;

UserMonetService::UserMonetService() {}

json UserMonetService::index()
{
    cout << "MonetUserService" << endl;
    json result = UserMonet::where<UserMonet>("id", ">" , "0")->limit(1000)->get();

    cout << "result" << endl;
    return result;
}

json UserMonetService::create(const crow::request &req)
{
    auto data = crow::json::load(req.body);

    static Logger myLogger;
    crow::logger::setHandler(&myLogger);

    if (!data) {
        myLogger.log("Грешка: Невалиден JSON в тялото!", crow::LogLevel::Warning);
        return JsonValue{}.operator[]("error") = "Invalid JSON";
    }

    UserMonet *user = new UserMonet();
    // user->where("id", to_string(id));



    myLogger.log("=============", crow::LogLevel::Info);

    if (data.has("id")) {
        // std::string id = "";
        std::string id = data["id"].s(); // .s() превръща в string

        myLogger.log("Намерен id: " + id, crow::LogLevel::Info);
        // id = user->where("email", "niki_arsov@abv.bg")->getId();
        // user->setId(id);

        myLogger.log("Намерен id: " + user->getId(), crow::LogLevel::Info);

        // Пълним attributes, за да може save() да работи
        // user->setId(id);

        // Ако това е CREATE, НЕ слагай setId(1), освен ако не искаш
        // винаги да презаписваш потребител №1 (UPDATE)
    }
    if (data.has("email")) {
        std::string emailValue = data["email"].s(); // .s() превръща в string

        myLogger.log("Намерен имейл: " + emailValue, crow::LogLevel::Info);

        // Пълним attributes, за да може save() да работи
        user->set("email", emailValue);

        // Ако това е CREATE, НЕ слагай setId(1), освен ако не искаш
        // винаги да презаписваш потребител №1 (UPDATE)
    }    else {
        myLogger.log("Липсва 'email' в JSON обекта", crow::LogLevel::Warning);
    }
    if (data.has("name")) {
        std::string nameValue = data["name"].s(); // .s() превръща в string

        myLogger.log("Намерен name: " + nameValue, crow::LogLevel::Info);

        // Пълним attributes, за да може save() да работи
        user->set("name", nameValue);

        // Ако това е CREATE, НЕ слагай setId(1), освен ако не искаш
        // винаги да презаписваш потребител №1 (UPDATE)
    } else {
        myLogger.log("Липсва 'name' в JSON обекта", crow::LogLevel::Warning);
    }

    if (data.has("password")) {
        std::string passwordValue = data["password"].s(); // .s() превръща в string

        myLogger.log("Намерен password: " + passwordValue, crow::LogLevel::Info);

        // Пълним attributes, за да може save() да работи
        user->set("password", passwordValue);

        // Ако това е CREATE, НЕ слагай setId(1), освен ако не искаш
        // винаги да презаписваш потребител №1 (UPDATE)
    } else {
        myLogger.log("Липсва 'password' в JSON обекта", crow::LogLevel::Warning);
    }


    user->save();
    myLogger.log("====2====", crow::LogLevel::Info);


    json res = json::Object();
    res["status"] = 200;
    return res;
}
