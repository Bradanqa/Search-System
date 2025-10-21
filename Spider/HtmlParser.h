#pragma once
#include <map>

#include <Utils.h>


class HtmlParser
{
public:
   HtmlParser(const std::string& html, const std::string& base_url);
   std::map<std::string, int> GetWordFrequencies() const;
   std::vector<std::string> GetLinks() const;

private:
   std::map<std::string, int> WordFreq;
   std::vector<std::string> Links;
};