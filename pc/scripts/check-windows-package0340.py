"""CPU-only release checks for the ROM-free Windows0340 package.

Exercises the actual self-contained importer with all three ROM byte orders.
Launch routing uses explicit fake child executables; the native game is never run.
"""
from pathlib import Path, PurePosixPath
import argparse, hashlib, importlib.util, json, os, shutil, subprocess, sys, time, uuid, zipfile
sys.dont_write_bytecode=True
SPEC=importlib.util.spec_from_file_location("package_windows0340",Path(__file__).with_name("package-windows0340.py"))
pkg=importlib.util.module_from_spec(SPEC);SPEC.loader.exec_module(pkg)
FAKE=r'''#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <filesystem>
#include <string>
static std::string utf8(const wchar_t* p){int n=WideCharToMultiByte(CP_UTF8,0,p,-1,nullptr,0,nullptr,nullptr);std::string s(static_cast<size_t>(n),'\0');WideCharToMultiByte(CP_UTF8,0,p,-1,s.data(),n,nullptr,nullptr);s.resize(s.size()-1);return s;}
int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int){wchar_t log[32768],cwd[32768];if(!GetEnvironmentVariableW(L"DINO_FAKE_LOG0340",log,32768))return 91;GetCurrentDirectoryW(32768,cwd);std::ofstream out{std::filesystem::path(log),std::ios::binary};out<<utf8(GetCommandLineW())<<"\n"<<utf8(cwd)<<"\n";return 0;}
'''
def main():
    ap=argparse.ArgumentParser();ap.add_argument("package_report",type=Path);args=ap.parse_args()
    original=pkg.read(args.package_report);folder=Path(original["package"]);archive=Path(original["archive"])
    out=pkg.PC/"build/publication-0340"/("windows-check-"+uuid.uuid4().hex[:12]);out.mkdir(parents=True)
    checks=[];rejected=[]
    def ok(v,label):pkg.need(v,label);checks.append(label)
    before={str(p):pkg.sha(p) for p in [archive,*folder.rglob("*")] if p.is_file()}
    baseline=pkg.validate(folder);ok(baseline["fileCount"]>90,"Full strict stage validation")
    with zipfile.ZipFile(archive) as z:
        ok(z.testzip() is None,"ZIP CRCs")
        names=z.namelist();ok(len(set(n.casefold() for n in names))==len(names),"ZIP no duplicate/case-colliding members")
        for n in names:
            p=PurePosixPath(n);ok(not p.is_absolute() and ".." not in p.parts and "\\" not in n,"ZIP safe path "+n)
        moved=out/"Déplacé é 🟢";moved.mkdir();z.extractall(moved)
    relocated=moved/folder.name
    ok(pkg.validate(relocated)==baseline,"Unicode relocation byte-equivalent")
    for flag,expected in (("--verify-layout",0),("--verify",3)):
        p=subprocess.run([relocated/"DinoTale.exe",flag],cwd=out,capture_output=True,timeout=10)
        ok(p.returncode==expected,"Real launcher passive "+flag)
    fake=pkg.compile_native(FAKE,out/"fake-child","FakeChild",{})
    fake_root=out/"Route contrôlée é 🟢";shutil.copytree(folder,fake_root)
    for p in [fake_root/"pc/build/native-special/DinoTale.exe",fake_root/pkg.WORKSHOP_DIR/"YoshiStory-AtelierPC.exe"]:shutil.copy2(fake,p)
    child_log=out/"fake-child.txt"
    env=os.environ.copy();env["DINO_FAKE_LOG0340"]=str(child_log)
    def launch(arguments,expected,cwd):
        if child_log.exists():child_log.unlink()
        p=subprocess.run([fake_root/"DinoTale.exe",*arguments],cwd=out,env=env,capture_output=True,timeout=10)
        ok(p.returncode==0,"Fake route parent exit "+expected)
        deadline=time.monotonic()+8
        while not child_log.exists() and time.monotonic()<deadline:time.sleep(.02)
        ok(child_log.exists(),"Fake route child started "+expected)
        lines=child_log.read_text(encoding="utf-8").splitlines()
        ok(expected in lines[0],"Fake route command "+expected)
        ok(Path(lines[1])==cwd,"Fake route working directory "+expected)
    launch([],"--setup-rom --play-after-setup",fake_root/"pc")
    launch(["--editor"],"--editor",fake_root/"pc")
    (fake_root/"pc/profiles/content-personal/yoshisstory.n64.us.z64").write_bytes(b"EXPLICIT TEST PLACEHOLDER - NOT A ROM")
    (fake_root/"pc/build/native-special/workshop-content.bin").write_bytes(b"EXPLICIT TEST PLACEHOLDER - NOT CONTENT")
    launch([],"--workshop-main-menu",fake_root/"pc/profiles/content-personal")
    # Actual frozen-source Workshop importer, with no ShowDialog/game branches.
    source=pkg.ROOT/"work/yoshis-story-decomp/baseroms/us/baserom.z64"
    ok(pkg.sha(source)==pkg.ROM_SHA,"Internal original ROM identity")
    raw=source.read_bytes();source_sha=pkg.sha(source)
    transformed={}
    v=bytearray(len(raw));v[::2]=raw[1::2];v[1::2]=raw[::2];transformed["v64"]=v
    n=bytearray(len(raw))
    for i in range(4):n[i::4]=raw[3-i::4]
    transformed["n64"]=n;transformed["z64"]=raw
    importer=relocated/pkg.WORKSHOP_DIR/"YoshiStory-AtelierPC.exe";import_sha=pkg.sha(importer)
    import_env=os.environ.copy()
    import_env.update(DOTNET_ROOT=str(out/"nonexistent-global-dotnet"),DOTNET_ROOT_X64=str(out/"nonexistent-global-dotnet"),DOTNET_MULTILEVEL_LOOKUP="0",DOTNET_BUNDLE_EXTRACT_BASE_DIR=str(out/"dotnet-bundle"))
    import_records=[]
    def invoke(root,rom,expect,label):
        p=subprocess.run([importer,root,"--startup-check","--import-rom-file",rom],cwd=root,env=import_env,capture_output=True,text=True,encoding="utf-8",errors="replace",timeout=90)
        (out/(label+".log")).write_text(p.stdout+p.stderr,encoding="utf-8")
        ok(p.returncode==expect,"Importer "+label+" expected exit "+str(expect));return p
    for order,data in transformed.items():
        fixture=out/("Import "+order+" é")/"pc";fixture.mkdir(parents=True)
        for name in ("pc/distribution/content.bps","pc/distribution/content-patch.json","pc/build/native-special/DinoTale.exe","pc/profiles/content-personal/saves/yoshisstory.n64.us.bin"):
            target=fixture/Path(name).relative_to("pc");target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(relocated/name,target)
        inp=fixture.parent/("original."+order);inp.write_bytes(data);inp_sha=pkg.sha(inp)
        seed=fixture/"profiles/content-personal/saves/yoshisstory.n64.us.bin"
        invoke(fixture,inp,0,order+"-import");invoke(fixture,inp,0,order+"-idempotent")
        ok(pkg.sha(seed)==pkg.FRESH_SHA,"Importer preserves fresh EEPROM "+order)
        ok(pkg.sha(inp)==inp_sha,"Importer input unchanged "+order)
        ok(pkg.sha(fixture/"profiles/content-personal/yoshisstory.n64.us.z64")==pkg.ROM_SHA,"Normalized ROM "+order)
        ok(pkg.sha(fixture/"build/native-special/workshop-content.bin")==pkg.CONTENT_SHA,"Actual BPS reconstruction "+order)
        import_records.append({"order":order,"inputSha256":inp_sha,"normalizedSha256":pkg.ROM_SHA,"contentSha256":pkg.CONTENT_SHA,"savePreserved":True,"idempotent":True})
    fixture=out/"Import z64 é"/"pc";inp=fixture.parent/"original.z64"
    manifest=fixture/"distribution/content-patch.json";saved=manifest.read_bytes();j=pkg.read(manifest);j["engineSha256"]="0"*64;pkg.write(manifest,j)
    invoke(fixture,inp,1,"reject-engine-binding");manifest.write_bytes(saved)
    ok(pkg.sha(fixture/"profiles/content-personal/saves/yoshisstory.n64.us.bin")==pkg.FRESH_SHA,"Rejected import preserves EEPROM")
    # Privacy/metadata rejection mutations only in a disposable extracted copy.
    mutate=out/"reject-fixture";shutil.copytree(folder,mutate)
    def rejected_edit(relative,content,label):
        p=mutate/relative;existed=p.exists();old=p.read_bytes() if existed else None;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(content)
        inventory=mutate/"contenu-sha256.json";original_inventory=inventory.read_bytes()
        pkg.write(inventory,{q.relative_to(mutate).as_posix():pkg.sha(q) for q in sorted(mutate.rglob("*")) if q.is_file() and q.name!="contenu-sha256.json"})
        try:
            try:pkg.validate(mutate)
            except (ValueError,KeyError,FileNotFoundError):rejected.append(label)
            else:raise AssertionError("Accepted invalid fixture: "+label)
        finally:
            if existed:p.write_bytes(old)
            else:p.unlink()
            inventory.write_bytes(original_inventory)
    rejected_edit("pc/build/native-special/assets/hidden.png",bytes.fromhex("80371240")+b"private","ROM magic")
    rejected_edit("pc/profiles/content-personal/pc-collection.json",b'{"currency":99}',"Private currency journal")
    rejected_edit("pc/profiles/content-personal/saves/yoshisstory.n64.us.bin",b"changed","Nonempty EEPROM")
    rejected_edit("pc/build/native-special/MusicRender0310.exe",b"changed","Changed worker")
    rejected_edit("pc/profiles/content-personal/sound.json",b'{"main_volume":10}',"Wrong volume")
    rejected_edit("pc/build/native-special/workshop-content.bin",b"private","Packaged game content")
    rejected_edit("pc/profiles/content-personal/controls.json",b"private","Personal controls")
    ok(pkg.sha(source)==source_sha,"Source ROM unchanged after all CPU imports")
    ok(pkg.sha(importer)==import_sha,"Self-contained importer unchanged")
    ok(all(pkg.sha(Path(p))==h for p,h in before.items()),"Candidate ZIP/stage unchanged")
    protected=pkg.read(Path(original["report"])/"protected-before.json")
    ok(all(pkg.sha(Path(p))==h for p,h in protected.items()),"Frozen engine/Atelier0311 unchanged")
    result={"passed":True,"checks":len(checks),"rejections":len(rejected),"checkLabels":checks,"rejectedFixtures":rejected,"package":str(folder),"archive":str(archive),"archiveSha256":pkg.sha(archive),"archiveBytes":archive.stat().st_size,"fileCount":baseline["fileCount"],"relocatedExtraction":str(relocated),"nativeEngineSha256":pkg.ENGINE_SHA,"sourceVersion":"0.33.0","distributionVersion":"0.34.0-beta.1","nativeEngineRebuilt":False,"gameOrGpuStarted":False,"fakeLaunchTargetsExplicit":True,"actualImporterExecuted":True,"importerSelfContainedWithGlobalRuntimeDisabled":True,"imports":import_records,"romIncludedInArchive":False,"freshSave":True,"personalDataAccessed":False,"volumePercent":3,"pendingManualTests":pkg.read(folder/"pc/portable-package.json")["pendingManualTests"],"sourceInputs":pkg.read(Path(original["report"])/"sources.json"),"verifierSha256":pkg.sha(Path(__file__)),"packageScriptSha256":pkg.sha(Path(__file__).with_name("package-windows0340.py"))}
    pkg.write(out/"verification.json",result);pkg.write(args.package_report.parent/"windows-verification.json",result)
    print(json.dumps({k:v for k,v in result.items() if k not in ("checkLabels","sourceInputs")},ensure_ascii=False,indent=2));print("REPORT",out/"verification.json")
if __name__=="__main__":main()

