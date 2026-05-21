#include "mprpcconfig.h"
#include <fstream>

void MprpcConfig::LoadConfigFile(const char *config_file)
{
  std::fstream ifs(config_file);
  if (!ifs.is_open())
  {
    return;
  }
  std::string line;
  while (std::getline(ifs, line))
  {
    if (line.empty())
    {
      continue;
    }
    if (line[0] =='#')
    {
      continue;
    }
    std::string key;
    std::string value;
    int idx = line.find('=');
    if (idx == -1)
    {
      continue;
    }
    key = line.substr(0, idx);
    Trim(key);
    value = line.substr(idx + 1, line.size() - idx);
    Trim(value);
    if (key.empty())
    {
      continue;
    }
    config_map_.insert({key, value});
  }
}

std::string MprpcConfig::Load(const std::string &key)
{
  auto it = config_map_.find(key);
  if (it == config_map_.end())
  {
    return "";
  }
  else
    return it->second;
}

void MprpcConfig::Trim(std::string &src_buf)
{
  int start_idx = src_buf.find_first_not_of(' ');
  if (start_idx == std::string::npos)
  {
    src_buf.clear();
    return;
  }
  int end_idx = src_buf.find_last_not_of(' ');
  src_buf = src_buf.substr(start_idx, end_idx - start_idx + 1);
}
