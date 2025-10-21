#pragma once
#include <string>

#include <pqxx/pqxx>


class Database
{
public:
   Database(const std::string& host,
      int port,
      const std::string& dbname,
      const std::string& user,
      const std::string& password);

   ~Database();

   void CreateTables();
   void InsertDocument(const std::string& url, const std::string& content);
   long GetDocumentId(const std::string& url);
   void InsertWord(const std::string& word);
   long GetWordId(const std::string& word);
   void InsertIndex(long doc_id, long word_id, int frequency);
   pqxx::connection& GetConnection();

private:
   pqxx::connection* connection = nullptr;
};