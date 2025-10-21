#pragma once
#include <iostream>
#include <regex>
#include <sstream>
#include <string>

#include <Database.h>
#include <Utils.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>


namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class HttpServer
{
public:
   HttpServer(int port, Database& db);
   void Run();

private:
   std::string BuildSearchForm();
   std::string BuildResultsPage(const std::vector<std::pair<std::string, int>>& results);
   std::vector<std::string> ParseQuery(const std::string& query_str);

   std::vector<std::pair<std::string, int>> PerformSearch(
      Database& db,
      const std::vector<std::string>& words);

   void HandleRequest(
      http::request<http::string_body>& req,
      http::response<http::string_body>& res,
      Database& db);

   int Port;
   Database& Db;
};