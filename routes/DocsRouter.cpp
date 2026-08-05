#include "DocsRouter.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace Routes
{
    // ----------------------------------------------------------------------
    // Languages
    // ----------------------------------------------------------------------
    const std::vector<std::string> &DocsRouter::languages()
    {
        static const std::vector<std::string> langs = {"en", "bg", "ru", "es", "tr", "pt"};
        return langs;
    }

    const std::string &DocsRouter::defaultLang()
    {
        static const std::string d = "en";
        return d;
    }

    // ----------------------------------------------------------------------
    // UI strings (EN + BG fully translated; others fall back to EN)
    // ----------------------------------------------------------------------
    const std::map<std::string, std::map<std::string, std::string>> &DocsRouter::strings()
    {
        static const std::map<std::string, std::map<std::string, std::string>> S = {
            {"t_nav_home",            {{"en", "Home"},            {"bg", "Начало"},        {"ru", "Главная"},     {"es", "Inicio"},        {"tr", "Ana sayfa"},   {"pt", "Início"}}},
            {"t_nav_getting_started", {{"en", "Getting Started"}, {"bg", "Начален пристъп"}, {"ru", "Начало работы"}, {"es", "Primeros pasos"},{"tr", "Başlangıç"},  {"pt", "Primeiros passos"}}},
            {"t_nav_guides",          {{"en", "Guides"},          {"bg", "Ръководства"},   {"ru", "Руководства"}, {"es", "Guías"},         {"tr", "Kılavuzlar"},  {"pt", "Guias"}}},
            {"t_nav_garvan",          {{"en", "Garvan"},          {"bg", "Гарван"},        {"ru", "Гарван"},      {"es", "Garvan"},        {"tr", "Garvan"},      {"pt", "Garvan"}}},
            {"t_nav_reference",       {{"en", "API Reference"},   {"bg", "API справка"},   {"ru", "Справочник API"}, {"es", "Referencia API"}, {"tr", "API Referansı"}, {"pt", "Referência da API"}}},

            {"t_side_project_templates", {{"en", "Project templates"}, {"bg", "Шаблони за проекти"}, {"ru", "Шаблоны проектов"}, {"es", "Plantillas de proyecto"}, {"tr", "Proje şablonları"}, {"pt", "Modelos de projeto"}}},
            {"t_side_first_app",         {{"en", "Your first application"}, {"bg", "Вашето първо приложение"}, {"ru", "Ваше первое приложение"}, {"es", "Tu primera aplicación"}, {"tr", "İlk uygulamanız"}, {"pt", "A sua primeira aplicação"}}},
            {"t_side_simple_webpage",    {{"en", "A simple webpage"}, {"bg", "Проста уеб страница"}, {"ru", "Простая веб-страница"}, {"es", "Una página web simple"}, {"tr", "Basit bir web sayfası"}, {"pt", "Uma página web simples"}}},
            {"t_side_routes",            {{"en", "Routes"}, {"bg", "Маршрути"}, {"ru", "Маршруты"}, {"es", "Rutas"}, {"tr", "Rotalar"}, {"pt", "Rotas"}}},
            {"t_side_logging",           {{"en", "Logging"}, {"bg", "Логиране"}, {"ru", "Логирование"}, {"es", "Registro"}, {"tr", "Günlükleme"}, {"pt", "Registo"}}},
            {"t_side_templating",        {{"en", "Templating (Mustache)"}, {"bg", "Шаблони (Mustache)"}, {"ru", "Шаблоны (Mustache)"}, {"es", "Plantillas (Mustache)"}, {"tr", "Şablonlar (Mustache)"}, {"pt", "Templates (Mustache)"}}},
            {"t_side_query_string",      {{"en", "Query strings"}, {"bg", "Параметри в URL"}, {"ru", "Параметры URL"}, {"es", "Cadenas de consulta"}, {"tr", "Sorgu dizeleri"}, {"pt", "Strings de consulta"}}},
            {"t_side_static",            {{"en", "Static files"}, {"bg", "Статични файлове"}, {"ru", "Статические файлы"}, {"es", "Archivos estáticos"}, {"tr", "Statik dosyalar"}, {"pt", "Ficheiros estáticos"}}},
            {"t_side_compression",       {{"en", "Compression"}, {"bg", "Компресия"}, {"ru", "Сжатие"}, {"es", "Compresión"}, {"tr", "Sıkıştırma"}, {"pt", "Compressão"}}},
            {"t_side_testing",           {{"en", "Writing tests"}, {"bg", "Тестване"}, {"ru", "Написание тестов"}, {"es", "Escribir pruebas"}, {"tr", "Test yazma"}, {"pt", "Escrever testes"}}},
            {"t_side_auth",              {{"en", "HTTP authorization"}, {"bg", "HTTP оторизация"}, {"ru", "HTTP авторизация"}, {"es", "Autorización HTTP"}, {"tr", "HTTP yetkilendirme"}, {"pt", "Autorização HTTP"}}},
            {"t_side_included_mw",       {{"en", "Included middlewares"}, {"bg", "Вградени middleware-и"}, {"ru", "Встроенные middleware"}, {"es", "Middlewares incluidos"}, {"tr", "Dahili middleware'ler"}, {"pt", "Middlewares incluídos"}}},
            {"t_side_proxies",           {{"en", "Proxies"}, {"bg", "Прокси сървъри"}, {"ru", "Прокси-серверы"}, {"es", "Proxies"}, {"tr", "Proxy sunucular"}, {"pt", "Proxies"}}},

            {"t_side_env",         {{"en", ".env configuration"}, {"bg", ".env конфигурация"}, {"ru", "Конфигурация .env"}, {"es", "Configuración .env"}, {"tr", ".env yapılandırması"}, {"pt", "Configuração .env"}}},
            {"t_side_orm",         {{"en", "ORM / Query Builder"}, {"bg", "ORM / Query Builder"}, {"ru", "ORM / Query Builder"}, {"es", "ORM / Query Builder"}, {"tr", "ORM / Query Builder"}, {"pt", "ORM / Query Builder"}}},
            {"t_side_models",      {{"en", "Models"}, {"bg", "Модели"}, {"ru", "Модели"}, {"es", "Modelos"}, {"tr", "Modeller"}, {"pt", "Modelos"}}},
            {"t_side_controllers", {{"en", "Controllers"}, {"bg", "Контролери"}, {"ru", "Контроллеры"}, {"es", "Controladores"}, {"tr", "Kontrolcüler"}, {"pt", "Controladores"}}},
            {"t_side_services",    {{"en", "Services"}, {"bg", "Услуги"}, {"ru", "Сервисы"}, {"es", "Servicios"}, {"tr", "Servisler"}, {"pt", "Serviços"}}},
            {"t_side_migrations",  {{"en", "Migrations"}, {"bg", "Миграции"}, {"ru", "Миграции"}, {"es", "Migraciones"}, {"tr", "Göçler"}, {"pt", "Migrações"}}},
            {"t_side_databases",   {{"en", "Database drivers"}, {"bg", "Драйвери за бази данни"}, {"ru", "Драйверы БД"}, {"es", "Controladores de BD"}, {"tr", "Veritabanı sürücüleri"}, {"pt", "Controladores de BD"}}},
            {"t_side_helpers",     {{"en", "Helpers"}, {"bg", "Помощни инструменти"}, {"ru", "Утилиты"}, {"es", "Utilidades"}, {"tr", "Yardımcılar"}, {"pt", "Utilitários"}}},
            {"t_side_jobs",        {{"en", "Jobs"}, {"bg", "Jobs"}, {"ru", "Jobs"}, {"es", "Jobs"}, {"tr", "Görevler"}, {"pt", "Jobs"}}},
            {"t_side_events",      {{"en", "Events"}, {"bg", "Events"}, {"ru", "События"}, {"es", "Eventos"}, {"tr", "Olaylar"}, {"pt", "Eventos"}}},
            {"t_side_mail",        {{"en", "Mail (SMTP)"}, {"bg", "Mail (SMTP)"}, {"ru", "Почта (SMTP)"}, {"es", "Correo (SMTP)"}, {"tr", "E-posta (SMTP)"}, {"pt", "E-mail (SMTP)"}}},
            {"t_side_events_and_jobs", {{"en", "Events & Jobs walkthrough"}, {"bg", "Events & Jobs — стъпка по стъпка"}, {"ru", "Events & Jobs — пошагово"}, {"es", "Events & Jobs — recorrido"}, {"tr", "Events & Jobs — adım adım"}, {"pt", "Events & Jobs — passo a passo"}}},
            {"t_side_api_index",   {{"en", "API index"}, {"bg", "API индекс"}, {"ru", "Индекс API"}, {"es", "Índice de API"}, {"tr", "API dizini"}, {"pt", "Índice da API"}}},

            {"t_on_this_page",        {{"en", "On this page"}, {"bg", "На тази страница"}, {"ru", "На этой странице"}, {"es", "En esta página"}, {"tr", "Bu sayfada"}, {"pt", "Nesta página"}}},
            {"t_license",             {{"en", "License"}, {"bg", "Лиценз"}, {"ru", "Лицензия"}, {"es", "Licencia"}, {"tr", "Lisans"}, {"pt", "Licença"}}},
            {"t_privacy",             {{"en", "Privacy"}, {"bg", "Поверителност"}, {"ru", "Конфиденциальность"}, {"es", "Privacidad"}, {"tr", "Gizlilik"}, {"pt", "Privacidade"}}},
            {"t_built_with",          {{"en", "Built on top of"}, {"bg", "Изградено върху"}, {"ru", "Построено на"}, {"es", "Construido sobre"}, {"tr", "Üzerine inşa edildi"}, {"pt", "Construído sobre"}}},
            {"t_translation_pending", {{"en", "This page is not yet translated to your language. Showing the English version."},
                                       {"bg", "Тази страница все още не е преведена на вашия език. Показва се английската версия."},
                                       {"ru", "Эта страница ещё не переведена на ваш язык. Показывается английская версия."},
                                       {"es", "Esta página aún no está traducida a tu idioma. Mostrando la versión en inglés."},
                                       {"tr", "Bu sayfa henüz dilinize çevrilmedi. İngilizce sürümü gösteriliyor."},
                                       {"pt", "Esta página ainda não foi traduzida para o seu idioma. A mostrar a versão em inglês."}}},
            {"t_404_title",           {{"en", "Page not found"}, {"bg", "Страницата не е намерена"}, {"ru", "Страница не найдена"}, {"es", "Página no encontrada"}, {"tr", "Sayfa bulunamadı"}, {"pt", "Página não encontrada"}}},
            {"t_404_body",            {{"en", "The page you requested does not exist."}, {"bg", "Страницата, която поискахте, не съществува."}, {"ru", "Запрошенная страница не существует."}, {"es", "La página que solicitó no existe."}, {"tr", "İstediğiniz sayfa mevcut değil."}, {"pt", "A página solicitada não existe."}}},
            {"t_prev",                {{"en", "Previous"}, {"bg", "Предишна"}, {"ru", "Предыдущая"}, {"es", "Anterior"}, {"tr", "Önceki"}, {"pt", "Anterior"}}},
            {"t_next",                {{"en", "Next"}, {"bg", "Следваща"}, {"ru", "Следующая"}, {"es", "Siguiente"}, {"tr", "Sonraki"}, {"pt", "Próxima"}}},
        };
        return S;
    }

    // ----------------------------------------------------------------------
    // Page index
    // ----------------------------------------------------------------------
    const std::unordered_map<std::string, PageMeta> &DocsRouter::pages()
    {
        static const std::unordered_map<std::string, PageMeta> P = {
            // -------- Home --------
            {"home", {
                "home", "home",
                "Home", "Начало",
                "Garvan C++ — a fast, easy to use C++ web framework built on Crow.",
                "Garvan C++ — бърз и лесен за използване C++ уеб фреймуърк, базиран на Crow.",
                {},
                "", "getting_started/setup_linux"
            }},

            // -------- Getting Started --------
            {"getting_started/setup_linux", {
                "getting_started/setup_linux", "getting_started",
                "Setup on Linux", "Инсталация в Linux",
                "How to install and build Garvan on Linux.", "Как да инсталирате и компилирате Garvan под Linux.",
                {
                    {"requirements", "Requirements", "Изисквания", false},
                    {"install-libgarvan", "Installing libgarvan", "Инсталиране на libgarvan", false},
                    {"build-from-source", "Building from source", "Компилиране от изходен код", false},
                    {"compile-your-project", "Compiling your project", "Компилиране на проекта ви", false},
                },
                "home", "getting_started/setup_macos"
            }},
            {"getting_started/setup_macos", {
                "getting_started/setup_macos", "getting_started",
                "Setup on macOS", "Инсталация в macOS",
                "How to install and build Garvan on macOS.", "Как да инсталирате и компилирате Garvan под macOS.",
                {
                    {"requirements", "Requirements", "Изисквания", false},
                    {"homebrew", "Using Homebrew", "С Homebrew", false},
                    {"compile-your-project", "Compiling your project", "Компилиране на проекта ви", false},
                },
                "getting_started/setup_linux", "getting_started/setup_windows"
            }},
            {"getting_started/setup_windows", {
                "getting_started/setup_windows", "getting_started",
                "Setup on Windows", "Инсталация в Windows",
                "How to install and build Garvan on Windows.", "Как да инсталирате и компилирате Garvan под Windows.",
                {
                    {"requirements", "Requirements", "Изисквания", false},
                    {"vcpkg", "Using vcpkg", "С vcpkg", false},
                    {"msvc", "Compiling with MSVC", "Компилация с MSVC", false},
                },
                "getting_started/setup_macos", "getting_started/project_templates"
            }},
            {"getting_started/project_templates", {
                "getting_started/project_templates", "getting_started",
                "Project templates", "Шаблони за проекти",
                "Starter project templates for Garvan.", "Стартови шаблони за проекти с Garvan.",
                {
                    {"directory-layout", "Directory layout", "Структура на директорията", false},
                    {"cmake-template", "CMake template", "CMake шаблон", false},
                },
                "getting_started/setup_windows", "getting_started/your_first_application"
            }},
            {"getting_started/your_first_application", {
                "getting_started/your_first_application", "getting_started",
                "Your first application", "Вашето първо приложение",
                "Write a Hello World HTTP server with Garvan.", "Напишете „Hello World“ HTTP сървър с Garvan.",
                {
                    {"include", "1. Include", "1. Включване", false},
                    {"app-declaration", "2. App declaration", "2. Декларация на приложението", false},
                    {"adding-routes", "3. Adding routes", "3. Добавяне на маршрути", false},
                    {"running", "4. Running the app", "4. Стартиране на приложението", false},
                    {"full-example", "Full example", "Пълен пример", false},
                },
                "getting_started/project_templates", "getting_started/a_simple_webpage"
            }},
            {"getting_started/a_simple_webpage", {
                "getting_started/a_simple_webpage", "getting_started",
                "A simple webpage", "Проста уеб страница",
                "Render an HTML page with mustache templating.", "Рендиране на HTML страница с mustache.",
                {
                    {"static-page", "Static page", "Статична страница", false},
                    {"with-variable", "Page with a variable", "Страница с променлива", false},
                },
                "getting_started/your_first_application", "guides/app"
            }},

            // -------- Guides --------
            {"guides/app", {
                "guides/app", "guides",
                "App", "Приложение (App)",
                "The SimpleApp / App class — the heart of Garvan.", "Класовете SimpleApp / App — сърцето на Garvan.",
                {
                    {"simpleapp", "SimpleApp", "SimpleApp", false},
                    {"app-with-middleware", "App with middleware", "App с middleware", false},
                    {"chaining", "Method chaining", "Верижно извикване", false},
                    {"async", "Running asynchronously", "Стартиране асинхронно", false},
                },
                "getting_started/a_simple_webpage", "guides/routes"
            }},
            {"guides/routes", {
                "guides/routes", "guides",
                "Routes", "Маршрути",
                "Defining HTTP routes with CROW_ROUTE.", "Дефиниране на HTTP маршрути с CROW_ROUTE.",
                {
                    {"macro", "The CROW_ROUTE macro", "Макросът CROW_ROUTE", false},
                    {"url-parameters", "URL parameters", "URL параметри", false},
                    {"methods", "HTTP methods", "HTTP методи", false},
                    {"handler", "Handler", "Обработваща функция", false},
                    {"response", "Response", "Отговор", false},
                    {"catchall", "Catchall routes", "Catchall маршрути", false},
                },
                "guides/app", "guides/logging"
            }},
            {"guides/logging", {
                "guides/logging", "guides",
                "Logging", "Логиране",
                "Crow's logger and Garvan's Logger helper.", "Логерът на Crow и помощникът Logger на Garvan.",
                {
                    {"crow-log", "CROW_LOG_* macros", "CROW_LOG_* макроси", false},
                    {"garvan-logger", "Garvan Logger", "Garvan Logger", false},
                    {"log-levels", "Log levels", "Нива на логиране", false},
                },
                "guides/routes", "guides/json"
            }},
            {"guides/json", {
                "guides/json", "guides",
                "JSON", "JSON",
                "Reading and writing JSON with crow::json and Garvan's JsonValue.", "Четене и писане на JSON с crow::json и JsonValue на Garvan.",
                {
                    {"reading", "Reading JSON", "Четене на JSON", false},
                    {"writing", "Writing JSON", "Писане на JSON", false},
                    {"garvan-jsonvalue", "Garvan JsonValue", "JsonValue на Garvan", false},
                },
                "guides/logging", "guides/templating"
            }},
            {"guides/templating", {
                "guides/templating", "guides",
                "Templating (Mustache)", "Шаблони (Mustache)",
                "Returning HTML pages with mustache templates.", "Връщане на HTML страници с mustache шаблони.",
                {
                    {"components", "Components", "Компоненти", false},
                    {"page", "Page", "Страница", false},
                    {"context", "Context", "Контекст", false},
                    {"partials", "Partials", "Партиали", false},
                    {"returning", "Returning a template", "Връщане на шаблон", false},
                },
                "guides/json", "guides/multipart"
            }},
            {"guides/multipart", {
                "guides/multipart", "guides",
                "Multipart", "Multipart",
                "Handling multipart/form-data requests.", "Обработка на multipart/form-data заявки.",
                {
                    {"reading", "Reading multipart", "Четене на multipart", false},
                    {"file-uploads", "File uploads", "Качване на файлове", false},
                },
                "guides/templating", "guides/query_string"
            }},
            {"guides/query_string", {
                "guides/query_string", "guides",
                "Query strings", "Параметри в URL",
                "Parsing URL query parameters.", "Парсване на параметри в URL.",
                {
                    {"basic", "Basic usage", "Основна употреба", false},
                    {"multiple-values", "Multiple values", "Множество стойности", false},
                },
                "guides/multipart", "guides/middleware"
            }},
            {"guides/middleware", {
                "guides/middleware", "guides",
                "Middleware", "Middleware",
                "Writing and composing middlewares.", "Писане и комбиниране на middleware-и.",
                {
                    {"concept", "What is middleware", "Какво е middleware", false},
                    {"writing", "Writing a middleware", "Писане на middleware", false},
                    {"using", "Using middleware", "Използване на middleware", false},
                },
                "guides/query_string", "guides/ssl"
            }},
            {"guides/ssl", {
                "guides/ssl", "guides",
                "SSL", "SSL",
                "Enabling HTTPS in Garvan.", "Включване на HTTPS в Garvan.",
                {
                    {"enabling", "Enabling SSL", "Активиране на SSL", false},
                    {"certificates", "Certificates", "Сертификати", false},
                },
                "guides/middleware", "guides/static"
            }},
            {"guides/static", {
                "guides/static", "guides",
                "Static files", "Статични файлове",
                "Serving static assets from the static/ folder.", "Сервиране на статични файлове от static/.",
                {
                    {"default", "The default static handler", "Стандартният static handler", false},
                    {"customizing", "Customizing", "Персонализация", false},
                },
                "guides/ssl", "guides/blueprints"
            }},
            {"guides/blueprints", {
                "guides/blueprints", "guides",
                "Blueprints", "Blueprints",
                "Grouping routes into reusable units.", "Групиране на маршрути в преизползваеми блокове.",
                {
                    {"creating", "Creating a blueprint", "Създаване на blueprint", false},
                    {"mounting", "Mounting on the app", "Прикрепяне към приложението", false},
                },
                "guides/static", "guides/compression"
            }},
            {"guides/compression", {
                "guides/compression", "guides",
                "Compression", "Компресия",
                "HTTP response compression with gzip and deflate.", "Компресия на HTTP отговори с gzip и deflate.",
                {
                    {"enabling", "Enabling compression", "Активиране на компресия", false},
                },
                "guides/blueprints", "guides/websockets"
            }},
            {"guides/websockets", {
                "guides/websockets", "guides",
                "WebSockets", "WebSockets",
                "Real-time bidirectional communication.", "Двупосочна комуникация в реално време.",
                {
                    {"defining", "Defining a websocket route", "Дефиниране на websocket маршрут", false},
                    {"events", "Connection events", "Събития на връзка", false},
                },
                "guides/compression", "guides/base64"
            }},
            {"guides/base64", {
                "guides/base64", "guides",
                "Base64", "Base64",
                "Encoding and decoding base64.", "Кодиране и декодиране на base64.",
                {
                    {"encode", "Encoding", "Кодиране", false},
                    {"decode", "Decoding", "Декодиране", false},
                },
                "guides/websockets", "guides/testing"
            }},
            {"guides/testing", {
                "guides/testing", "guides",
                "Writing tests", "Тестване",
                "Testing your Garvan handlers.", "Тестване на handler-ите на Garvan.",
                {
                    {"unit-tests", "Unit tests", "Юнит тестове", false},
                    {"integration", "Integration tests", "Интеграционни тестове", false},
                },
                "guides/base64", "guides/auth"
            }},
            {"guides/auth", {
                "guides/auth", "guides",
                "HTTP authorization", "HTTP оторизация",
                "Basic and bearer authentication.", "Basic и bearer оторизация.",
                {
                    {"basic", "Basic auth", "Basic оторизация", false},
                    {"bearer", "Bearer tokens", "Bearer токени", false},
                },
                "guides/testing", "guides/included_middleware"
            }},
            {"guides/included_middleware", {
                "guides/included_middleware", "guides",
                "Included middlewares", "Вградени middleware-и",
                "Middlewares shipped with Garvan / Crow.", "Middleware-и, доставени с Garvan / Crow.",
                {
                    {"cors", "CORS handler", "CORS handler", false},
                    {"cookie-parser", "Cookie parser", "Cookie parser", false},
                    {"sessions", "Session middleware", "Session middleware", false},
                },
                "guides/auth", "guides/proxies"
            }},
            {"guides/proxies", {
                "guides/proxies", "guides",
                "Proxies", "Прокси сървъри",
                "Running Garvan behind nginx, Apache, or Caddy.", "Пускане на Garvan зад nginx, Apache или Caddy.",
                {
                    {"nginx", "nginx", "nginx", false},
                    {"apache", "Apache", "Apache", false},
                    {"caddy", "Caddy", "Caddy", false},
                },
                "guides/included_middleware", "guides/systemd"
            }},
            {"guides/systemd", {
                "guides/systemd", "guides",
                "systemd service", "systemd услуга",
                "Run your Garvan app on boot using systemd.", "Стартиране на Garvan приложение при boot чрез systemd.",
                {
                    {"unit-file", "Unit file", "Unit файл", false},
                    {"enable", "Enabling the service", "Активиране на услугата", false},
                },
                "guides/proxies", "guides/events_and_jobs"
            }},
            {"guides/events_and_jobs", {
                "guides/events_and_jobs", "guides",
                "Events & Jobs walkthrough", "Events & Jobs — стъпка по стъпка",
                "End-to-end: fire an event, watch the job pipeline dispatch a real SMTP mail.",
                "От край до край: fire на event, listener chain, dispatch на job и реален SMTP mail.",
                {
                    {"flow", "The full flow", "Пълният поток", false},
                    {"curl", "Trigger with curl", "Тригериране с curl", false},
                    {"logs", "Reading the server log", "Четене на server log-а", false},
                    {"troubleshoot", "Troubleshooting", "Отстраняване на проблеми", false},
                },
                "guides/systemd", "garvan/env"
            }},

            // -------- Garvan-specific --------
            {"garvan/env", {
                "garvan/env", "garvan",
                ".env configuration", ".env конфигурация",
                "Loading configuration from a .env file.", "Зареждане на конфигурация от .env файл.",
                {
                    {"loading", "Loading the file", "Зареждане на файла", false},
                    {"variables", "Common variables", "Често срещани променливи", false},
                },
                "guides/events_and_jobs", "garvan/orm"
            }},
            {"garvan/orm", {
                "garvan/orm", "garvan",
                "ORM / Query Builder", "ORM / Query Builder",
                "The Garvan query builder and ORM model layer.", "Query builder-ът и ORM слоят на Garvan.",
                {
                    {"builder", "Builder", "Builder", false},
                    {"omodel", "OModel", "OModel", false},
                    {"grammars", "Grammars per database", "Граматики за всяка база", false},
                },
                "garvan/env", "garvan/models"
            }},
            {"garvan/models", {
                "garvan/models", "garvan",
                "Models", "Модели",
                "Defining models for your tables.", "Дефиниране на модели за таблиците ви.",
                {
                    {"base-model", "BaseModel", "BaseModel", false},
                    {"relations", "Relations", "Връзки", false},
                },
                "garvan/orm", "garvan/controllers"
            }},
            {"garvan/controllers", {
                "garvan/controllers", "garvan",
                "Controllers", "Контролери",
                "Writing controllers with BaseController.", "Писане на контролери с BaseController.",
                {
                    {"base-controller", "BaseController", "BaseController", false},
                    {"rest-actions", "REST actions", "REST действия", false},
                },
                "garvan/models", "garvan/services"
            }},
            {"garvan/services", {
                "garvan/services", "garvan",
                "Services", "Услуги",
                "Business logic layer with BaseService.", "Слой с бизнес логика чрез BaseService.",
                {
                    {"base-service", "BaseService", "BaseService", false},
                    {"injection", "Using a service in a controller", "Употреба в контролер", false},
                },
                "garvan/controllers", "garvan/migrations"
            }},
            {"garvan/migrations", {
                "garvan/migrations", "garvan",
                "Migrations", "Миграции",
                "Database migrations with garvan-migrate.", "Миграции на база данни с garvan-migrate.",
                {
                    {"running", "Running migrations", "Изпълняване на миграции", false},
                    {"writing", "Writing a migration", "Писане на миграция", false},
                    {"rollback", "Rollback", "Връщане назад", false},
                },
                "garvan/services", "garvan/kalpasan"
            }},
            {"garvan/kalpasan", {
                "garvan/kalpasan", "garvan",
                "Kalpasan CLI", "Kalpasan CLI",
                "The Garvan command line tool.", "Командният инструмент на Garvan.",
                {
                    {"install", "Installing", "Инсталация", false},
                    {"commands", "Scaffolding & DB commands", "Скафолдинг и DB команди", false},
                    {"runtime-verbs", "Runtime verbs (jobs, events, config)", "Runtime verbs (jobs, events, config)", false},
                },
                "garvan/migrations", "garvan/jobs"
            }},
            {"garvan/jobs", {
                "garvan/jobs", "garvan",
                "Jobs", "Jobs",
                "Job classes, JobRegistry, JobDispatcher and the sync driver.",
                "Job класове, JobRegistry, JobDispatcher и sync driver-ът.",
                {
                    {"api", "Job API", "Job API", false},
                    {"registry", "Registering a job", "Регистрация на job", false},
                    {"dispatcher", "Dispatching", "Dispatch-ване", false},
                    {"sync-driver", "SyncDriver", "SyncDriver", false},
                    {"example", "Example: SendTestMail", "Пример: SendTestMail", false},
                },
                "garvan/kalpasan", "garvan/events"
            }},
            {"garvan/events", {
                "garvan/events", "garvan",
                "Events", "Events",
                "Events, listeners and the EventDispatcher.",
                "Events, listener-и и EventDispatcher.",
                {
                    {"api", "Event API", "Event API", false},
                    {"listener", "Writing a listener", "Писане на listener", false},
                    {"dispatcher", "Firing events", "Изстрелване на event-и", false},
                    {"provider", "Registering in a service provider", "Регистрация в service provider", false},
                    {"example", "Example: UserRegistered chain", "Пример: UserRegistered chain", false},
                },
                "garvan/jobs", "garvan/mail"
            }},
            {"garvan/mail", {
                "garvan/mail", "garvan",
                "Mail (SMTP)", "Mail (SMTP)",
                "Real HTML mail sending over SMTP via libcurl.",
                "Реален HTML mail send по SMTP през libcurl.",
                {
                    {"config", "MAIL_* configuration", "MAIL_* конфигурация", false},
                    {"ports", "Ports & encryption", "Портове и encryption", false},
                    {"html", "HTML content", "HTML съдържание", false},
                    {"debug", "Debugging the SMTP dialog", "Debug на SMTP dialog-а", false},
                    {"providers", "Provider examples", "Примери с provider-и", false},
                },
                "garvan/events", "garvan/databases"
            }},
            {"garvan/databases", {
                "garvan/databases", "garvan",
                "Database drivers", "Драйвери за бази данни",
                "MySQL, PostgreSQL, MongoDB, MonetDB and SQLite support.", "Поддръжка на MySQL, PostgreSQL, MongoDB, MonetDB и SQLite.",
                {
                    {"mysql", "MySQL", "MySQL", false},
                    {"postgres", "PostgreSQL", "PostgreSQL", false},
                    {"mongodb", "MongoDB", "MongoDB", false},
                    {"monetdb", "MonetDB", "MonetDB", false},
                    {"sqlite", "SQLite", "SQLite", false},
                },
                "garvan/mail", "garvan/helpers"
            }},
            {"garvan/helpers", {
                "garvan/helpers", "garvan",
                "Helpers", "Помощни инструменти",
                "Utility classes shipped with Garvan.", "Помощни класове, доставени с Garvan.",
                {
                    {"helper", "Helper", "Helper", false},
                    {"jsonvalue", "JsonValue", "JsonValue", false},
                    {"logger", "Logger", "Logger", false},
                },
                "garvan/databases", "reference"
            }},

            // -------- API Reference --------
            {"reference", {
                "reference", "reference",
                "API Reference", "API справка",
                "Generated API reference for Garvan.", "Генерирана API справка за Garvan.",
                {
                    {"about", "About", "Относно", false},
                    {"namespaces", "Namespaces", "Пространства от имена", false},
                },
                "garvan/helpers", ""
            }},
        };
        return P;
    }

    // ----------------------------------------------------------------------
    // Helpers
    // ----------------------------------------------------------------------
    std::string DocsRouter::tr(const std::string &key, const std::string &lang)
    {
        const auto &S = strings();
        auto it = S.find(key);
        if (it == S.end()) return key;
        auto lit = it->second.find(lang);
        if (lit != it->second.end()) return lit->second;
        auto eit = it->second.find("en");
        if (eit != it->second.end()) return eit->second;
        return key;
    }

    std::string DocsRouter::detectLang(const crow::request &req)
    {
        const auto &langs = languages();
        std::string cookie = req.get_header_value("Cookie");
        if (cookie.empty()) return defaultLang();

        // Find lang= in the cookie string
        auto pos = cookie.find("lang=");
        if (pos == std::string::npos) return defaultLang();
        pos += 5;
        auto end = cookie.find(';', pos);
        std::string val = cookie.substr(pos, (end == std::string::npos) ? std::string::npos : end - pos);

        // Trim whitespace
        while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) val.erase(0, 1);
        while (!val.empty() && (val.back()  == ' ' || val.back()  == '\t')) val.pop_back();

        if (std::find(langs.begin(), langs.end(), val) != langs.end()) return val;
        return defaultLang();
    }

    std::string DocsRouter::langCookieHeader(const std::string &lang)
    {
        std::string l = lang;
        const auto &langs = languages();
        if (std::find(langs.begin(), langs.end(), l) == langs.end()) l = defaultLang();
        return "lang=" + l + "; Path=/; Max-Age=31536000; SameSite=Lax";
    }

    static bool fileExists(const std::string &path)
    {
        std::ifstream f(path);
        return f.good();
    }

    static std::string readFile(const std::string &path)
    {
        std::ifstream f(path);
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    crow::mustache::context DocsRouter::buildContext(const std::string &page_key,
                                                     const std::string &lang)
    {
        const auto &P = pages();
        crow::mustache::context ctx;

        // Resolve the page meta (fallback to a synthetic 404 record)
        auto it = P.find(page_key);
        bool found = (it != P.end());
        const PageMeta *meta = found ? &it->second : nullptr;

        // Translation function bound for this lang
        auto t = [&](const std::string &k) { return tr(k, lang); };

        // -------- Title / description / language --------
        std::string title = "Garvan C++";
        std::string desc  = "Garvan C++ — fast and easy C++ web framework.";
        if (meta)
        {
            title = (lang == "bg" && !meta->title_bg.empty()) ? meta->title_bg : meta->title_en;
            desc  = (lang == "bg" && !meta->desc_bg.empty())  ? meta->desc_bg  : meta->desc_en;
        }
        ctx["title"]       = title;
        ctx["description"] = desc;
        ctx["lang"]        = lang;

        // -------- Translatable UI strings --------
        for (const auto &kv : strings()) ctx[kv.first] = t(kv.first);

        // Language label (uppercased code) and is_* flags
        std::string upper = lang;
        for (auto &c : upper) c = std::toupper(static_cast<unsigned char>(c));
        ctx["lang_label"] = upper;
        ctx["is_en"] = (lang == "en");
        ctx["is_bg"] = (lang == "bg");
        ctx["is_ru"] = (lang == "ru");
        ctx["is_es"] = (lang == "es");
        ctx["is_tr"] = (lang == "tr");
        ctx["is_pt"] = (lang == "pt");

        // -------- Sidebar / topnav active flags --------
        std::string section = meta ? meta->section : "";
        ctx["active_home"]            = (section == "home");
        ctx["active_getting_started"] = (section == "getting_started");
        ctx["active_guides"]          = (section == "guides");
        ctx["active_garvan"]          = (section == "garvan");
        ctx["active_reference"]       = (section == "reference");

        ctx["sec_getting_started_open"] = (section == "getting_started");
        ctx["sec_guides_open"]          = (section == "guides");
        ctx["sec_garvan_open"]          = (section == "garvan");
        ctx["sec_reference_open"]       = (section == "reference");

        // Per-page sidebar highlight (p_<basename>)
        auto setActivePage = [&](const std::string &k) {
            if (!meta) return;
            auto pos = meta->key.rfind('/');
            std::string base = (pos == std::string::npos) ? meta->key : meta->key.substr(pos + 1);
            ctx["p_" + base] = true;
        };
        setActivePage(page_key);

        // -------- TOC --------
        if (meta && !meta->toc.empty())
        {
            ctx["has_toc"] = true;
            std::vector<crow::json::wvalue> items;
            for (const auto &t_item : meta->toc)
            {
                crow::json::wvalue w;
                w["anchor"] = t_item.anchor;
                w["label"]  = (lang == "bg" && !t_item.label_bg.empty()) ? t_item.label_bg : t_item.label_en;
                w["sub"]    = t_item.sub;
                items.push_back(std::move(w));
            }
            ctx["toc_items"] = std::move(items);
        }
        else
        {
            ctx["has_toc"] = false;
        }

        // -------- Prev/Next --------
        auto setNav = [&](const std::string &key, const std::string &out_title, const std::string &out_href) {
            (void)key;
            ctx[out_title] = std::string("");
            ctx[out_href]  = std::string("");
        };
        setNav("", "prev_title", "prev_href");
        setNav("", "next_title", "next_href");

        if (meta)
        {
            if (!meta->prev_key.empty())
            {
                auto pit = P.find(meta->prev_key);
                if (pit != P.end())
                {
                    ctx["has_prev"]   = true;
                    ctx["prev_title"] = (lang == "bg" && !pit->second.title_bg.empty()) ? pit->second.title_bg : pit->second.title_en;
                    ctx["prev_href"]  = (meta->prev_key == "home") ? "/" : ("/" + meta->prev_key);
                }
            }
            if (!meta->next_key.empty())
            {
                auto nit = P.find(meta->next_key);
                if (nit != P.end())
                {
                    ctx["has_next"]   = true;
                    ctx["next_title"] = (lang == "bg" && !nit->second.title_bg.empty()) ? nit->second.title_bg : nit->second.title_en;
                    ctx["next_href"]  = (meta->next_key == "home") ? "/" : ("/" + meta->next_key);
                }
            }
        }

        // -------- Content (load page fragment with EN fallback) --------
        std::string content;
        bool translation_pending = false;
        if (found)
        {
            std::string fileLang  = "public/pages/" + lang + "/" + page_key + ".html";
            std::string fileEn    = "public/pages/en/" + page_key + ".html";
            std::string fileBg    = "public/pages/bg/" + page_key + ".html";

            std::string chosen;
            if (fileExists(fileLang)) {
                chosen = fileLang;
            } else if (lang != "en" && fileExists(fileEn)) {
                chosen = fileEn;
                translation_pending = (lang != "en");
            } else if (fileExists(fileBg)) {
                chosen = fileBg;
            }

            if (!chosen.empty())
            {
                std::string raw = readFile(chosen);
                // Render the page itself through mustache so it picks up t_* vars and shared ctx
                auto pageTpl = crow::mustache::compile(raw);
                content = pageTpl.render_string(ctx);
            }
            else
            {
                content = "<h1>" + t("t_404_title") + "</h1><p>" + t("t_404_body") + "</p>";
            }
        }
        else
        {
            content = "<h1>" + t("t_404_title") + "</h1><p>" + t("t_404_body") + "</p>";
        }

        if (translation_pending)
        {
            content = "<div class=\"admonition note\"><div class=\"admonition-title\">i18n</div><p>" +
                      t("t_translation_pending") + "</p></div>" + content;
        }

        ctx["content"] = content;
        return ctx;
    }

    std::string DocsRouter::render(const std::string &page_key,
                                   const std::string &lang)
    {
        auto ctx = buildContext(page_key, lang);
        auto layout = crow::mustache::load("_layout.html");
        return layout.render_string(ctx);
    }

    std::string DocsRouter::render404(const std::string &lang)
    {
        crow::mustache::context ctx = buildContext("home", lang); // reuse for chrome
        ctx["title"]       = tr("t_404_title", lang);
        ctx["content"]     = "<h1>" + tr("t_404_title", lang) + "</h1><p>" +
                             tr("t_404_body", lang) + "</p>";
        ctx["has_toc"]     = false;
        ctx["has_prev"]    = false;
        ctx["has_next"]    = false;
        auto layout = crow::mustache::load("_layout.html");
        return layout.render_string(ctx);
    }
}
