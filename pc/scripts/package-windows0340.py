"""Create a fresh, ROM-free Windows public prerelease without rebuilding the game.

The immutable Windows engine 0.33.0 is copied byte-for-byte. Workshop/Core
sources are snapshotted and published self-contained; only a new small launcher
is compiled. Existing exports, source profiles and frozen runtimes are untouched.
"""
from __future__ import annotations
import argparse, hashlib, json, os, re, shutil, struct, subprocess, sys, time, uuid, zipfile
from pathlib import Path, PurePosixPath
from datetime import datetime, timezone

PC=Path(__file__).resolve().parents[1]
ROOT=PC.parent
ROM_SHA="e8a63388c38f8f0bea37e9b13a97f0898ac5496e08ca2028afb8db9a866e0ce9"
CONTENT_SHA="d5800ba53ceceb261048d72289805ed531e0f708916546febf7a41c22cdee87d"
FRESH_SHA="ccca4870f3b5f2d11519568c1f003387a62f5d13adbed9ed99c8089b74c68585"
ENGINE_SHA="8163acd89a22865616d2cb074af5a9d88db1846effb1c5eb76080b8ffd4b95af"
WORKSHOP_DIR="pc/build/workshop-0.21.0" # Last path recognized by the immutable engine0330.
RUNTIME_FILES=("DinoTale.exe","MusicRender0310.exe","SDL2.dll","dxcompiler.dll","dxil.dll")
CRT_FILES=("msvcp140.dll","msvcp140_atomic_wait.dll","vcruntime140.dll","vcruntime140_1.dll")
PROFILE_FILES={"saves/yoshisstory.n64.us.bin","menu-progress.json","general.json","graphics.json","sound.json","portable.txt","book-menu.json"}
ROOT_FILES={"DinoTale.exe","Jouer.cmd","Atelier.cmd","LIRE-MOI.txt","README.txt","contenu-sha256.json"}
PC_FILES={"portable-package.json","upstream-lock.json","windows-runtime-libraries.json","distribution/content.bps","distribution/content-patch.json","save-imports/content-default.bin","save-imports/content-import.json","build/native-special/engine-build.json"}
MAGICS={bytes.fromhex(x) for x in ("80371240","37804012","40123780")}
SYS_DLLS={"kernel32.dll","user32.dll","gdi32.dll","advapi32.dll","shell32.dll","ole32.dll","oleaut32.dll","comdlg32.dll","comctl32.dll","version.dll","winmm.dll","imm32.dll","setupapi.dll","hid.dll","ws2_32.dll","iphlpapi.dll","shlwapi.dll","dwmapi.dll","uxtheme.dll","ntdll.dll","bcrypt.dll","crypt32.dll","secur32.dll","wintrust.dll","dbghelp.dll","psapi.dll","cfgmgr32.dll","powrprof.dll","rpcrt4.dll","d3d12.dll","d3dcompiler_47.dll","dxgi.dll","d3d11.dll","dxguid.dll","msvcrt.dll","ucrtbase.dll","msimg32.dll","win32u.dll","propsys.dll","normaliz.dll","winhttp.dll","wininet.dll","urlmon.dll","dnsapi.dll","winspool.drv","mswsock.dll"}
def need(value,message):
    if not value: raise ValueError(message)
def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def read(path): return json.loads(Path(path).read_text(encoding="utf-8-sig"))
def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
def copy(src,dst,inputs):
    need(src.is_file() and not src.is_symlink(),"Missing/linked source: "+str(src))
    inputs[str(src.resolve())]=sha(src)
    dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(src,dst)
def run(argv,cwd,log,env=None,timeout=300):
    result=subprocess.run([str(a) for a in argv],cwd=cwd,env=env,capture_output=True,text=True,encoding="utf-8",errors="replace",timeout=timeout)
    log.write_text(result.stdout+result.stderr,encoding="utf-8")
    need(result.returncode==0,"Command failed; see "+str(log));return result
