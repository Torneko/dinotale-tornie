"""Build0355 from locally verified prepared functions and locally prepared audio RSP."""
from pathlib import Path
from datetime import datetime,timezone
import argparse,hashlib,importlib.util,json,os,re,subprocess,sys,uuid
CONTENT_SHA='d5800ba53ceceb261048d72289805ed531e0f708916546febf7a41c22cdee87d'
ORIGINAL_SHA='e8a63388c38f8f0bea37e9b13a97f0898ac5496e08ca2028afb8db9a866e0ce9'
def need(ok,message):
 if not ok:raise ValueError(message)
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def read(path):return json.loads(Path(path).read_text(encoding='utf-8-sig'))
def write(path,value):path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8')
def checked_inputs(root):
 pc=root/'pc';generated=pc/'build/special-port';expected=read(pc/'recompile/generated-0355-sha256.json');folder=generated/'RecompiledFuncs';need(folder.is_dir(),'Run prepare-content0355.py first')
 actual={f.name:hashlib.sha256(f.read_bytes().replace(b'\r\n',b'\n')).hexdigest()for f in folder.iterdir()if f.is_file()};need(len(actual)==219 and actual==expected,'Prepared0355 functions differ from the published219-file manifest')
 content=generated/'content.z64';need(content.is_file() and content.stat().st_size==16777216 and sha(content)==CONTENT_SHA,'Prepared content differs')
 need(not (pc/'dinotale/yoshisstory.us.z64').exists(),'Move the original source-tree ROM aside before this prepared-source build; no file was changed')
 return generated,actual
def configure_command(pc,build,runtime,generated,platform,sdl2_dir=None):
 cc,cxx=('clang-cl','clang-cl')if platform=='windows'else('clang-15','clang++-15')
 command=['cmake','-S',str(pc/'dinotale'),'-B',str(build),'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_C_COMPILER='+cc,'-DCMAKE_CXX_COMPILER='+cxx,'-DWORKSHOP_DIR='+str(pc),'-DDINO_RECOMP_DIR='+str(generated/'RecompiledFuncs'),'-DWORKSHOP_CONTENT_ROM='+str(generated/'content.z64'),'-DWORKSHOP_RUNTIME_OUTPUT='+str(runtime),'-DWORKSHOP_SPECIAL_CONTENT=ON','-DWORKSHOP_HYBRID_FINDS=ON','-DWORKSHOP_NATIVE_COOP=ON','-DCMAKE_PROJECT_DinoTale_INCLUDE='+str(pc/'recompile/use-prepared-rsp0355.cmake'),'-DDINO_PREPARED_RSP_SOURCE='+str(pc/'dinotale/rsp/aspMain.cpp')]
 if platform=='linux':command.append('-DCMAKE_BUILD_RPATH_USE_ORIGIN=ON')
 if sdl2_dir:command.append('-DSDL2_DIR='+str(sdl2_dir.resolve()))
 return command
