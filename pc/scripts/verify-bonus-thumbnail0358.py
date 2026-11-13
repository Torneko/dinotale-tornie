"""Native CPU check of installation-local Bonus PNG reading and actual menu image wiring.
Uses synthetic bounded PNG fixtures only; no ROM, screenshot, game, GPU or personal profile.
"""
from pathlib import Path
import argparse,hashlib,json,subprocess,sys,uuid,struct,zlib,datetime,shutil
ROOT=Path('D:/Yoshi-Story')
def sha(p):
 with p.open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def quote(v):return "'"+str(v).replace("'","''")+"'"
def need(v,m):
 if not v:raise RuntimeError(m)
def png(w,h,color,pixel,broken=False,extra=0,filter_type=0,wrong_adler=False):
 def chunk(kind,data):return struct.pack('>I',len(data))+kind+data+struct.pack('>I',zlib.crc32(kind+data))
 header=struct.pack('>IIBBBBB',w,h,8,color,0,0,0)
 rows=(bytes([filter_type])+bytes(pixel)*w)*h+b'x'*extra
 compressed=b'\x78\x9cbroken'if broken else zlib.compress(rows)
 if wrong_adler:compressed=compressed[:-1]+bytes([compressed[-1]^1])
 return b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',header)+chunk(b'IDAT',compressed)+chunk(b'IEND',b'')
