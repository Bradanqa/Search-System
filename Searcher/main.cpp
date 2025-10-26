#include <iostream>

#include "HttpServer.h"
#include <Config.h>

#include <boost/locale.hpp>


int main()
{
   setlocale(LC_ALL, "ru_RU");
   boost::locale::generator gen;
   std::locale::global(gen("ru_RU.UTF-8"));

   try
   {
      Config config("config.ini");

      std::cout << "Подключились к БД\n";

      std::string host = config.Get("database", "host");
      int port_db = std::stoi(config.Get("database", "port"));
      std::string dbname = config.Get("database", "dbname");
      std::string user = config.Get("database", "user");
      std::string password = config.Get("database", "password");
      Database db(host, port_db, dbname, user, password, std::thread::hardware_concurrency() - 1);

      int port_http = std::stoi(config.Get("searcher", "port"));
      HttpServer server(port_http, db);
      std::cout << "Searcher running on port " << port_http << std::endl;
      server.Run();
   }
   catch (const std::exception& e)
   {
      std::cerr << "Error: " << e.what() << std::endl;
      return 1;
   }
}