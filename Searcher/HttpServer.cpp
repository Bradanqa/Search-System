#include "HttpServer.h"


HttpServer::HttpServer(int port, Database& db)
   : Port(port), Db(db)
{}

void HttpServer::Run()
{
   try
   {
      net::io_context ioc{ 1 };
      tcp::acceptor acceptor{ ioc, {tcp::v4(), static_cast<unsigned short>(Port)} };

      while (true)
      {
         tcp::socket socket{ ioc };
         acceptor.accept(socket);

         beast::flat_buffer buffer;
         http::request<http::string_body> req;
         http::read(socket, buffer, req);

         http::response<http::string_body> res;
         res.version(req.version());
         res.keep_alive(false);

         try
         {
            HandleRequest(req, res, Db);
         }
         catch (...)
         {
            res.result(http::status::internal_server_error);
            res.body() = "<h1>Internal Error</h1>";
            res.prepare_payload();
         }

         http::write(socket, res);
      }
   }
   catch (const std::exception& e)
   {
      std::cerr << "Server error: " << e.what() << std::endl;
   }
}

std::string HttpServer::BuildSearchForm()
{
   return R"(
         <!DOCTYPE html>
         <html>
         <head><title>Search Engine</title></head>
         <body>
         <h2>Search</h2>
         <form method="POST">
         <input type="text" name="q" size="50" placeholder="Search...">
         <input type="submit" value="Find">
         </form>
         </body>
         </html>
         )";
}

std::string HttpServer::BuildResultsPage(const std::vector<std::pair<std::string, int>>& results)
{
   std::ostringstream oss;
   oss << R"(
         <!DOCTYPE html>
         <html>
         <head><title>Results</title></head>
         <body>
         <h2>Results</h2>
         )";

   if (results.empty()) {
      oss << "<p>Find nothing.</p>";
   }
   else
   {
      for (const auto& [url, score] : results) {
         oss << "<p><a href=\"" << url << "\">" << url << "</a> (relev: " << score << ")</p>\n";
      }
   }

   oss << R"(
         <hr>
         <a href="/">New search</a>
         </body>
         </html>
         )";

   return oss.str();
}

std::vector<std::string> HttpServer::ParseQuery(const std::string& query_str)
{
   std::string clean = std::regex_replace(query_str, std::regex("[^\\w\\s]"), " ");
   clean = boost::locale::to_lower(clean);
   return TokenizeText(clean);
}

std::vector<std::pair<std::string, int>> HttpServer::PerformSearch(Database& db, const std::vector<std::string>& words)
{
   if (words.empty()) {
      return {};
   }

   pqxx::work txn(db.GetConnection());
   std::ostringstream sql;
   sql << "SELECT d.url, SUM(i.frequency) AS total_freq "
      << "FROM documents d "
      << "JOIN index_table i ON d.id = i.doc_id "
      << "JOIN words w ON i.word_id = w.id "
      << "WHERE w.word IN (";

   for (size_t i = 0; i < words.size(); ++i)
   {
      if (i > 0) {
         sql << ",";
      }
      sql << "'" << words[i] << "'";
   }
   sql << ") GROUP BY d.url HAVING COUNT(DISTINCT w.word) = " << words.size()
      << " ORDER BY total_freq DESC LIMIT 10";

   auto res = txn.exec(sql.str());

   std::vector<std::pair<std::string, int>> results;
   for (const auto& row : res) {
      results.emplace_back(row[0].as<std::string>(), row[1].as<int>());
   }

   return results;
}

void HttpServer::HandleRequest(http::request<http::string_body>& req, http::response<http::string_body>& res, Database& db)
{
   if (req.method() == http::verb::get)
   {
      res.set(http::field::content_type, "text/html; charset=utf-8");
      res.body() = BuildSearchForm();
   }
   else if (req.method() == http::verb::post)
   {
      std::string query;
      if (req.body().find("q=") != std::string::npos)
      {
         size_t pos = req.body().find("q=") + 2;
         size_t end = req.body().find('&', pos);
         if (end == std::string::npos) {
            end = req.body().size();
         }
         query = req.body().substr(pos, end - pos);
         size_t plus;
         while ((plus = query.find('+')) != std::string::npos) {
            query.replace(plus, 1, " ");
         }
      }
      auto words = ParseQuery(query);
      auto results = PerformSearch(db, words);
      res.set(http::field::content_type, "text/html; charset=utf-8");
      res.body() = BuildResultsPage(results);
   }
   else
   {
      res.result(http::status::bad_request);
      res.body() = "Bad Request";
   }
   res.prepare_payload();
}
