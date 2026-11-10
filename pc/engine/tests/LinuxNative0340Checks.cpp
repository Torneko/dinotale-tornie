#include "../story_hybrid0320.hpp"
#include "../story_hard0330.hpp"
#include "../story_rights0320.hpp"
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
namespace fs=std::filesystem;
namespace hybrid=workshop::story_hybrid0320;
namespace hard=workshop::story_hard0330;
namespace {unsigned checks=0;void need(bool v,const char* s){++checks;if(!v)throw std::runtime_error(s);}std::string bytes(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}void write(const fs::path& p,const std::string& s){std::ofstream f(p,std::ios::binary);f<<s;need(bool(f),"Fixture write failed");}struct Held {int fd;explicit Held(const fs::path& p):fd(::open(p.c_str(),O_RDWR|O_CREAT|O_NOFOLLOW|O_CLOEXEC,0600)){need(fd>=0&&::flock(fd,LOCK_EX|LOCK_NB)==0,"Cannot hold POSIX lock fixture");}~Held(){::flock(fd,LOCK_UN);::close(fd);}};}
int main(int argc,char** argv){try{
 need(argc==2,"Fresh output directory required");const fs::path root=fs::absolute(argv[1]);need(!fs::exists(root),"Refuse existing fixture");fs::create_directories(root);const auto unicode=root/fs::path(u8"Données été 🥚");fs::create_directory(unicode);
 const auto hfile=unicode/hybrid::filename;hybrid::Journal h([]{return hybrid::Order{5,4,3,2,1,0};});h.initialize(hfile);need(h.prepare(5,1,3),"Create hybrid POSIX journal");const auto initial=bytes(hfile);
 {Held lock(fs::path(hfile.native()+".lock"));need(!h.successful_world(0,true)&&!h.view().saved&&h.view().worlds==0&&bytes(hfile)==initial,"Held POSIX lock awarded world or overwrote hybrid bytes");}
 need(h.successful_world(0,true)&&h.view().saved&&h.view().worlds==1,"Hybrid explicit retry after lock release failed");
 const auto symlinkdir=root/"symlink";fs::create_directory(symlinkdir);const auto target=symlinkdir/"original.json";write(target,"owned-target-sentinel");fs::create_symlink(target,symlinkdir/hybrid::filename);hybrid::Journal bad;bad.initialize(symlinkdir/hybrid::filename);need(!bad.view().readable&&!bad.prepare(5)&&bytes(target)=="owned-target-sentinel"&&!fs::exists(symlinkdir/(std::string(hybrid::filename)+".lock")),"Hybrid journal symlink was followed or locked");
 const auto harddir=unicode/"hard";fs::create_directory(harddir);const auto hardfile=harddir/hard::production_filename;hard::Journal j([](unsigned){return 0u;});j.initialize(hardfile);const workshop::story_hard_access0330::Evidence evidence{65535,65535,16,true,true,{}};need(j.prepare(6,evidence)&&j.new_run(std::string(32,'a'),true),"Create Hard POSIX journal");const auto hardbefore=bytes(hardfile);
 {Held lock(fs::path(hardfile.native()+".lock"));need(!j.world_finished(0,std::string(32,'a'),true)&&!j.view().saved&&j.view().run.worlds==0&&bytes(hardfile)==hardbefore,"Held POSIX lock awarded Hard receipt or overwrote bytes");}
 need(j.world_finished(0,std::string(32,'a'),true)&&j.view().saved&&j.view().run.worlds==1,"Hard explicit retry failed");
 const auto hardlinkdir=root/"hard-symlink";fs::create_directory(hardlinkdir);fs::create_symlink(hardfile,hardlinkdir/hard::production_filename);hard::Journal linked;linked.initialize(hardlinkdir/hard::production_filename);const auto hardgood=bytes(hardfile);need(!linked.view().readable&&!linked.prepare(6,evidence)&&bytes(hardfile)==hardgood,"Hard symlink target changed");
 const auto locklinkdir=root/"lock-symlink";fs::create_directory(locklinkdir);const auto lockfile=locklinkdir/hard::production_filename;fs::copy_file(hardfile,lockfile);fs::create_symlink(target,fs::path(lockfile.native()+".lock"));hard::Journal locklinked;locklinked.initialize(lockfile);need(!locklinked.prepare(6,evidence)&&!locklinked.view().saved&&bytes(lockfile)==hardgood&&bytes(target)=="owned-target-sentinel","POSIX lock symlink was followed");
 struct stat statbuf{};need(::stat(hfile.c_str(),&statbuf)==0&&(statbuf.st_mode&0777)==0600,"Published hybrid journal permissions not private0600");need(::stat(hardfile.c_str(),&statbuf)==0&&(statbuf.st_mode&0777)==0600,"Published Hard journal permissions not private0600");
 for(const auto& d:{unicode,harddir})for(const auto& item:fs::directory_iterator(d))need(item.path().filename().string().find(".tmp-")==std::string::npos,"Failed publication leaked an owned temp");
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"posixHeldLockDenials\":2,\"symlinkDenials\":3,\"unicodePaths\":true,\"journalPermissions0600\":true,\"game\":false,\"gpu\":false}"<<std::endl;return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL POSIX0340: "<<e.what()<<std::endl;return 1;}}
