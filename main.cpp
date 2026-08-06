#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>

#include "vendors/Garvan/crow.h"
#include "vendors/Garvan/include/tools/Helper.h"
#include "vendors/Garvan/include/app/AppKernel.h"

#include "routes/ApiRoutes.h"
#include "routes/WebRoutes.h"
#include "routes/JobRoute.h"
#include "routes/EventRoute.h"
#include "routes/AdminRoute.h"

#include "app/providers/AppServiceProvider.h"
#include "app/providers/JobServiceProvider.h"
#include "app/providers/EventServiceProvider.h"

using namespace Routes;

int main()
{
   // --- CWD fix ------------------------------------------------------------
   // Прави binary-то независимо от директорията, от която е стартирано.
   // Резолвира абсолютния път на изпълнимия файл и сменя CWD на project root
   // (parent на bin/), за да работят коректно всички релативни пътища:
   //   - crow::mustache::set_global_base("public/")
   //   - DocsRouter четенето на public/pages/<lang>/...
   //   - Crow static handler (static/)
   //   - .env loader
   {
      char buf[4096];
      ssize_t len = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
      if (len == -1) {
         std::cerr << "FATAL: cannot resolve executable path via /proc/self/exe\n";
         return EXIT_FAILURE;
      }
      buf[len] = '\0';
      try {
         std::filesystem::path exePath(buf);
         // exePath = <root>/bin/app.bin  =>  project root = parent.parent
         std::filesystem::path projectRoot = exePath.parent_path().parent_path();
         std::filesystem::current_path(projectRoot);
      } catch (const std::exception &e) {
         std::cerr << "FATAL: cannot change working directory: " << e.what() << "\n";
         return EXIT_FAILURE;
      }
   }
   // -----------------------------------------------------------------------

   // --- APP_KEY guard ------------------------------------------------------
   // Refuse to start if APP_KEY is missing, empty, or whitespace-only.
   // Try the framework's .env loader first, then fall back to the process env.
   std::string app_key = Garvan::Helper::getenv("APP_KEY");
   if (app_key.empty()) {
      if (const char *env = std::getenv("APP_KEY")) {
         app_key = env;
      }
   }
   // trim
   auto not_space = [](unsigned char c) { return !std::isspace(c); };
   auto first = std::find_if(app_key.begin(), app_key.end(), not_space);
   auto last  = std::find_if(app_key.rbegin(), app_key.rend(), not_space).base();
   if (first >= last) {
      std::cerr
         << "FATAL: APP_KEY is missing or empty.\n"
         << "Set APP_KEY in your .env file (or export it in the environment) "
         << "before starting the server.\n"
         << "Example:\n"
         << "  APP_KEY=base64:<32-byte base64 secret>\n"
         << "Tip: generate one with `openssl rand -base64 32`.\n";
      return EXIT_FAILURE;
   }
   // -----------------------------------------------------------------------

   // --- Boot service providers (Jobs, Events, ...) ------------------------
   // Ordering matters: AppServiceProvider трябва да е първи, за да
   // регистрира queue driver-ите преди JobServiceProvider да опита
   // да ги ползва (макар че в Phase A register-фазата само пълни
   // registry-та — реалният dispatch идва по-късно, при requests).
   auto& kernel = Garvan::AppKernel::instance();
   kernel.addProvider(std::make_unique<AppProviders::AppServiceProvider>());
   kernel.addProvider(std::make_unique<AppProviders::JobServiceProvider>());
   kernel.addProvider(std::make_unique<AppProviders::EventServiceProvider>());
   kernel.bootAll();
   // -----------------------------------------------------------------------

   crow::SimpleApp *app = new crow::SimpleApp();
   crow::mustache::set_global_base("public/");
   // CatchallRule и WebSocket handler-ите не sync-ват per-request base с
   // global base (за разлика от TaggedRule/DynamicRule), затова explicit-но
   // сетваме и per-request base, иначе `mustache::load` използва default
   // "templates/" и връща празен template → бял екран за catchall route-и
   // като /garvan/orm.
   crow::mustache::set_base("public/");

   ApiRoutes  apiRoutes(*app);
   WebRoutes  webRoutes(*app);
   JobRoute   jobRoute(*app);
   EventRoute eventRoute(*app);
   AdminRoute adminRoute(*app);

   app->port(9090)
       .multithreaded()
       .run();

   // Graceful shutdown -- Phase B ще завърши background worker
   // thread-овете тук.
   kernel.shutdown();
}
