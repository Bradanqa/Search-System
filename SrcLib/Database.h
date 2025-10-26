#pragma once
#include <string>
#include <mutex>
#include <queue>

#include <pqxx/pqxx>


class Database
{
public:
   Database(const std::string& host,
      int port,
      const std::string& dbname,
      const std::string& user,
      const std::string& password,
      size_t pool_size);

   ~Database();

   void CreateTables();
   void InsertDocument(const std::string& url, const std::string& content);
   long GetDocumentId(const std::string& url);
   void InsertWord(const std::string& word);
   long GetWordId(const std::string& word);
   void InsertIndex(long doc_id, long word_id, int frequency);

   std::shared_ptr<pqxx::connection> GetConnection();
   void ReleaseConnection(std::shared_ptr<pqxx::connection> conn);

private:
   std::string ConnStr;
   std::mutex PoolMutex;
   std::queue<std::shared_ptr<pqxx::connection>> ConnPool;
};