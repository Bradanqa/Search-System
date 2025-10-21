#include "Spider.h"


Spider::Spider(const std::string& start_url, int max_depth, Database& db)
   : StartUrl(start_url), MaxDepth(max_depth), Db(db)
{
}

std::string Spider::FetchPage(const std::string& url)
{
   namespace beast = boost::beast;
   namespace http = beast::http;
   namespace net = boost::asio;
   using tcp = net::ip::tcp;

   if (url.substr(0, 7) != "http://") {
      return "";
   }

   size_t host_start = 7;
   size_t path_start = url.find('/', host_start);
   std::string host = url.substr(host_start, path_start - host_start);
   std::string path = (path_start == std::string::npos) ? "/" : url.substr(path_start);

   try
   {
      net::io_context ioc;
      tcp::resolver resolver(ioc);
      beast::tcp_stream stream(ioc);
      auto const results = resolver.resolve(host, "80");
      stream.connect(results);

      http::request<http::string_body> req{ http::verb::get, path, 11 };
      req.set(http::field::host, host);
      req.set(http::field::user_agent, "SearchEngine Spider/1.0");

      http::write(stream, req);
      beast::flat_buffer buffer;
      http::response<http::dynamic_body> res;
      http::read(stream, buffer, res);

      if (res.result() != http::status::ok) {
         return "";
      }

      return beast::buffers_to_string(res.body().data());
   }
   catch (...)
{
      return "";
   }
}

void Spider::ProcessPage(const std::string& url, int depth)
{
   {
      std::lock_guard lock(VisitedMutex);
      if (Visited.count(url)) {
         return;
      }
      Visited.insert(url);
   }

   std::cout << "Обработка: " << url << " (глубина " << depth << ")" << std::endl;

   std::string html = FetchPage(url);
   if (html.empty())
   {
      std::cout << "Не удалось загрузить: " << url << std::endl;
      return;
   }
   std::cout << "Загружено " << html.size() << " байт" << std::endl;

   Db.InsertDocument(url, html);
   long doc_id = Db.GetDocumentId(url);
   if (doc_id == -1) {
      return;
   }

   HtmlParser parser(html, url);
   auto freqs = parser.GetWordFrequencies();
   for (const auto& [word, freq] : freqs)
   {
      Db.InsertWord(word);
      long word_id = Db.GetWordId(word);
      if (word_id != -1) {
         Db.InsertIndex(doc_id, word_id, freq);
      }
   }

   if (depth < MaxDepth)
   {
      auto links = parser.GetLinks();
      {
         std::lock_guard lock(QueueMutex);
         for (const auto& link : links) {
            UrlQueue.emplace(link, depth + 1);
         }
      }
   }
}

void Spider::Worker()
{
   while (!Stop)
   {
      std::pair<std::string, int> task;
      {
         std::lock_guard lock(QueueMutex);
         if (UrlQueue.empty())
         {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
         }
         task = UrlQueue.front();
         UrlQueue.pop();
      }
      ProcessPage(task.first, task.second);
   }
}

void Spider::Run()
{
   Db.CreateTables();

   {
      std::lock_guard lock(QueueMutex);
      UrlQueue.emplace(StartUrl, 1);
   }

   int num_threads = std::min(4u, std::thread::hardware_concurrency());
   for (int i = 0; i < num_threads; ++i) {
      Workers.emplace_back(&Spider::Worker, this);
   }

   for (auto& t : Workers)
   {
      if (t.joinable()) {
         t.join();
      }
   }
}