def allowed_file(name):
    p=PurePosixPath(name)
    if p.is_absolute() or ":" in name or "\\" in name or any(x in ("",".","..") for x in name.split("/")):return False
    if name in ROOT_FILES:return True
    if re.fullmatch(r"Licences/[A-Za-z0-9._+-]+\.(txt|rtf)",name):return True
    if not name.startswith("pc/"):return False
    n=name[3:]
    if n in PC_FILES:return True
    if n.startswith("build/native-special/"):
        tail=n.removeprefix("build/native-special/")
        if tail in RUNTIME_FILES+CRT_FILES:return True
    if n==WORKSHOP_DIR[3:]+"/YoshiStory-AtelierPC.exe":return True
    if n.startswith("profiles/content-personal/"):
        tail=n.removeprefix("profiles/content-personal/")
        if tail in PROFILE_FILES:return True
    for prefix in ("build/native-special/assets/","profiles/content-personal/assets/"):
        if n.startswith(prefix):return p.suffix.lower() in (".png",".svg",".ttf",".txt",".md",".rcss")
    return bool(re.fullmatch(r"assets/editor/(peche|mure|bleuet|poire-verte|prune|kiwi)-24x21\.png",n))
def pe_imports(path):
    b=Path(path).read_bytes();need(b[:2]==b"MZ","Not PE: "+str(path))
    off=struct.unpack_from("<I",b,0x3c)[0];need(b[off:off+4]==b"PE\0\0","Invalid PE")
    machine,count,_,_,_,optlen,_=struct.unpack_from("<HHIIIHH",b,off+4)
    need(machine==0x8664,"Not Windows x64: "+str(path));opt=off+24
    need(struct.unpack_from("<H",b,opt)[0]==0x20b,"Not PE32+")
    sections=[]
    for i in range(count):
        sec=opt+optlen+i*40;vs,va,rawsz,raw=struct.unpack_from("<IIII",b,sec+8);sections.append((va,max(vs,rawsz),raw))
    def rva(v):
        for va,size,raw in sections:
            if va<=v<va+size:return raw+v-va
        need(v<struct.unpack_from("<I",b,opt+60)[0],"RVA outside PE");return v
    imports=[]
    imp,sz=struct.unpack_from("<II",b,opt+112+8)
    if imp:
        cur=rva(imp)
        for _ in range(8192):
            desc=struct.unpack_from("<IIIII",b,cur)
            if not any(desc):break
            at=rva(desc[3]);end=b.index(0,at);imports.append(b[at:end].decode("ascii").lower());cur+=20
        else:raise ValueError("Unterminated PE imports")
    return sorted(set(imports))
