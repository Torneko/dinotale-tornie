#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <filesystem>
#include <string>
#include <vector>

int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR args,int){
    const bool verify=std::wstring(args)==L"--verify";
    auto fail=[&](const wchar_t* message,int code){if(!verify)MessageBoxW(nullptr,message,L"DinoTale",MB_OK|MB_ICONERROR);return code;};
    std::vector<wchar_t> path(32768);
    const DWORD length=GetModuleFileNameW(nullptr,path.data(),DWORD(path.size()));
    if(!length || length>=path.size())return fail(L"Le dossier de DinoTale est introuvable.",2);
    const auto root=std::filesystem::path(path.data()).parent_path();
    const auto engine=root/L"pc/build/native-special/DinoTale.exe";
    const auto profile=root/L"pc/profiles/content-personal";
    std::error_code error;
    if(!std::filesystem::is_regular_file(engine,error))return fail(L"Extrais le dossier portable complet, puis lance DinoTale.exe depuis ce dossier.",3);
    if(!std::filesystem::is_regular_file(profile/L"yoshisstory.n64.us.z64",error) || !std::filesystem::is_regular_file(engine.parent_path()/L"workshop-content.bin",error)){
        if(verify)return 3;if(*args)return fail(L"Lance DinoTale.exe sans argument.",64);
        const auto workshop=root/L"pc/build/workshop-0.15.0/YoshiStory-AtelierPC.exe";
        if(!std::filesystem::is_regular_file(workshop,error))return fail(L"L’outil d’import de la ROM est absent. Extrais le paquet complet.",3);
        std::wstring setup=L"\""+workshop.wstring()+L"\" \""+(root/L"pc").wstring()+L"\" --setup-rom --play-after-setup";
        STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION child{};
        if(!CreateProcessW(workshop.c_str(),setup.data(),nullptr,nullptr,FALSE,0,nullptr,(root/L"pc").c_str(),&startup,&child))return fail(L"L’outil d’import n’a pas pu démarrer.",4);
        CloseHandle(child.hThread);CloseHandle(child.hProcess);return 0;
    }
    if(verify)return 0;
    if(*args)return fail(L"Lance DinoTale.exe sans argument pour ouvrir le menu principal.",64);
    std::wstring command=L"\""+engine.wstring()+L"\" --workshop-main-menu";
    STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION child{};
    if(!CreateProcessW(engine.c_str(),command.data(),nullptr,nullptr,FALSE,0,nullptr,profile.c_str(),&startup,&child))
        return fail((L"Le jeu n'a pas pu démarrer (erreur "+std::to_wstring(GetLastError())+L").").c_str(),4);
    CloseHandle(child.hThread);CloseHandle(child.hProcess);return 0;
}
