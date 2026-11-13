#pragma once
#include "json/json.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
namespace workshop::boss_evidence0354 {
inline constexpr size_t max_file_bytes=16*1024*1024;
inline constexpr unsigned max_runs=100000;
struct Ledger {unsigned width=0;uint16_t mask16=0;uint32_t mask18=0;std::array<unsigned,16> counts16{};std::array<unsigned,18> counts18{};std::map<std::string,unsigned> runs;};
inline bool token(std::string_view s){return s.size()==32&&std::all_of(s.begin(),s.end(),[](unsigned char c){return (c>='0'&&c<='9')||(c>='a'&&c<='f');});}
// Schema1 remains read-only compatible. It never asserts the two new colors.
// Schema2 stores the same once-only receipt tokens with an explicit16-bit width.
inline Ledger decode(std::string_view bytes){
 if(bytes.size()>max_file_bytes)throw std::runtime_error("ledger-too-large");
 bool duplicate=false;std::vector<std::set<std::string>> keys;
 auto callback=[&](int,nlohmann::json::parse_event_t event,nlohmann::json& value){
  if(event==nlohmann::json::parse_event_t::object_start)keys.emplace_back();
  else if(event==nlohmann::json::parse_event_t::key){if(keys.empty()||!keys.back().insert(value.get<std::string>()).second)duplicate=true;}
  else if(event==nlohmann::json::parse_event_t::object_end&&!keys.empty())keys.pop_back();return true;
 };
 const auto j=nlohmann::json::parse(bytes,callback);
 if(duplicate||!j.is_object()||j.size()!=4||!j.contains("schema")||!j["schema"].is_number_unsigned()||(j["schema"]!=1u&&j["schema"]!=2u&&j["schema"]!=3u))throw std::runtime_error("ledger-format-invalid");
 Ledger out;out.width=j["schema"]==1u?14u:j["schema"]==2u?16u:18u;
 const char* mask_key=out.width==14?"mask14":out.width==16?"mask16":"mask18";const char* counts_key=out.width==14?"counts14":out.width==16?"counts16":"counts18";
 if(!j.contains(mask_key)||!j[mask_key].is_number_unsigned()||j[mask_key].get<uint64_t>()>((1u<<out.width)-1)||!j.contains(counts_key)||!j[counts_key].is_array()||j[counts_key].size()!=out.width||!j.contains("runs")||!j["runs"].is_array()||j["runs"].size()>max_runs)throw std::runtime_error("ledger-format-invalid");
 for(const auto& row:j["runs"]){
  if(!row.is_object()||row.size()!=2||!row.contains("token")||!row["token"].is_string()||!row.contains("color")||!row["color"].is_number_unsigned()||row["color"].get<uint64_t>()>=out.width)throw std::runtime_error("ledger-run-invalid");
  const auto identity=row["token"].get<std::string>();const auto color=row["color"].get<unsigned>();
  if(!token(identity)||!out.runs.emplace(identity,color).second)throw std::runtime_error("ledger-run-identity-invalid");
  ++out.counts18[color];out.mask18|=1u<<color;
 }
 if(out.mask18!=j[mask_key].get<unsigned>())throw std::runtime_error("ledger-mask-count-mismatch");
 for(unsigned n=0;n<out.width;++n)if(!j[counts_key][n].is_number_unsigned()||j[counts_key][n].get<uint64_t>()!=out.counts18[n])throw std::runtime_error("ledger-count-mismatch");
 out.mask16=uint16_t(out.mask18&0xFFFFu);std::copy_n(out.counts18.begin(),16,out.counts16.begin());return out;
}
} // namespace workshop::boss_evidence0354
