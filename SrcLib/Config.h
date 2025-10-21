#pragma once
#include <fstream>
#include <map>
#include <string>


class Config
{
public:
   Config(const std::string& filename);
   std::string Get(const std::string& section, const std::string& key) const;

private:
   std::map<std::string, std::map<std::string, std::string>> Data;
   void Parse(const std::string& filename);
};