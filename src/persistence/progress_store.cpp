#include "bond/persistence.hpp"
#include <fstream>
#include <sstream>
namespace bond {
bool ProgressStore::save(const std::filesystem::path& file, const Progress& progress) { std::ofstream out(file); if(!out) return false; out << "version=" << progress.schema_version << '\n' << "completed="; for(auto id: progress.completed_lessons) out << id << ','; return static_cast<bool>(out); }
Progress ProgressStore::load(const std::filesystem::path& file) { Progress value; std::ifstream in(file); std::string line; if(!in) return value; while(std::getline(in,line)) { if(line.rfind("version=",0)==0) value.schema_version=std::stoi(line.substr(8)); if(line.rfind("completed=",0)==0) { std::stringstream ids(line.substr(10)); std::string id; while(std::getline(ids,id,',')) if(!id.empty()) value.completed_lessons.push_back(std::stoi(id)); } } return value; }
}