def body(text,prefix):
 start=text.index(prefix);p=text.index('{',start);depth=1;q=p+1
 while depth:depth+=(text[q]=='{')-(text[q]=='}');q+=1
 return text[start:q]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--dev-ready',action='store_true');a=p.parse_args()
 if not a.dev_ready:
  vs=subprocess.check_output(['C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe','-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip();need(vs,'VisualStudio C++ tools missing')
  cmd="$ErrorActionPreference='Stop'; & "+quote(Path(vs)/'Common7/Tools/Launch-VsDevShell.ps1')+" -Arch amd64 -HostArch amd64 -SkipAutomaticLocation | Out-Null; & "+quote(sys.executable)+" -X utf8 -B "+quote(Path(__file__).resolve())+" --dev-ready; exit $LASTEXITCODE"
  return subprocess.run([shutil.which('pwsh.exe')or'powershell.exe','-NoProfile','-NonInteractive','-Command',cmd],cwd=ROOT).returncode
 out=ROOT/('pc/build/bonus-thumbnail0358/native-'+uuid.uuid4().hex[:12]);out.mkdir(parents=True,exist_ok=False)
 inputs=[Path(__file__),ROOT/'pc/engine/bonus_thumbnail0358.hpp',ROOT/'pc/engine/workshop_menu.cpp',ROOT/'pc/engine/tests/BonusThumbnail0358Checks.cpp',ROOT/'pc/dinotale/lib/rt64/src/contrib/stb/stb_image.h'];frozen={str(v):sha(v)for v in inputs}
 text=inputs[2].read_text(encoding='utf-8-sig');actual='\n'.join(body(text,p)for p in('void hide_bonus_thumbnail0358(){','bool bounded_bonus_png0358(','workshop::bonus_thumbnail0358::View read_bonus_thumbnail0358('));(out/'menu.inc').write_text(actual,encoding='utf-8')
 fixtures=out/'fixtures';fixtures.mkdir();(fixtures/'rgba320.png').write_bytes(png(320,180,6,[7,31,53,255]));(fixtures/'rgb640.png').write_bytes(png(640,360,2,[19,43,67]));(fixtures/'changed320.png').write_bytes(png(320,180,6,[91,113,157,255]));(fixtures/'bad-zlib.png').write_bytes(png(320,180,6,[7,31,53,255],True));(fixtures/'extra-expanded.png').write_bytes(png(320,180,6,[7,31,53,255],extra=1));(fixtures/'expanded-bomb.png').write_bytes(png(320,180,6,[7,31,53,255],extra=10*1024*1024));(fixtures/'filter-invalid.png').write_bytes(png(320,180,6,[7,31,53,255],filter_type=5));(fixtures/'adler-invalid.png').write_bytes(png(320,180,6,[7,31,53,255],wrong_adler=True))
 outside=out/'outside';outside.mkdir();(outside/'7-1.png').write_bytes((fixtures/'rgba320.png').read_bytes());link_parent=out/'junction-pc/content';link_parent.mkdir(parents=True)
 command="$ErrorActionPreference='Stop'; New-Item -ItemType Junction -Path "+quote(link_parent/'bonus-thumbnails0358')+" -Target "+quote(outside)+" | Out-Null"
 result=subprocess.run(['powershell.exe','-NoProfile','-NonInteractive','-Command',command],capture_output=True,text=True,creationflags=subprocess.CREATE_NO_WINDOW);need(result.returncode==0,'Could not prepare isolated junction test: '+result.stderr)
 unit='#define BONUS_THUMBNAIL_ACTUAL_BODY "'+str(out/'menu.inc').replace('\\','/')+'"\n#include "'+str(inputs[3]).replace('\\','/')+'"\n';(out/'unit.cpp').write_text(unit,encoding='utf-8')
 exe=out/'checks.exe';cmd=[str(ROOT/'pc/tools/llvm-20.1.8/bin/clang-cl.exe'),'/nologo','/std:c++20','/O2','/W4','/WX','/EHsc','/I'+str(ROOT/'pc/dinotale/lib/rt64/src/contrib'),str(out/'unit.cpp'),'/Fo'+str(out/'checks.obj'),'/Fe'+str(exe)]
 build=subprocess.run(cmd,cwd=out,capture_output=True,text=True);(out/'compile.log').write_text(build.stdout+build.stderr,encoding='utf-8');need(build.returncode==0,'Native CPU test did not compile: '+str(out/'compile.log'))
 legacy=actual.replace('workshop::bonus_thumbnail0358::u32(png,p)','u32(png,p)').replace('workshop::bonus_thumbnail0358::u32(compressed,compressed.size()-4)','u32(compressed,compressed.size()-4)');need(legacy!=actual,'Legacy u32 control did not change the actual Menu body');(out/'menu-unqualified-control.inc').write_text(legacy,encoding='utf-8')
 control=unit.replace(str(out/'menu.inc').replace('\\','/'),str(out/'menu-unqualified-control.inc').replace('\\','/'));(out/'unqualified-control.cpp').write_text(control,encoding='utf-8')
 control_cmd=[value.replace(str(out/'unit.cpp'),str(out/'unqualified-control.cpp')).replace(str(out/'checks.obj'),str(out/'unqualified-control.obj')).replace(str(exe),str(out/'unqualified-control.exe'))for value in cmd]
 refused=subprocess.run(control_cmd,cwd=out,capture_output=True,text=True);control_log=refused.stdout+refused.stderr;(out/'unqualified-control.log').write_text(control_log,encoding='utf-8');need(refused.returncode!=0 and control_log.count("reference to 'u32' is ambiguous")>=2,'Global u32 alias control was not reproduced')
 run=subprocess.run([str(exe),str(out)],cwd=out,capture_output=True,text=True,timeout=45);(out/'stdout.log').write_text(run.stdout,encoding='utf-8');(out/'stderr.log').write_text(run.stderr,encoding='utf-8');need(run.returncode==0,'Native CPU test failed: '+run.stderr)
 result=json.loads(run.stdout);need(all(sha(Path(n))==h for n,h in frozen.items()),'Native inputs changed during verification')
 report={'passed':True,'result':result,'sourceSha256':frozen,'actualMenuBodiesSha256':hashlib.sha256(actual.encode()).hexdigest(),'globalU32AliasBoundaryCovered':True,'unqualifiedLegacyCompileRefused':True,'unqualifiedControlExitCode':refused.returncode,'inputsUnchanged':True,'readOnlyLibraryPath':'pc_root/content/bonus-thumbnails0358/7-{1..6}.png','cachePolicy':'Six immutable process-lifetime PNG slots; changed file hidden with restart requirement. Same fixed keys may rebind after menu-context recreation.','noFrontendFilesChanged':True,'screenshotOrGameplayAuthenticityClaimed':False,'gameGpuUiOrPersonalUsed':False,'createdUtc':datetime.datetime.now(datetime.timezone.utc).isoformat()}
 (out/'verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps({'passed':True,'report':str(out/'verification.json'),'result':result},indent=2));return 0
if __name__=='__main__':raise SystemExit(main())