def validate(folder):
    need(folder.is_dir(),"Missing staged package")
    paths=sorted(p for p in folder.rglob("*") if p.is_file())
    for p in paths:
        n=p.relative_to(folder).as_posix();need(allowed_file(n),"Unapproved file: "+n)
        need(not p.is_symlink(),"Symlink exported");need(p.suffix.lower() not in (".z64",".n64",".v64",".eep",".rdram",".wav",".pcm",".pdb",".log",".bak",".dmp"),"Private/generated file: "+n)
        need(p.read_bytes()[:4] not in MAGICS,"ROM bytes exported: "+n)
        if p.suffix.lower()==".bin":need(n in ("pc/profiles/content-personal/saves/yoshisstory.n64.us.bin","pc/save-imports/content-default.bin"),"Unexpected binary data")
    inv=read(folder/"contenu-sha256.json")
    actual={p.relative_to(folder).as_posix():sha(p) for p in paths if p.name!="contenu-sha256.json"}
    need(inv==actual,"Inventory differs")
    profile=folder/"pc/profiles/content-personal"
    need(sha(profile/"saves/yoshisstory.n64.us.bin")==FRESH_SHA and sha(folder/"pc/save-imports/content-default.bin")==FRESH_SHA,"Save not empty seed")
    progress=read(profile/"menu-progress.json")
    need(progress["valid"] and all(progress[k]==0 for k in ("storyMask","trialMask","extraYoshiMask","trialEggs")),"Forced progress")
    need(read(profile/"sound.json")=={"main_volume":3},"Volume not 3%")
    game=folder/"pc/build/native-special/DinoTale.exe";worker=game.with_name("MusicRender0310.exe")
    need(sha(game)==ENGINE_SHA,"Engine0330 changed")
    meta=read(folder/"pc/portable-package.json");patch=read(folder/"pc/distribution/content-patch.json")
    need(meta["freshSave"] and not meta["personalProgressIncluded"] and not meta["romIncluded"] and not meta["testUnlocksForced"],"Invalid private metadata")
    need(meta["engineSourceVersion"]=="0.33.0" and meta["workshopSelfContained"] and meta["volumePercent"]==3,"Provenance mismatch")
    need(patch["engineSha256"]==sha(game) and patch["patchSha256"]==sha(folder/"pc/distribution/content.bps") and patch["targetSha256"]==CONTENT_SHA and patch["sourceSha256"]==ROM_SHA,"Importer identity mismatch")
    need(sha(worker)==meta["menuMusicWorkerSha256"],"Worker missing/changed")
    native=game.parent;bundled={p.name.lower():p for p in native.glob("*.dll")}
    deps={}
    for p in [game,worker,folder/"DinoTale.exe",folder/WORKSHOP_DIR/"YoshiStory-AtelierPC.exe",*bundled.values()]:
        names=pe_imports(p);deps[p.relative_to(folder).as_posix()]=names
        for n in names:need(n in bundled or n in SYS_DLLS or n.startswith(("api-ms-win-","ext-ms-win-")),"Unresolved non-system dependency: "+n+" in "+p.name)
    for p in (folder/"pc/build/native-special/assets").rglob("*"):
        if p.is_file():need(sha(p)==sha(profile/"assets"/p.relative_to(native/"assets")),"Profile bootstrap assets mismatch")
    return {"fileCount":len(paths),"bytesUncompressed":sum(p.stat().st_size for p in paths),"peImports":deps,"romIncluded":False,"freshSave":True,"volumePercent":3,"engineUnchanged":True,"workshopSelfContained":True}

