#pragma once
#include <regex>
#include <set>
#include <string>
#include <vector>

#include <boost/locale.hpp>


std::string StripHtmlTags(const std::string& html);
std::vector<std::string> TokenizeText(const std::string& text);
std::set<std::string> ExtractLinks(const std::string& html, const std::string& base_url);
std::string ResolveUrl(const std::string& base_url, const std::string& href);