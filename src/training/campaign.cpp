#include "bond/training.hpp"
#include <fstream>
#include <sstream>
namespace bond {
bool Campaign::load(const std::filesystem::path& file) { std::ifstream input(file); if (!input) return false; std::vector<Lesson> loaded; std::string line; std::getline(input,line); while(std::getline(input,line)) { std::stringstream row(line); std::string field; Lesson lesson; if(!std::getline(row,field,'|')) continue; lesson.id=std::stoi(field); std::getline(row,field,'|'); lesson.module=std::stoi(field); std::getline(row,lesson.key,'|'); std::getline(row,lesson.title_key,'|'); std::getline(row,lesson.requirement_key,'|'); std::getline(row,field,'|'); lesson.target_harvest=std::stoi(field); loaded.push_back(std::move(lesson)); } lessons_=std::move(loaded); return !lessons_.empty(); }
std::optional<Lesson> Campaign::find(int id) const { for(const auto& lesson: lessons_) if(lesson.id==id) return lesson; return std::nullopt; }
}
