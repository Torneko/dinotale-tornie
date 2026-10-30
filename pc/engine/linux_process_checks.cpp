#include "linux_process.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
using namespace workshop::linux_process;
int main(int argc,char** argv){
    if(argc==4 && std::string(argv[1])=="--wait"){
        if(!wait_parent(unsigned(std::stoul(argv[2]))))return 2;
        std::ofstream(argv[3])<<"waited for parent";return 0;
    }
    if(argc==3 && std::string(argv[1])=="--relay"){
        auto folder=std::filesystem::path(argv[2]);
        if(!launch(executable(),{"--wait",std::to_string(id()),(folder/"done.txt").string()},folder))return 3;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        if(std::filesystem::exists(folder/"done.txt"))return 4;
        return 0;
    }
    if(argc!=2)return 5;
    auto folder=std::filesystem::absolute(argv[1]);std::filesystem::create_directories(folder);
    if(wait_parent(0) || wait_parent(id()))return 6;
    if(launch(folder/"missing",{},folder))return 7;
    if(!launch(executable(),{"--relay",folder.string()},folder))return 8;
    for(int i=0;i<500;i++){
        if(std::filesystem::exists(folder/"done.txt")){std::cout<<"Linux spawn, Unicode arguments, working directory and parent barrier passed.\n";return 0;}
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 9;
}
