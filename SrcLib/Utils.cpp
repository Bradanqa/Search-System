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

   for (; it != end; ++it)
   {
      std::string href = it->str(1);
      if (href.empty() || href[0] == '#') {
         continue;
      }
      
      if (href.substr(0, 4) != "http")
      {
         if (href[0] == '/') {
            href = base_url + href;
         }
         else {
            continue;
         }
      }
      links.insert(href);
   }
   return links;
}