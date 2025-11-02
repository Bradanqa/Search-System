#pragma once
#include <atomic>
#include <iostream>
#include <mutex>
#include <queue>
#include <set>
#include <thread>

#include "HtmlParser.h"
#include <Database.h>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <openssl/ssl.h>


class Spider
{
public:
   Spider(const std::string& start_url, int max_depth, Database& db);
   void Run();

private:
   void Worker();
   std::string FetchPage(const std::string& url, int redirect_count = 0);
   void ProcessPage(const std::string& url, int depth);

   std::string StartUrl;
   int MaxDepth;
   Database& Db;
   std::queue<std::pair<std::string, int>> UrlQueue;
   std::set<std::string> Visited;
   std::mutex QueueMutex, VisitedMutex;
   std::atomic<bool> Stop = false;
   std::vector<std::thread> Workers;
};