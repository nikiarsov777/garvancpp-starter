#include "UserPsqlService.h"

#include "vendors/Garvan/crow.h"
#include "vendors/Garvan/include/tools/Logger.h"

#include "../models/User.h"
#include "../models/Role.h"
#include "../models/Team.h"
#include "../models/Post.h"

using namespace AppModels;
using namespace AppServices;

UserPsqlService::UserPsqlService()
{
}

UserPsqlService::~UserPsqlService()
{
}

json UserPsqlService::index()
{
    // UserModel *userModel = new UserModel();
    // pqxx::result r = userModel->where("id" , "<", "2")->belongsTo(Role())->all();

    // Team *team = new Team();
    // json result = team->where("id" , "5")->with(User())->get();

    User *user = new User();
    // json result = User::where<User>("id", ">" , "0")->hasOne(Team())->get();
    json result = User::where<User>("id", ">" , "0")->limit(1000)->get();
    // user->email = "test@test.bg";
    // user->save();
    // json result = User::hasOne(Team())->get();
    // json result = user->get();

    // vector<string> columns = {"id", "name"};
    // json jsonData = printToJason(r, columns);
    // json result = json::object();
    // result["result"] = "OK";
    return result;
}

json AppServices::UserPsqlService::get(int id)
{
    User *user = new User();
    // json result = user->where("id" , "2")->hasOne(Team())->get();
    // json result = user->find(id);
    try
    {
        // json result = user->findOrFail(id);
        // json result = user->first();
        json result = user->firstOrFail();
        return result;
    }
    catch (std::exception &e)
    {
        json err = json::Object();
        err["error"] = e.what();
        return err;
    }
}

json AppServices::UserPsqlService::posts(int id)
{
    User *user = new User();

    try
    {

        json result = user->where("id", to_string(id))->hasMany(Post())->get();
        // json result = dynamic_cast<User *>(user->where("id", to_string(id)))->posts()->get();
        return result;
    }
    catch (std::exception &e)
    {
        json err = json::Object();
        err["error"] = e.what();
        return err;
    }
}

json AppServices::UserPsqlService::roles(int id)
{
    User *user = new User();

    try
    {

        // json result = User::fastWhere<User>("id", ">" , "0")->hasOne(Team())->get();
        json result = User::fastWhere<User>("id", ">" , "0")->limit(1000)->get();
        // json result = user->where("id", to_string(id))->hasMany(Post())->get();
        // json result = User::staticWhere<User>("id", to_string(id))->belongsToMany(Role())->get();
        return result;
    }
    catch (std::exception &e)
    {
        json err = json::Object();
        err["error"] = e.what();
        return err;
    }
}

json UserPsqlService::create(const crow::request& req)
{
    auto data = crow::json::load(req.body);

    static Logger myLogger;
    crow::logger::setHandler(&myLogger);

    if (!data) {
        myLogger.log("Грешка: Невалиден JSON в тялото!", crow::LogLevel::Warning);
        return JsonValue{}.operator[]("error") = "Invalid JSON";
    }

    User *user = new User();
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
