#include "story_hard_access0330.hpp"
#include "story_rights0320.hpp"
#include "hybrid_finds.hpp"
#include "metal_yoshis0300.hpp"
#include "json/json.hpp"
#include "boss_evidence0354.hpp"
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
bool entitled(const Evidence& e){return e.readable&&e.saved&&(e.bossWidth==16||e.bossWidth==18)&&e.owned16==all_yoshis&&e.bossWins16==all_yoshis;}
namespace {
inline constexpr size_t max_file_bytes=16*1024*1024;
Evidence read_evidence(const std::filesystem::path& path){
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
  const auto ledger=boss_evidence0354::decode(bytes);
  result.bossWins16=ledger.mask16;result.bossWidth=ledger.width;result.readable=result.saved=true;
  if(ledger.width==14)result.reason="boss-evidence-covers-only14";

 }catch(const std::exception& error){result.readable=result.saved=false;result.reason=error.what();}
 return result;
}
}
Evidence inspect(const story_rights0320::View& canonical,const finds::View& hybrids,
 const metal_yoshis0300::View& metals,const std::filesystem::path& path){
 const auto mask=owned(canonical,hybrids,metals);
 if(!mask){Evidence rejected;rejected.reason="owned-yoshi-evidence-unavailable";return rejected;}
 auto result=read_evidence(path);result.owned16=*mask;
 if(result.readable&&*mask!=all_yoshis)result.reason="all16-yoshis-must-be-owned";
 return result;
}
} // namespace workshop::story_hard_access0330
