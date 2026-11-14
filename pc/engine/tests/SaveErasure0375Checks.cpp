#include "../save_erasure0375.hpp"

#include <iostream>

#include <map>

namespace er=workshop::save_erasure0375;namespace fs=std::filesystem;

static unsigned checks=0;

static void need(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}

static void put(const fs::path& p,const std::string& b){fs::create_directories(p.parent_path());std::ofstream f(p,std::ios::binary);f<<b;need(bool(f),"fixture write");}

static std::string bytes(const fs::path& p){std::ifstream f(p,std::ios::binary);return{std::istreambuf_iterator<char>(f),{}};}

static std::map<std::string,std::string> tree(const fs::path& p){std::map<std::string,std::string> r;for(const auto& e:fs::recursive_directory_iterator(p))if(e.is_regular_file()){const auto u=e.path().lexically_relative(p).generic_u8string();r[std::string(u.begin(),u.end())]=bytes(e.path());}return r;}

template<class F>static void refused(F f){bool denied=false;try{f();}catch(const std::exception&){denied=true;}need(denied,"unsafe reset admitted");}

int main(int argc,char**argv){try{

 need(argc==2,"fresh QA output expected");const auto out=fs::absolute(argv[1]);need(!fs::exists(out),"existing QA output refused");fs::create_directories(out);

 const auto profile=out/fs::path(u8"profil-\u00e9ph\u00e9m\u00e8re");put(profile/"portable.txt","");

 const auto neighbor=out/"other-profile";put(neighbor/"saves/yoshisstory.n64.us.bin","untouched other profile");const auto other=tree(neighbor);

 const std::string token(32,'a');

 for(const char* n:{"pc-achievements.json","pc-collection.json","pc-wallet.json","pc-heart-wallet.json","pc-heart-wallet.lock","pc-hybrid-records.json","pc-hybrid-completion.json","pc-hybrid-finds.json","pc-metal-yoshis.json","pc-elemental-yoshis.json","pc-boss-challenges.json","pc-story-hybrid.json","pc-story-hard.json","pc-story-hard-preview.json","pc-trial-special-lives0350.json","pc-bonus-session0353.json","pc-bonus-session0352.json","menu-progress.json","menu-progress-story-hybrid.json","menu-progress-story-hard.json","menu-progress-story-hard-preview.json","menu-progress-bonus0352.json","book-session.json","custom-run-result.json","custom-session.json","manual-preview.txt","restauration.json"}){

  need(er::journal(n),"progress missing from scope");put(profile/n,"earned");put(profile/(std::string(n)+".bak"),"old backup");put(profile/(std::string(n)+".tmp-123"),"pending write");

 }

 for(const char* n:{"yoshisstory.n64.us.bin","yoshisstory.n64.us.story-hybrid0320.bin","yoshisstory.n64.us.story-hard0330.bin","yoshisstory.n64.us.story-hard0330-preview.bin","yoshisstory.n64.us.bonus0353.bin","yoshisstory.n64.us.bin.bak","yoshisstory.n64.us.bin.tmp"})put(profile/"saves"/n,"native old progress");

 std::map<std::string,std::string> protected_files;

 for(const char* n:{"yoshisstory.n64.us.z64","sound.json","controls.json","graphics.json","book-menu.json","book-window.json","content-profile.json","mods/config.json","content/my.yslevel","my.yslevel","saves/othergame.bin","assets/image.png"}){put(profile/n,std::string("preserve:")+n);protected_files[n]=bytes(profile/n);need(!er::journal(n),"preference in delete scope");}

 put(profile/fs::path(u8"cr\u00e9ation-\u00e9t\u00e9.yslevel"),"unicode creation");

 const auto before=tree(profile);unsigned unconfirmed_waits=0;need(!er::finish_after_parent(profile,[&](unsigned){++unconfirmed_waits;return false;}),"unconfirmed reset executed");need(tree(profile)==before&&unconfirmed_waits==0,"no request changed state or waited");

 refused([&]{er::prepare(profile,0,token);});refused([&]{er::prepare(profile,99,"bad");});need(tree(profile)==before,"invalid confirmation mutated progress");

 er::prepare(profile,99,token);const auto pending=tree(profile);unsigned waits=0;

 refused([&]{er::finish_after_parent(profile,[&](unsigned pid){++waits;need(pid==99&&tree(profile)==pending,"mutation before parent closed");return false;});});need(tree(profile)==pending&&waits==1,"blocked parent changed files");

 auto forged=er::request(profile);forged.token[0]='b';refused([&]{er::erase(profile,forged);});need(tree(profile)==pending,"wrong nonce changed files");

 need(er::finish_after_parent(profile,[&](unsigned pid){++waits;need(pid==99&&tree(profile)==pending,"old save not retained through barrier");return true;}),"confirmed reset missing");need(waits==2&&!er::request(profile).pending,"reset did not finish");

 need(bytes(profile/"saves/yoshisstory.n64.us.bin")==er::factory(),"canonical factory differs");need(er::factory().size()==2048&&er::factory().substr(0,1024)==er::factory().substr(1024),"factory checksum mirror differs");

 for(const auto& [n,b]:protected_files)need(bytes(profile/n)==b,"ROM/preferences/creation modified");need(tree(neighbor)==other,"neighbor profile changed");need(bytes(profile/fs::path(u8"cr\u00e9ation-\u00e9t\u00e9.yslevel"))=="unicode creation","Unicode creation lost");

 for(const auto& [n,b]:tree(profile))if(er::journal(fs::path(n).filename().string())){(void)b;need(n=="pc-metal-yoshis.json"||n=="pc-elemental-yoshis.json"||n=="pc-hybrid-completion.json","old journal or backup survived");}

 const auto done=tree(profile);need(!er::finish_after_parent(profile,[](unsigned){return false;})&&tree(profile)==done,"done marker repeated deletion");

 // A second explicit reset is distinct; failed handoff can cancel it safely.

 er::prepare(profile,100,token);er::cancel(profile,99);need(er::request(profile).pending,"foreign process cancelled request");er::cancel(profile,100);need(!fs::exists(profile/er::marker),"failed launch request retained");

 // All matching entries are checked before the first deletion.

 const auto guarded=out/"guarded";put(guarded/"portable.txt","");put(guarded/"pc-achievements.json","earned");fs::create_directories(guarded/"pc-collection.json");er::prepare(guarded,99,token);const auto guard_before=tree(guarded);refused([&]{er::finish_after_parent(guarded,[](unsigned){return true;});});need(tree(guarded)==guard_before,"failed preflight partly erased files");

 // A pending request survives interruption and is completed on a fresh startup.

 fs::remove(guarded/"pc-collection.json");fs::remove(guarded/"pc-achievements.json");need(er::finish_after_parent(guarded,[](unsigned){return true;})&&bytes(guarded/"saves/yoshisstory.n64.us.bin")==er::factory(),"interrupted reset not recoverable");

 const auto badtemp=out/"badtemp";put(badtemp/"portable.txt","");put(badtemp/"pc-achievements.json","earned");er::prepare(badtemp,99,token);fs::create_directory(badtemp/(std::string(er::marker)+".tmp"));const auto tempbefore=tree(badtemp);refused([&]{er::finish_after_parent(badtemp,[](unsigned){return true;});});need(tree(badtemp)==tempbefore,"bad marker temp caused partial deletion");

 const auto malformed=out/"malformed";put(malformed/"portable.txt","");put(malformed/er::marker,"unexpected");refused([&]{er::finish_after_parent(malformed,[](unsigned){return true;});});

 const auto nonportable=out/"nonportable";fs::create_directories(nonportable);refused([&]{er::prepare(nonportable,99,token);});

 put(out/"factory.bin",er::factory());std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"actualFilesystemReset\":true,\"waitBeforeFirstMutation\":true,\"pendingRecovery\":true,\"neighborProfilePreserved\":true,\"personalOrTest100Used\":false}"<<std::endl;return 0;

 }catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<": "<<e.what()<<std::endl;return 1;}}

