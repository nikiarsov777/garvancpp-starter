#include "ApiRoutes.h"
#include "../app/controllers/api/UserMonetController.h"
#include "../app/controllers/api/UserMongoController.h"
#include "../app/controllers/api/UserMysqlController.h"
#include "../app/controllers/api/TeamController.h"
#include "../app/controllers/api/UserPsqlController.h"
#include "../app/controllers/api/RoleController.h"
#include "../app/controllers/api/PostController.h"

using namespace Routes;
using namespace AppControllersApi;

ApiRoutes::ApiRoutes(crow::SimpleApp &app)
{
   struct jsonresponse : crow::response
   {
      jsonresponse(const json &_body) : crow::response{_body.dump()}
      {
         add_header("Access-Control-Allow-Origin", "*");
         add_header("Access-Control-Allow-Headers", "Content-Type");
         add_header("Content-Type", "application/json");
      }
   };

   // CROW_ROUTE(app, "/users")
   // ([]()
   //  {
   //     auto header = crow::mustache::load_text("header.html");
   //     // auto ctx["header"] = header;
   //     auto page = crow::mustache::load("index.html");
   //     json data;
   //     data['header'] = header;
   //     data['name'] = "Niki Arsov";
   //     return crow::response(200, page.render(data));
   //     //
   //  });

   CROW_ROUTE(app, "/api/mysql/users")
   ([]()
    {
        cout << "UserMysqlController" << endl;
        UserMysqlController *userController = new UserMysqlController();

        cout << "jsonresponse" << endl;
        jsonresponse return_json{userController->index()};

        cout << "return_json" << endl;
        return return_json;
        //
    });

   CROW_ROUTE(app, "/api/monetdb/users")
   ([]()
    {
        cout << "UserMonetController" << endl;
        UserMonetController *userController = new UserMonetController();

        cout << "jsonresponse" << endl;
        jsonresponse return_json{userController->index()};

        cout << "return_json" << endl;
        return return_json;
        //
    });


   CROW_ROUTE(app, "/api/mongo/users")
   ([]()
    {
       cout << "MongoUserController" << endl;
        UserMongoController *userController = new UserMongoController();

       cout << "jsonresponse" << endl;
        jsonresponse return_json{userController->index()};

        cout << "return_json" << endl;
        return return_json;
        //
    });

   CROW_ROUTE(app, "/api/psql/users")
   ([]()
    {
       UserPsqlController *userController = new UserPsqlController();

       jsonresponse return_json{userController->index()};

       return return_json;
       //
    });

   CROW_ROUTE(app, "/api/psql/users")
   .methods("POST"_method)
   ([](const crow::request& req)
    {
        UserPsqlController *userController = new UserPsqlController();

        jsonresponse return_json{userController->create(req)};

        return return_json;
        //
    });

   CROW_ROUTE(app, "/api/monet/users")
   .methods("POST"_method)
   ([](const crow::request& req)
    {
        UserMonetController *userController = new UserMonetController();

        jsonresponse return_json{userController->create(req)};

        return return_json;
        //
    });

   CROW_ROUTE(app, "/api/mysql/users")
   .methods("POST"_method)
   ([](const crow::request& req)
    {
        UserMysqlController *userController = new UserMysqlController();

        jsonresponse return_json{userController->create(req)};

        return return_json;
        //
    });

   CROW_ROUTE(app, "/api/mongo/users")
       .methods("POST"_method)
       ([](const crow::request& req)
        {
            UserMongoController *userController = new UserMongoController();

            jsonresponse return_json{userController->create(req)};

            return return_json;
            //
        });


   CROW_ROUTE(app, "/api/psql/users/<int>")
   ([](int id)
    {
       UserPsqlController *userController = new UserPsqlController();

       jsonresponse return_json{userController->get(id)};

       return return_json;
       //
    });

   CROW_ROUTE(app, "/api/psql/users/<int>/posts")
   ([](int id)
    {
       UserPsqlController *userController = new UserPsqlController();

       jsonresponse return_json{userController->posts(id)};

       return return_json;
       //
    });

    CROW_ROUTE(app, "/api/psql/users/<int>/roles")
   ([](int id)
    {
       UserPsqlController *userController = new UserPsqlController();

       jsonresponse return_json{userController->roles(id)};

       return return_json;
       //
    });

   CROW_ROUTE(app, "/api/psql/teams")
   ([]()
    {
       TeamController *teamController = new TeamController();

       jsonresponse return_json{teamController->index()};

       return return_json;
       //
    });

   CROW_ROUTE(app, "/api/psql/roles")
   ([]()
    {
       RoleController *roleController = new RoleController();

       jsonresponse return_json{roleController->index()};

       return return_json;
       //
    });

   CROW_ROUTE(app, "/api/psql/posts")
   ([]()
    {
       PostController *postController = new PostController();

       jsonresponse return_json{postController->index()};

       return return_json;
       //
    });
}

ApiRoutes::~ApiRoutes()
{
}