def bind_importer(pc,build,engine_sha):
 path=pc/'distribution/content-patch.json';need(path.is_file() and not path.is_symlink(),'A regular importer manifest is required')
 data=path.read_bytes();manifest=read(path);need(manifest.get('sourceSha256')==ORIGINAL_SHA and manifest.get('targetSha256')==CONTENT_SHA and manifest.get('patchSha256')==sha(pc/'distribution/content.bps'),'Importer BPS identity differs')
 backup=build/('content-patch.before-binding0355-'+uuid.uuid4().hex[:12]+'.json');backup.parent.mkdir(parents=True,exist_ok=True)
 with backup.open('xb')as stream:stream.write(data)
 manifest['engineSha256']=engine_sha;manifest['engineVersion']='0.35.5';temp=path.with_name('.content-patch0355-'+uuid.uuid4().hex+'.tmp')
 try:
  with temp.open('x',encoding='utf-8',newline='\n')as stream:stream.write(json.dumps(manifest,indent=2)+'\n')
  need(path.read_bytes()==data,'Importer manifest changed during binding; preserved');os.replace(temp,path)
 finally:
  if temp.exists():temp.unlink()
 return {'backup':str(backup),'beforeSha256':hashlib.sha256(data).hexdigest(),'afterSha256':sha(path),'engineSha256':engine_sha}
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--platform',choices=['windows','linux'],required=True);p.add_argument('--rsp-recompiler',type=Path);p.add_argument('--sdl2-dir',type=Path);p.add_argument('--jobs',type=int,default=2);p.add_argument('--bind-importer',action='store_true',help='Back up and bind the local importer manifest to this freshly built executable');a=p.parse_args()
 need(1<=a.jobs<=2,'Use one or two compiler jobs');need((a.platform=='windows' and os.name=='nt')or(a.platform=='linux' and sys.platform=='linux'),'Build on the selected native host')
 root=Path(__file__).resolve().parents[2];pc=root/'pc';generated,inputs=checked_inputs(root)
 pins=read(root/'upstream.json')['submodules']
 for name,expected in pins.items():
  folder=pc/'dinotale'/name;need((folder/'CMakeLists.txt').is_file(),'Initialize pinned recursive submodules first: '+name)
  actual=subprocess.check_output(['git','-C',str(folder),'rev-parse','HEAD'],text=True).strip();need(actual==expected,'Wrong pinned dependency '+name)
 spec=importlib.util.spec_from_file_location('prepare_audio0355',pc/'scripts/prepare-audio0355.py');audio=importlib.util.module_from_spec(spec);spec.loader.exec_module(audio);rsp=audio.prepare(root,generated/'content.z64',a.rsp_recompiler)
 build=pc/('build/native-windows0355'if a.platform=='windows'else'build/native-linux0355');runtime=pc/'build/native-special'
 if a.platform=='linux':
  dxc=pc/'dinotale/lib/rt64/src/contrib/dxc/bin/x64/dxc-linux';need(dxc.is_file(),'Pinned Linux DXC missing; initialize recursive submodules');dxc.chmod(dxc.stat().st_mode|0o111)
 command=configure_command(pc,build,runtime,generated,a.platform,a.sdl2_dir);subprocess.run(command,cwd=root,check=True);subprocess.run(['cmake','--build',str(build),'--target','DinoTale','--parallel',str(a.jobs)],cwd=root,check=True)
 suffix='.exe'if a.platform=='windows'else'';game=runtime/('DinoTale'+suffix);worker=runtime/('MusicRender0310'+suffix);need(game.is_file() and worker.is_file(),'Expected native outputs missing');magic=b'MZ'if a.platform=='windows'else b'\x7fELF';need(game.read_bytes().startswith(magic) and worker.read_bytes().startswith(magic),'Wrong executable format')
 _,after=checked_inputs(root);need(after==inputs and sha(pc/'dinotale/rsp/aspMain.cpp')==rsp['rspSha256'],'Prepared build input changed')
 identity=subprocess.check_output([str(worker),'--identity'],cwd=runtime,text=True,timeout=15).strip();need(re.fullmatch(r'[0-9a-f]{64}',identity),'Worker identity check failed')
 caps=read(pc/'release/engine-capabilities0355.json');caps.update(version='0.35.5',platform='Windows x64'if a.platform=='windows'else'Linux x86-64',engineSha256=sha(game),exeSha256=sha(game),workerSha256=sha(worker),menuMusicWorkerSha256=sha(worker),menuMusicWorker=worker.name,releaseReady=False,manualUserAccepted=False,personalProfileInstalled=False,nativeBuildVerified=True,standaloneMusicCpuVerified=False,freshMusicRomRenderVerified=False,rendererIdentity=identity,qualification='Local0355 source build; executable format and worker identity checked. No gameplay, GPU or audio-render qualification.',validationScope='Compilation and worker identity only; natural gameplay and ROM-backed audio render still require qualification.')
 write(runtime/'engine-build.json',caps);binding=bind_importer(pc,build,sha(game))if a.bind_importer else None
 report={'passed':True,'version':'0.35.5','platform':a.platform,'compilerJobs':a.jobs,'engineSha256':sha(game),'workerSha256':sha(worker),'rendererIdentity':identity,'preparedFunctions':219,'rsp':rsp,'importerBinding':binding,'gameGpuOrPhysicalAudioLaunched':False,'freshRomAudioRenderVerified':False,'createdUtc':datetime.now(timezone.utc).isoformat()};write(build/'source-build0355.json',report);print(json.dumps(report,indent=2))
 if not a.bind_importer:print('Importer manifest was preserved. Rebind its engineSha256 before using the rebuilt game through the Workshop, or rerun this command with --bind-importer (a backup will be retained).')
if __name__=='__main__':main()
