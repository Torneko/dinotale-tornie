#include "linux_process.hpp"
#ifdef __linux__
#include <cerrno>
#include <chrono>
#include <poll.h>
#include <signal.h>
#include <spawn.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
extern char** environ;
namespace workshop::linux_process {
std::filesystem::path executable(){std::error_code ec;return std::filesystem::read_symlink("/proc/self/exe",ec);}
unsigned id(){return unsigned(getpid());}
bool launch(const std::filesystem::path& file,const std::vector<std::string>& args,const std::filesystem::path& cwd){
    auto name=file.string();std::vector<char*> argv{const_cast<char*>(name.c_str())};
    for(const auto& arg:args)argv.push_back(const_cast<char*>(arg.c_str()));argv.push_back(nullptr);
    posix_spawn_file_actions_t actions;
    if(posix_spawn_file_actions_init(&actions)!=0)return false;
    const int chdir_result=posix_spawn_file_actions_addchdir_np(&actions,cwd.c_str());
    pid_t child=-1;
    const int result=chdir_result?chdir_result:posix_spawn(&child,file.c_str(),&actions,nullptr,argv.data(),environ);
    posix_spawn_file_actions_destroy(&actions);
    if(result)return false;
    std::thread([child]{int status;while(waitpid(child,&status,0)<0 && errno==EINTR){}}).detach();
    return true;
}
bool wait_parent(unsigned pid){
    if(!pid || pid==id())return false;
#ifdef SYS_pidfd_open
    const int fd=int(syscall(SYS_pidfd_open,pid,0));
    if(fd>=0){pollfd p{fd,POLLIN,0};int result;do{result=poll(&p,1,30000);}while(result<0 && errno==EINTR);close(fd);return result>0 && (p.revents&POLLIN);}
    if(errno==ESRCH)return true;
#endif
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(30);
    while(std::chrono::steady_clock::now()<deadline){
        if(kill(pid,0)<0 && errno==ESRCH)return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return false;
}
}
#endif
