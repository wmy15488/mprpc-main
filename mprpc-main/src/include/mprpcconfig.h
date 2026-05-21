#pragma once

#include <string>
#include <unordered_map>

class MprpcConfig {
public:
  void LoadConfigFile(const char* config_file);

  std::string Load(const std::string& key);

private:
  void Trim(std::string& src_buf);

private:
  std::unordered_map<std::string, std::string> config_map_;
};
