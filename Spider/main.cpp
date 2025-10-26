#include "Spider.h"

#include <Config.h>


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
      int port = std::stoi(config.Get("database", "port"));
      std::string dbname = config.Get("database", "dbname");
      std::string user = config.Get("database", "user");
      std::string password = config.Get("database", "password");
      Database db(host, port, dbname, user, password, std::thread::hardware_concurrency() - 1);

      std::string start_url = config.Get("spider", "start_url");
      int max_depth = std::stoi(config.Get("spider", "max_depth"));
      Spider spider(start_url, max_depth, db);
      
      std::cout << "Запускаем паука...\n";
      spider.Run();
      std::cout << "Паук завершил работу.\n";
   }
   catch (const std::exception& e)
   {
      std::cerr << "Error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}