#include "HtmlParser.h"


HtmlParser::HtmlParser(const std::string& html, const std::string& base_url)
{
   std::string clean_text = StripHtmlTags(html);
   auto words = TokenizeText(clean_text);
   for (const auto& w : words) {
      WordFreq[w]++;
   }
   auto link_set = ExtractLinks(html, base_url);
   Links = std::vector<std::string>(link_set.begin(), link_set.end());
}

std::map<std::string, int> HtmlParser::GetWordFrequencies() const
{
   return WordFreq;
}

std::vector<std::string> HtmlParser::GetLinks() const
{
   return Links;
}