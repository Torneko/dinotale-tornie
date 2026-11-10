#include "story_hard_access0330.hpp"
#include "story_rights0320.hpp"
#include "hybrid_finds.hpp"
#include "metal_yoshis0300.hpp"
#include "json/json.hpp"
#include <algorithm>
#include <array>
#include <fstream>
#include <set>
#include <stdexcept>
#include <system_error>
#include <vector>
namespace workshop::story_hard_access0330 {
std::optional<unsigned> owned(const story_rights0320::View& canonical,
 const finds::View& hybrids,const metal_yoshis0300::View& metals){
 if(!canonical.present||!canonical.readable||canonical.trial!=canonical.progression.trial
   ||canonical.bw_earned!=canonical.progression.eggs||canonical.bw_earned>3
   ||!progress::valid(canonical.progression,canonical.story)
   ||!hybrids.initialized||!hybrids.readable||!hybrids.saved||!hybrids.migrated||hybrids.found>63
   ||!metals.initialized||!metals.readable||!metals.saved||metals.found>3)return {};
 return 0x3Fu|(canonical.bw_earned<<6)|(hybrids.found<<8)|(metals.found<<14);
}
bool entitled(const Evidence& e){return e.readable&&e.saved&&e.bossWidth==16&&e.owned16==all_yoshis&&e.bossWins16==all_yoshis;}
namespace {
inline constexpr size_t max_file_bytes=16*1024*1024;
inline constexpr unsigned max_runs=100000;
bool token(const std::string& s){return s.size()==32&&std::all_of(s.begin(),s.end(),[](unsigned char c){return (c>='0'&&c<='9')||(c>='a'&&c<='f');});}
Evidence read_legacy(const std::filesystem::path& path){
 Evidence result;
 try{
  if(path.empty())throw std::runtime_error("boss-evidence-path-missing");
  std::error_code error;const auto status=std::filesystem::symlink_status(path,error);
  if(error&&error!=std::errc::no_such_file_or_directory)throw std::runtime_error("boss-evidence-status-failed");
  if(!std::filesystem::exists(status)){result.readable=result.saved=true;result.reason="boss-evidence-missing";return result;}
  if(std::filesystem::is_symlink(status)||!std::filesystem::is_regular_file(status))throw std::runtime_error("boss-evidence-not-regular");
  const auto length=std::filesystem::file_size(path);if(length>max_file_bytes)throw std::runtime_error("boss-evidence-too-large");
  std::ifstream input(path,std::ios::binary);if(!input)throw std::runtime_error("boss-evidence-open-failed");
  std::string bytes(size_t(length)+1,'\0');input.read(bytes.data(),std::streamsize(bytes.size()));bytes.resize(size_t(input.gcount()));
  if(bytes.size()!=length||(!input.eof()&&!input))throw std::runtime_error("boss-evidence-changed-during-read");
  bool duplicate=false;std::vector<std::set<std::string>> keys;
  auto callback=[&](int,nlohmann::json::parse_event_t event,nlohmann::json& value){
   if(event==nlohmann::json::parse_event_t::object_start)keys.emplace_back();
   else if(event==nlohmann::json::parse_event_t::key){if(keys.empty()||!keys.back().insert(value.get<std::string>()).second)duplicate=true;}
   else if(event==nlohmann::json::parse_event_t::object_end&&!keys.empty())keys.pop_back();return true;
  };
  const auto json=nlohmann::json::parse(bytes,callback);
  if(duplicate||!json.is_object()||json.size()!=4||!json.contains("schema")||!json["schema"].is_number_unsigned()||json["schema"]!=1u
    ||!json.contains("mask14")||!json["mask14"].is_number_unsigned()||json["mask14"].get<uint64_t>()>0x3FFF
    ||!json.contains("counts14")||!json["counts14"].is_array()||json["counts14"].size()!=14
    ||!json.contains("runs")||!json["runs"].is_array()||json["runs"].size()>max_runs)throw std::runtime_error("boss-evidence-legacy-format-invalid");
  std::array<unsigned,14> counts{};std::set<std::string> identities;unsigned mask=0;
  for(const auto& row:json["runs"]){
   if(!row.is_object()||row.size()!=2||!row.contains("token")||!row["token"].is_string()||!row.contains("color")||!row["color"].is_number_unsigned()||row["color"].get<uint64_t>()>=14)throw std::runtime_error("boss-evidence-legacy-run-invalid");
   const auto identity=row["token"].get<std::string>();const auto color=row["color"].get<unsigned>();
   if(!token(identity)||!identities.insert(identity).second)throw std::runtime_error("boss-evidence-legacy-token-invalid");
   ++counts[color];mask|=1u<<color;
  }
  if(mask!=json["mask14"].get<unsigned>())throw std::runtime_error("boss-evidence-legacy-mask-mismatch");
  for(unsigned color=0;color<14;++color)if(!json["counts14"][color].is_number_unsigned()||json["counts14"][color].get<uint64_t>()!=counts[color])throw std::runtime_error("boss-evidence-legacy-count-mismatch");
  result.bossWins16=mask;result.bossWidth=14;result.readable=result.saved=true;result.reason="boss-evidence-covers-only14";
 }catch(const std::exception& error){result.readable=result.saved=false;result.reason=error.what();}
 return result;
}
}
Evidence inspect(const story_rights0320::View& canonical,const finds::View& hybrids,
 const metal_yoshis0300::View& metals,const std::filesystem::path& path){
 const auto mask=owned(canonical,hybrids,metals);
 if(!mask){Evidence rejected;rejected.reason="owned-yoshi-evidence-unavailable";return rejected;}
 auto result=read_legacy(path);result.owned16=*mask;
 if(result.readable&&*mask!=all_yoshis)result.reason="all16-yoshis-must-be-owned";
 return result;
}
} // namespace workshop::story_hard_access0330
