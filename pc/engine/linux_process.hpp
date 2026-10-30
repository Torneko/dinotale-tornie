#pragma once
#include <filesystem>
#include <string>
#include <vector>
namespace workshop::linux_process {
std::filesystem::path executable();
unsigned id();
bool launch(const std::filesystem::path&,const std::vector<std::string>&,const std::filesystem::path&);
bool wait_parent(unsigned pid);
}
