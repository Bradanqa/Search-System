#include "pch.h"
#include "Utils.h"


std::string StripHtmlTags(const std::string& html)
{
   std::regex tag_regex("<[^>]*>");
   std::string result = std::regex_replace(html, tag_regex, " ");

   result = std::regex_replace(result, std::regex("[^\\w\\s]"), " ");
   result = std::regex_replace(result, std::regex("\\s+"), " ");

   return boost::locale::to_lower(result);
}

std::vector<std::string> TokenizeText(const std::string& text)
{
   std::vector<std::string> words;
   std::istringstream iss(text);
   std::string word;

   while (iss >> word)
   {
      if (word.size() >= 3 && word.size() <= 32) {
         words.push_back(word);
      }
   }
   return words;
}

std::set<std::string> ExtractLinks(const std::string& html, const std::string& base_url)
{
   std::regex link_regex(R"(<a\s+[^>]*href\s*=\s*["']([^"']+)["'][^>]*>)", std::regex::icase);
   std::sregex_iterator it(html.begin(), html.end(), link_regex);
   std::sregex_iterator end;

   std::set<std::string> links;
   for (; it != end; ++it) {
      std::string href = it->str(1);
      std::string full_url = ResolveUrl(base_url, href);
      if (full_url.empty()) {
         continue;
      }

      if (full_url.find("?") != std::string::npos ||
         full_url.find("#") != std::string::npos ||
         full_url.find("Special:") != std::string::npos ||
         full_url.find("action=") != std::string::npos) {
         continue;
      }

      links.insert(full_url);
   }
   return links;
}

std::string ResolveUrl(const std::string& base_url, const std::string& href)
{
   std::string link = href;
   link.erase(link.find_last_not_of(" \t\n\r") + 1);
   link.erase(0, link.find_first_not_of(" \t\n\r"));

   if (link.empty() || link[0] == '#') {
      return "";
   }

   if (link.substr(0, 4) == "http") {
      return link;
   }

   size_t proto_end = base_url.find("://");
   if (proto_end == std::string::npos) {
      return "";
   }
   std::string proto = base_url.substr(0, proto_end + 3);
   size_t host_end = base_url.find('/', proto_end + 3);
   std::string host;
   if (host_end == std::string::npos) {
      host = base_url;
   }
   else {
      host = base_url.substr(0, host_end);
   }

   if (link[0] == '/') {
      return host + link;
   }
   else
   {
      size_t last_slash = base_url.find_last_of('/');
      if (last_slash != std::string::npos && last_slash > proto_end + 3)
      {
         std::string base_path = base_url.substr(0, last_slash + 1);
         return base_path + link;
      }
      else {
         return host + "/" + link;
      }
   }
}