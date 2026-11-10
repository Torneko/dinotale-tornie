#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <filesystem>
#include <string>
#include <vector>
static std::wstring quote(const std::wstring& value) { std::wstring out=L"\""; unsigned slashes=0; for(wchar_t c:value){ if(c==L'\\'){++slashes;continue;} if(c==L'"'){out.append(slashes*2+1,L'\\');out+=c;}else{out.append(slashes,L'\\');out+=c;}slashes=0;}out.append(slashes*2,L'\\');return out+L"\""; }
int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int) {
 int argc=0; LPWSTR* argv=CommandLineToArgvW(GetCommandLineW(),&argc); if(!argv)return 2;
 const bool verify=argc==2&&std::wstring(argv[1])==L"--verify";
 const bool layout=argc==2&&std::wstring(argv[1])==L"--verify-layout";
 const bool editor=argc==2&&std::wstring(argv[1])==L"--editor";
 auto fail=[&](const std::wstring& msg,int code){if(!verify&&!layout)MessageBoxW(nullptr,msg.c_str(),L"DinoTale Tornie",MB_OK|MB_ICONERROR);LocalFree(argv);return code;};
 if(argc>2||(argc==2&&!verify&&!layout&&!editor))return fail(L"Ouvrir DinoTale.exe sans argument, ou Atelier.cmd pour l’éditeur.\nRun DinoTale.exe without arguments, or Atelier.cmd for the editor.",64);
 std::vector<wchar_t> buffer(32768);DWORD length=GetModuleFileNameW(nullptr,buffer.data(),DWORD(buffer.size()));if(!length||length>=buffer.size())return fail(L"Dossier introuvable / Folder unavailable",2);
 const auto root=std::filesystem::path(buffer.data()).parent_path(),pc=root/L"pc",engine=pc/L"build/native-special/DinoTale.exe",profile=pc/L"profiles/content-personal",atelier=pc/L"build/workshop-0.21.0/YoshiStory-AtelierPC.exe";
 auto file=[](const std::filesystem::path& p){std::error_code e;return std::filesystem::is_regular_file(p,e);};
 for(const auto& p:{engine,engine.parent_path()/L"MusicRender0310.exe",atelier,pc/L"distribution/content.bps",pc/L"distribution/content-patch.json",profile/L"assets/workshop-menu/book-right-approved.png",profile/L"saves/yoshisstory.n64.us.bin"})if(!file(p))return fail(L"Extraire tout le dossier du ZIP.\nExtract the complete ZIP folder.",3);
 if(layout){LocalFree(argv);return 0;}
 const bool setup=!file(profile/L"yoshisstory.n64.us.z64")||!file(engine.parent_path()/L"workshop-content.bin");
 if(verify){LocalFree(argv);return setup?3:0;}
 const auto executable=(editor||setup)?atelier:engine;
 std::wstring command=quote(executable.wstring());
 std::filesystem::path cwd=profile;
 if(editor||setup){command+=L" "+quote(pc.wstring());cwd=pc;command+=editor?L" --editor":L" --setup-rom --play-after-setup";const auto cache=pc/L"cache/dotnet";SetEnvironmentVariableW(L"DOTNET_BUNDLE_EXTRACT_BASE_DIR",cache.c_str());}
 else command+=L" --workshop-main-menu";
 STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION child{};
 if(!CreateProcessW(executable.c_str(),command.data(),nullptr,nullptr,FALSE,0,nullptr,cwd.c_str(),&startup,&child))return fail(L"Démarrage impossible / Could not start ("+std::to_wstring(GetLastError())+L")",4);
 CloseHandle(child.hThread);CloseHandle(child.hProcess);LocalFree(argv);return 0;
}
