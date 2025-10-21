#include "pch.h"
#include "Config.h"


Config::Config(const std::string& filename)
{
   Parse(filename);
}

void Config::Parse(const std::string& filename)
{
   std::ifstream file(filename);
   if (!file.is_open()) {
      throw std::runtime_error("Cannot open config file: " + filename);
   }

   std::string line, section;
   while (std::getline(file, line))
   {
      size_t comment = line.find(';');
      if (comment != std::string::npos) {
         line.erase(comment);
      }
      
      if (line.empty()) {
         continue;
      }

      if (line.front() == '[' && line.back() == ']')
      {
         section = line.substr(1, line.size() - 2);
         continue;
      }

      size_t eq = line.find('=');
      if (eq == std::string::npos) {
         continue;
      }

      std::string key = line.substr(0, eq);
      std::string value = line.substr(eq + 1);
      key.erase(key.find_last_not_of(" \t") + 1);
      value.erase(0, value.find_first_not_of(" \t"));
      Data[section][key] = value;
   }
}

std::string Config::Get(const std::string& section, const std::string& key) const
{
   auto sec_it = Data.find(section);
   if (sec_it == Data.end()) {
      throw std::runtime_error("Section not found: " + section);
   }
   
   auto key_it = sec_it->second.find(key);
   if (key_it == sec_it->second.end()) {
      throw std::runtime_error("Key not found: " + key);
   }

   return key_it->second;
}