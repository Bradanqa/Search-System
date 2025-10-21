#include "pch.h"
#include "Database.h"


Database::Database(
   const std::string& host,
   int port,
   const std::string& dbname,
   const std::string& user,
   const std::string& password)
{
   std::string conn_str =
      "host=" + host +
      " port=" + std::to_string(port) +
      " dbname=" + dbname +
      " user=" + user +
      " password=" + password;

   connection = new pqxx::connection(conn_str);
}

Database::~Database()
{
   if (connection) {
      delete connection;
   }
}

void Database::CreateTables()
{
   pqxx::work txn(*connection);
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
}

void Database::InsertDocument(const std::string& url, const std::string& content)
{
   pqxx::work txn(*connection);
   txn.exec_params("INSERT INTO documents (url) VALUES ($1) ON CONFLICT DO NOTHING", url);
   txn.commit();
}

long Database::GetDocumentId(const std::string& url)
{
   pqxx::work txn(*connection);
   auto res = txn.exec_params("SELECT id FROM documents WHERE url = $1", url);
   
   if (res.empty()) {
      return -1;
   }

   return res[0][0].as<long>();
}

void Database::InsertWord(const std::string& word)
{
   pqxx::work txn(*connection);
   txn.exec_params("INSERT INTO words (word) VALUES ($1) ON CONFLICT DO NOTHING", word);
   txn.commit();
}

long Database::GetWordId(const std::string& word)
{
   pqxx::work txn(*connection);
   auto res = txn.exec_params("SELECT id FROM words WHERE word = $1", word);
   
   if (res.empty()) {
      return -1;
   }

   return res[0][0].as<long>();
}

void Database::InsertIndex(long doc_id, long word_id, int frequency)
{
   pqxx::work txn(*connection);
   txn.exec_params(
      "INSERT INTO index_table (doc_id, word_id, frequency) VALUES ($1, $2, $3) "
      "ON CONFLICT (doc_id, word_id) DO UPDATE SET frequency = EXCLUDED.frequency",
      doc_id, word_id, frequency
   );
   txn.commit();
}

pqxx::connection& Database::GetConnection()
{
   return *connection;
}