LAUNCHER=r'''#define WIN32_LEAN_AND_MEAN
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
'''
FAKE=r'''#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <filesystem>
int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int){wchar_t log[32768],cwd[32768];if(!GetEnvironmentVariableW(L"DINO_FAKE_LOG0340",log,32768))return 91;GetCurrentDirectoryW(32768,cwd);std::wofstream out{std::filesystem::path(log)};out<<GetCommandLineW()<<L"\n"<<cwd<<L"\n";return 0;}
'''
def compile_native(source,outdir,name,inputs,icon=False):
    outdir.mkdir(parents=True,exist_ok=True);cpp=outdir/(name+".cpp");cpp.write_text(source,encoding="utf-8")
    compiler=PC/"tools/llvm-20.1.8/bin/clang-cl.exe"
    vs=Path(r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat")
    cmd=[compiler,"/nologo","/std:c++20","/EHsc","/W4","/WX","/O2","/MT","/utf-8",cpp,"/Fo:"+str(outdir/(name+".obj")),"/Fe:"+str(outdir/(name+".exe")),"/link","/SUBSYSTEM:WINDOWS","user32.lib","shell32.lib"]
    if icon:
        res=PC/"build/portable-launcher-public/launcher.res";inputs[str(res)]=sha(res);cmd.append(res)
        copy(PC/"dinotale/icons/app.ico",outdir/"app.ico",inputs)
    batch=outdir/"compile.cmd";batch.write_text('@echo off\ncall "'+str(vs)+'" -no_logo -arch=x64\nif errorlevel 1 exit /b 1\n'+subprocess.list2cmdline([str(x) for x in cmd])+'\n',encoding="utf-8")
    run(["cmd.exe","/d","/c",batch],outdir,outdir/"compile.log",timeout=120)
    return outdir/(name+".exe")
def main():
    ap=argparse.ArgumentParser();ap.add_argument("--output",type=Path,required=True);ap.add_argument("--version",default="0.34.0-beta.1");ap.add_argument("--source-tag",default="v0.34.0-beta.1");args=ap.parse_args()
    need(os.name=="nt","Package on Windows");need(not args.output.exists(),"Never overwrite an export")
    need(re.fullmatch(r"[A-Za-z0-9._-]+",args.version) and re.fullmatch(r"[A-Za-z0-9._-]+",args.source_tag),"Invalid version/tag")
    runtime=PC/"build/native-solo-0.33.0";frozen=PC/"build/workshop-solo-0.31.1";seed=PC/"profiles/portable-fresh-seed-20261028/saves/yoshisstory.n64.us.bin"
    inputs={};protected={}
    for directory,manifest in ((runtime,"review-engine.json"),(frozen,"review-workshop.json")):
        review=read(directory/manifest)
        for n,h in review["files"].items():need(sha(directory/n)==h,"Frozen source differs: "+n);protected[str(directory/n)]=h
        protected[str(directory/manifest)]=sha(directory/manifest)
    need(sha(runtime/"DinoTale.exe")==ENGINE_SHA and sha(seed)==FRESH_SHA,"Unexpected engine/seed")
    args.output.mkdir(parents=True);report=PC/"build/publication-0340"/("windows-"+uuid.uuid4().hex[:12]);report.mkdir(parents=True)
    write(report/"protected-before.json",protected)
    src=report/"windows-ateliersrc"
    for name in ("Core","Workshop"):
        for p in sorted((PC/"workshop"/name).iterdir()):
            if p.is_file() and p.suffix in (".cs",".csproj"):copy(p,src/name/p.name,inputs)
    write(src/"global.json",{"sdk":{"version":"9.0.300","rollForward":"disable"}})
    publish=report/"workshop-publish"
    env=os.environ.copy();env["DOTNET_CLI_TELEMETRY_OPTOUT"]="1";env["DOTNET_NOLOGO"]="1"
    run(["dotnet","publish",src/"Workshop/Workshop.csproj","-c","Release","-r","win-x64","--self-contained","true","-p:RuntimeFrameworkVersion=9.0.5","-p:PublishSingleFile=true","-p:IncludeNativeLibrariesForSelfExtract=true","-p:DebugType=None","-p:DebugSymbols=false","-o",publish],src,report/"workshop-publish.log",env=env)
    workshop=publish/"YoshiStory-AtelierPC.exe";need(workshop.stat().st_size>90_000_000,"Workshop is not self-contained")
    launcher=compile_native(LAUNCHER,report/"launcher","DinoTale",inputs,icon=True)
    folder=args.output/("DinoTale-Tornie-"+args.version+"-Windows-x64");folder.mkdir()
    engine=folder/"pc/build/native-special";profile=folder/"pc/profiles/content-personal"
    for n in RUNTIME_FILES:copy(runtime/n,engine/n,inputs)
    copy(runtime/"engine-build.json",engine/"engine-build.json",inputs) # Keep engine version0330 honest.
    crt=ROOT/"dist/Export-0.12.4-Windows-Vierge/DinoTale-0.12.4-Windows-Vierge/pc/build/native-special"
    for n in CRT_FILES:copy(crt/n,engine/n,inputs)
    for p in sorted((runtime/"assets").rglob("*")):
        if p.is_file():
            copy(p,engine/"assets"/p.relative_to(runtime/"assets"),inputs)
            copy(p,profile/"assets"/p.relative_to(runtime/"assets"),inputs)
    copy(workshop,folder/WORKSHOP_DIR/"YoshiStory-AtelierPC.exe",inputs);copy(launcher,folder/"DinoTale.exe",inputs)
    for n in ("peche","mure","bleuet","poire-verte","prune","kiwi"):copy(PC/"assets/editor"/(n+"-24x21.png"),folder/"pc/assets/editor"/(n+"-24x21.png"),inputs)
    copy(PC/"upstream-lock.json",folder/"pc/upstream-lock.json",inputs)
    old=PC/"build/publication-0150/distribution";patch=read(old/"content-patch.json");need(patch["patchSha256"]==sha(old/"content.bps") and patch["targetSha256"]==CONTENT_SHA,"BPS source changed")
    copy(old/"content.bps",folder/"pc/distribution/content.bps",inputs)
    patch.update(version=args.version,engineSha256=ENGINE_SHA);write(folder/"pc/distribution/content-patch.json",patch)
    copy(seed,profile/"saves/yoshisstory.n64.us.bin",inputs);copy(seed,folder/"pc/save-imports/content-default.bin",inputs)
    write(folder/"pc/save-imports/content-import.json",{"seedSha256":FRESH_SHA,"testUnlocksUsed":False,"earnedUnlocksPreserved":True,"earnedYoshis":[]})
    write(profile/"menu-progress.json",{"version":1,"valid":True,"sourceSaveSha256":FRESH_SHA,"storyMask":0,"trialMask":0,"extraYoshiMask":0,"trialEggs":0})
    write(profile/"general.json",{"background_input_mode":"On","debug_mode":False,"joystick_deadzone":5,"rumble_strength":25})
    write(profile/"graphics.json",{"api_option":"D3D12","ar_option":"Original","developer_mode":False,"ds_option":0,"hpfb_option":"Off","hr_option":"Clamp16x9","hybrid_dimming":88,"msaa_option":"None","res_option":"Original","rr_manual_value":60,"rr_option":"Original","screen_filter":"None","wm_option":"Windowed"})
    write(profile/"sound.json",{"main_volume":3});write(profile/"book-menu.json",{"version":1,"language":"fr"});(profile/"portable.txt").write_text("",encoding="utf-8")
    for n,arg in (("Jouer.cmd",""),("Atelier.cmd"," --editor")):(folder/n).write_bytes(('@echo off\r\nstart "" "%~dp0DinoTale.exe"'+arg+'\r\n').encode("ascii"))
    notices={"DinoTale-GPL":"COPYING","RT64":"lib/rt64/LICENSE","N64ModernRuntime":"lib/N64ModernRuntime/COPYING","N64Recomp":"lib/N64ModernRuntime/N64Recomp/LICENSE","plume":"lib/rt64/src/contrib/plume/LICENSE","hlslpp":"lib/rt64/src/contrib/hlslpp/LICENSE","imgui":"lib/rt64/src/contrib/imgui/LICENSE.txt","implot":"lib/rt64/src/contrib/implot/LICENSE","im3d":"lib/rt64/src/contrib/im3d/LICENSE","ddspp":"lib/rt64/src/contrib/ddspp/LICENSE","nativefiledialog":"lib/rt64/src/contrib/nativefiledialog-extended/LICENSE","stb":"lib/rt64/src/contrib/stb/LICENSE","xxHash":"lib/rt64/src/contrib/xxHash/LICENSE","zstd":"lib/rt64/src/contrib/zstd/LICENSE","re-spirv":"lib/rt64/src/contrib/re-spirv/LICENSE","miniz":"lib/N64ModernRuntime/thirdparty/miniz/LICENSE","o1heap":"lib/N64ModernRuntime/thirdparty/o1heap/LICENSE","RmlUi":"lib/RecompFrontend/recompui/lib/RmlUi/LICENSE.txt","lunasvg":"lib/RecompFrontend/recompui/lib/lunasvg/LICENSE","plutovg":"lib/RecompFrontend/recompui/lib/lunasvg/plutovg/LICENSE","GamepadMotionHelpers":"lib/RecompFrontend/lib/GamepadMotionHelpers/LICENSE","SlotMap":"lib/RecompFrontend/lib/SlotMap/slot_map.h","LZMA-SDK":"thirdparty/lzma-sdk/lzma-sdk.txt","screen-filters":"licenses/screen-filters.txt","DirectXShaderCompiler":"licenses/DirectXShaderCompiler.txt","SDL2":"lib/rt64/src/contrib/mupen64plus-win32-deps/SDL2-2.26.3/COPYING.txt"}
    for n,p in notices.items():copy(PC/"dinotale"/p,folder/"Licences"/(n+".txt"),inputs)
    nuget=Path.home()/".nuget/packages"
    for s,n in (("microsoft.netcore.app.runtime.win-x64/9.0.5/LICENSE.TXT","DotNET-Runtime.txt"),("microsoft.netcore.app.runtime.win-x64/9.0.5/THIRD-PARTY-NOTICES.TXT","DotNET-ThirdParty.txt"),("microsoft.windowsdesktop.app.runtime.win-x64/9.0.5/LICENSE","DotNET-Desktop.txt")):copy(nuget/s,folder/"Licences"/n,inputs)
    redist=Path(r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Licenses\1036")
    copy(redist/"Redist.txt",folder/"Licences/VisualCpp-Redist.txt",inputs)
    (folder/"Licences/Sources.txt").write_text("DinoTale Tornie "+args.version+"\nCorresponding sources: https://github.com/Torneko/dinotale-tornie/tree/"+args.source_tag+"\nDistribution "+args.version+"; immutable Windows engine0.33.0; Workshop source snapshot0340.\nUpstream: https://github.com/wootbeer/dinotale/tree/65060b54925016f634b4e0e327878f72051cd4a9\nPinned submodules and project patches accompany the source release.\nNo Nintendo game image or game-content binary is distributed.\n",encoding="utf-8")
    common="Story/Trial, modes hybrides, 16 couleurs solo et éditeur de niveaux. Story difficile reste verrouillé par ses prérequis actuels. La coop et le Boss Battle sont inachevés et différés. Les essais humains complets de Story difficile et des nouveautés de l’éditeur restent à effectuer."
    (folder/"LIRE-MOI.txt").write_text("DinoTale Tornie "+args.version+" — préversion publique expérimentale Windows10/11 x64\n\nExtraire TOUT le dossier, puis ouvrir DinoTale.exe. Atelier.cmd ouvre l’éditeur.\nAu premier lancement, sélectionner une ROM originale USA de Yoshi’s Story. Les formats .z64, .v64 et .n64 sont reconnus.\nAucun émulateur, .NET, Python ou compilateur à installer. Import local ; aucune ROM incluse ni téléchargée.\nSauvegarde vide, aucune monnaie ni progression personnelle. Volume initial3%.\nConserver pc/profiles/content-personal pour sauvegarder sa progression ; créations dans pc/content.\n\nDistribution "+args.version+", moteur Windows0.33.0 inchangé, Atelier autonome publié depuis le snapshot0340. Le nom du dossier workshop-0.21.0 sert uniquement de chemin compatible avec ce moteur.\n"+common+"\n\nGuide FR: https://torneko.github.io/dinotale-tornie/\nGuide EN: https://torneko.github.io/dinotale-tornie/index-en.html\nSources: https://github.com/Torneko/dinotale-tornie/tree/"+args.source_tag+"\n",encoding="utf-8")
    (folder/"README.txt").write_text("DinoTale Tornie "+args.version+" — experimental public prerelease, Windows10/11 x64\n\nExtract the COMPLETE folder, then open DinoTale.exe. Atelier.cmd opens the editor.\nOn first launch, select an original USA Yoshi’s Story ROM. .z64, .v64 and .n64 formats are supported.\nNo emulator, .NET, Python or compiler installation is required. Local import; no ROM is included or downloaded.\nFresh save, no personal progression/currency. Initial volume3%.\nBack up pc/profiles/content-personal; creations are in pc/content.\n\nDistribution "+args.version+"; unchanged Windows engine0.33.0; self-contained Workshop built from snapshot0340. The workshop-0.21.0 directory name is an engine compatibility path.\nStory/Trial, hybrid modes, 16 solo colors and the level editor are included. Hard Story retains its current unlock prerequisites. Co-op and Boss Battle are unfinished and deferred. Complete natural Hard Story and editor acceptance tests remain pending.\nGuide: https://torneko.github.io/dinotale-tornie/index-en.html\nSource: https://github.com/Torneko/dinotale-tornie/tree/"+args.source_tag+"\n",encoding="utf-8")
    write(folder/"pc/portable-package.json",{"format":1,"version":args.version,"distributionVersion":args.version,"platform":"Windows x64","channel":"experimental-public-prerelease","engineSourceVersion":"0.33.0","engineSha256":ENGINE_SHA,"workshopSourceBaseline":"0.31.1 + Windows-preserving Core0340","workshopSha256":sha(workshop),"workshopSelfContained":True,"workshopRuntime":".NET9.0.5 win-x64 bundled","workshopCompatibilityDirectory":"build/workshop-0.21.0","launcherSha256":sha(launcher),"menuMusicWorkerSha256":sha(engine/"MusicRender0310.exe"),"freshSave":True,"personalProgressIncluded":False,"testUnlocksForced":False,"volumePercent":3,"romIncluded":False,"originalRomRequired":True,"saveFolder":"pc/profiles/content-personal","coopAndBossBattleDeferred":True,"manualUserAccepted":False,"releaseReady":False,"publicationAuthorized":True,"sourceTag":args.source_tag,"pendingManualTests":["Natural Hard Story six worlds and Bowser","Natural Hard Story egg placements and recruitment","Editor/custom metallic progression acceptance"]})
    write(folder/"pc/windows-runtime-libraries.json",{"schema":1,"bundled":{n:sha(engine/n) for n in CRT_FILES+("SDL2.dll","dxcompiler.dll","dxil.dll")},"systemDependencies":"Windows10/11 x64 graphics/OS drivers; D3D12 by default","workshop":"Self-contained .NET9 Desktop win-x64; no global runtime required","engineAndWorkerCopiedUnmodified":True})
    write(folder/"contenu-sha256.json",{p.relative_to(folder).as_posix():sha(p) for p in sorted(folder.rglob("*")) if p.is_file()})
    write(report/"sources.json",inputs)
    result=validate(folder)
    archive=args.output/(folder.name+".zip")
    with zipfile.ZipFile(archive,"x",zipfile.ZIP_DEFLATED,compresslevel=6) as z:
        for p in sorted(folder.rglob("*")):
            if p.is_file():z.write(p,p.relative_to(args.output))
    with zipfile.ZipFile(archive) as z:need(z.testzip() is None,"ZIP CRC error")
    need(all(sha(Path(p))==h for p,h in inputs.items()),"Packaging source changed")
    need(all(sha(Path(p))==h for p,h in protected.items()),"Frozen runtime changed")
    write(report/"sources.json",inputs);write(report/"protected-after.json",protected)
    result.update(archive=str(archive),archiveSha256=sha(archive),archiveBytes=archive.stat().st_size,package=str(folder),report=str(report),createdUtc=datetime.now(timezone.utc).isoformat(),nativeGameRebuilt=False,gameOrGpuStarted=False,frozenSourcesUnchanged=True,sourceTag=args.source_tag,workshopPublishCommandLog=str(report/"workshop-publish.log"),launcherSource=str(report/"launcher/DinoTale.cpp"),cpuImportTests="pending separate verifier",sourceSnapshot=str(src))
    write(args.output/"package-report.json",result);write(report/"package-report.json",result);print(json.dumps(result,ensure_ascii=False,indent=2))
if __name__=="__main__":main()

