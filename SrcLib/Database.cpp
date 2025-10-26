#include "pch.h"
#include "Database.h"


Database::Database(
   const std::string& host,
   int port,
   const std::string& dbname,
   const std::string& user,
   const std::string& password,
   size_t pool_size)
{
   ConnStr =
      "host=" + host +
      " port=" + std::to_string(port) +
      " dbname=" + dbname +
      " user=" + user +
      " password=" + password;

   for (size_t i = 0; i < pool_size; ++i)
   {
      ConnPool.push(std::make_shared<pqxx::connection>(ConnStr));
   }
}

Database::~Database()
{
   while (!ConnPool.empty()) {
      ConnPool.pop();
   }
}

void Database::CreateTables()
{
   auto conn = GetConnection();
   pqxx::work txn(*conn);

   txn.exec(R"(
        CREATE TABLE IF NOT EXISTS documents (
            id SERIAL PRIMARY KEY,
            url TEXT UNIQUE NOT NULL
        );
        CREATE TABLE IF NOT EXISTS words (
            id SERIAL PRIMARY KEY,
            word TEXT UNIQUE NOT NULL
        );
        CREATE TABLE IF NOT EXISTS index_table (
            doc_id INTEGER REFERENCES documents(id) ON DELETE CASCADE,
            word_id INTEGER REFERENCES words(id) ON DELETE CASCADE,
            frequency INTEGER NOT NULL,
            PRIMARY KEY (doc_id, word_id)
        );
    )");
   txn.commit();
   ReleaseConnection(conn);
}

void Database::InsertDocument(const std::string& url, const std::string& content)
{
   auto conn = GetConnection();
   pqxx::work txn(*conn);

   txn.exec_params("INSERT INTO documents (url) VALUES ($1) ON CONFLICT DO NOTHING", url);
   txn.commit();
   ReleaseConnection(conn);
}

long Database::GetDocumentId(const std::string& url)
{
   auto conn = GetConnection();
   pqxx::work txn(*conn);

   auto res = txn.exec_params("SELECT id FROM documents WHERE url = $1", url);
   ReleaseConnection(conn);

   if (res.empty()) {
      return -1;
   }

   return res[0][0].as<long>();
}

void Database::InsertWord(const std::string& word)
{
   auto conn = GetConnection();
   pqxx::work txn(*conn);

   txn.exec_params("INSERT INTO words (word) VALUES ($1) ON CONFLICT DO NOTHING", word);
   txn.commit();
   ReleaseConnection(conn);
}

long Database::GetWordId(const std::string& word)
{
   auto conn = GetConnection();
   pqxx::work txn(*conn);

   auto res = txn.exec_params("SELECT id FROM words WHERE word = $1", word);
   ReleaseConnection(conn);

   if (res.empty()) {
      return -1;
   }

   return res[0][0].as<long>();
}

void Database::InsertIndex(long doc_id, long word_id, int frequency)
{
   auto conn = GetConnection();
   pqxx::work txn(*conn);

   txn.exec_params(
      "INSERT INTO index_table (doc_id, word_id, frequency) VALUES ($1, $2, $3) "
      "ON CONFLICT (doc_id, word_id) DO UPDATE SET frequency = EXCLUDED.frequency",
      doc_id, word_id, frequency
   );
   txn.commit();
   ReleaseConnection(conn);
}

std::shared_ptr<pqxx::connection> Database::GetConnection()
{
   std::lock_guard<std::mutex> lock(PoolMutex);
   if (ConnPool.empty()) {
      return std::make_shared<pqxx::connection>(ConnStr);
   }
   auto conn = ConnPool.front();
   ConnPool.pop();
   return conn;
}


void Database::ReleaseConnection(std::shared_ptr<pqxx::connection> conn)
{
   std::lock_guard<std::mutex> lock(PoolMutex);
   ConnPool.push(conn);
}