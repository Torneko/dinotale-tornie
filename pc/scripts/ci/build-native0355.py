"""Fresh Ubuntu22.04/Clang15 native build using verified private C/RSP, never a ROM."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,re,shutil,subprocess,sys
R=Path(__file__).resolve().parents[3];PC=R/'pc'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def canonical(value):return json.dumps(value,sort_keys=True,separators=(',',':')).encode()
def normalized(p):return hashlib.sha256(p.read_text(encoding='utf-8').encode()).hexdigest()
def load(p):return json.loads(p.read_text(encoding='utf-8'))
def write(p,value):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8')
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--private-inputs',type=Path,required=True);p.add_argument('--source-commit',required=True);p.add_argument('--capabilities-template',type=Path,default=PC/'release/engine-capabilities0355.json');a=p.parse_args()
assert sys.platform=='linux' and Path('/etc/os-release').read_text().find('VERSION_ID="22.04"')>=0
assert os.environ.get('GITHUB_ACTIONS')=='true','Dedicated GitHub Actions runner only'
os.sched_setaffinity(0,set(sorted(os.sched_getaffinity(0))[:2]));assert len(os.sched_getaffinity(0))<=2
assert subprocess.check_output(['git','rev-parse','HEAD'],cwd=R,text=True).strip()==a.source_commit
assert not subprocess.check_output(['git','diff','--name-only','--ignore-submodules=all'],cwd=R,text=True).strip(),'Tracked source tree must be frozen'
assert not (PC/'dinotale/yoshisstory.us.z64').exists(),'No ROM allowed'
def submodule_commits():
 lines=subprocess.check_output(['git','submodule','status','--recursive'],cwd=R,text=True).splitlines()
 assert lines and all(line.startswith(' ') for line in lines),'Recursive submodule HEAD must match recorded gitlink pins'
 return {line[1:].split()[1]:line[1:].split()[0] for line in lines}
submodule_before=submodule_commits()
private=a.private_inputs.resolve();assert private.is_relative_to(PC/'build/linux-mint-0355')
manifest_path=private/'manifest.json';manifest=load(manifest_path);transport=load(private/'transport-receipt.json')
assert manifest['version']=='0.35.5' and manifest['sourceCommit']==a.source_commit and manifest['generatedCount']==219
assert sha(manifest_path)==transport['manifestSha256'] and transport['sourceCommit']==a.source_commit
assert all(sha(private/n)==digest for n,digest in manifest['files'].items())
assert manifest['originalRomIncluded'] is False and manifest['preparedContentIncluded'] is False and manifest['saveIncluded'] is False
frozen=load(PC/'recompile/generated-0355-sha256.json');assert frozen==manifest['generatedNormalizedSha256']
assert hashlib.sha256(canonical(frozen)).hexdigest()==manifest['generatedManifestSha256']
assert all(normalized(private/'RecompiledFuncs'/n)==digest for n,digest in frozen.items())
assert normalized(private/'rsp/aspMain.cpp')==manifest['rspNormalizedSha256']
assert manifest['contentSha256']=='d5800ba53ceceb261048d72289805ed531e0f708916546febf7a41c22cdee87d' and manifest['contentFnv64']=='5E83770716DC3E31'
O=PC/'build/linux-mint-0355';O.mkdir(parents=True,exist_ok=True);B=PC/'build/linux-native0355-ci';T=PC/'build/native-linux-0355'
assert not B.exists() and not T.exists(),'Fresh native build/output required'
# Snapshot only the public tracked inputs and the separately verified private inputs.
tracked=subprocess.check_output(['git','ls-files','-z'],cwd=R).decode().split('\0');before={n:sha(R/n) for n in tracked if n and (R/n).is_file()}
private_before={n:sha(private/n) for n in manifest['files']}
log_names=[]
def command(command,name,timeout=7200):
 log=O/(name+'.private.log');log_names.append(log)
 with log.open('wb') as stream:result=subprocess.run([str(v) for v in command],cwd=R,stdout=stream,stderr=subprocess.STDOUT,timeout=timeout)
 assert result.returncode==0,name+' failed; diagnostic log stays private on runner'
 return log
rsp=PC/'dinotale/rsp/aspMain.cpp';assert not rsp.exists(),'Do not overwrite a source RSP';rsp.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(private/'rsp/aspMain.cpp',rsp)
placeholder=private/'placeholder';placeholder.mkdir();content=placeholder/'workshop-content.bin';content.write_bytes(b'\0')
header='#pragma once\n#include <cstdint>\ninline constexpr uint64_t workshop_content_hash=0x5E83770716DC3E31ULL;\n';(placeholder/'content_config.hpp').write_text(header)
report={'passed':False,'version':'0.35.5','sourceCommit':a.source_commit,'freshNativeBuild':True,'originalRomUsed':False,'preparedContentUsed':False,'gameGpuOrAudioDeviceUsed':False,'physicalGameplayVerified':False,'maximumBuildJobs':2,'cpuAffinity':sorted(os.sched_getaffinity(0))}
try:
 command(['cmake','-S',PC/'dinotale','-B',B,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_C_COMPILER=clang-15','-DCMAKE_CXX_COMPILER=clang++-15','-DWORKSHOP_DIR='+str(PC),'-DDINO_RECOMP_DIR='+str(private/'RecompiledFuncs'),'-DWORKSHOP_CONTENT_ROM='+str(content),'-DWORKSHOP_RUNTIME_OUTPUT='+str(T),'-DWORKSHOP_SPECIAL_CONTENT=ON','-DWORKSHOP_HYBRID_FINDS=ON','-DWORKSHOP_NATIVE_COOP=ON','-DCMAKE_BUILD_RPATH_USE_ORIGIN=ON','-DSDL2_DIR='+str(PC/'tools/sdl2-linux/lib/cmake/SDL2'),'-DCMAKE_PROJECT_DinoTale_INCLUDE='+str(PC/'scripts/ci/preserved-rsp0355.cmake'),'-DDT_CI_RSP_SOURCE='+str(rsp)],'native-configure')
 ninja=(B/'build.ninja').read_text();assert not re.search(r'RSPRecomp[^\n]*aspMain[.]us[.]toml|COMMAND[^\n]*aspMain[.]us[.]toml',ninja),'No RSP generation permitted'
 compilation=load(B/'compile_commands.json');native_rsp=[row for row in compilation if Path(row['file'])==rsp and 'DinoTale.dir' in row.get('command','')]
 assert len(native_rsp)==1 and '-DDT_HAS_RSP_AUDIO' in native_rsp[0]['command'],'Native preserved RSP audio not compiled'
 generated_rows=[row for row in compilation if Path(row['file']).parent==private/'RecompiledFuncs'];assert len(generated_rows)==217,'All216 C bodies and lookup.cpp must compile'
 assert all('-std=gnu17' in row['command'] or '-std=c17' in row['command'] for row in generated_rows if row['file'].endswith('.c')),'Generated bodies must compile as C17'
 patch_series={}
 for name in ('rt64','N64ModernRuntime','RecompFrontend'):
  patch_files=sorted((PC/'dinotale/lib-patches'/name).glob('*.patch'))
  expected=hashlib.sha256(''.join(sha(f)+'\n' for f in patch_files).encode()).hexdigest()
  gitdir=Path(subprocess.check_output(['git','rev-parse','--absolute-git-dir'],cwd=PC/'dinotale/lib'/name,text=True).strip())
  assert (gitdir/'dinotale-patches').read_text().strip()==expected,'Applied library patch series mismatch'
  patch_series[name]=expected
 command(['cmake','--build',B,'--target','DinoTale','--parallel','2'],'native-build')
 assert sha(rsp)==manifest['rspSha256'],'Preserved RSP was regenerated'
 assert submodule_commits()==submodule_before,'Submodule gitlink HEAD changed during build'
 game=T/'DinoTale';worker=T/'MusicRender0310';assert game.read_bytes()[:4]==worker.read_bytes()[:4]==b'\x7fELF'
 assert (T/'workshop-content.bin').read_bytes()==b'\0';(T/'workshop-content.bin').unlink();content.unlink()
 worker_identity=subprocess.check_output([str(worker),'--identity'],cwd=T,text=True,timeout=10).strip()
 identity=load(B/'menu-music-native0310/renderer-identity.json');prior=load(PC/'release/worker-reference0355.json')
 assert worker_identity==identity['sha256']==prior['rendererIdentity']
 assert hashlib.sha256(canonical(identity['inputs'])).hexdigest()==worker_identity
 assert identity['inputs']==prior['sourceInputs'],'Current worker compiled-source identity differs from historical audio qualification'
 source_map={n:(B/'menu-music-native0310/audio_closure.c' if n=='closure' else R/n) for n in identity['inputs']}
 assert all(normalized(source_map[n])==digest for n,digest in identity['inputs'].items())
 assert all(sha(R/n)==digest for n,digest in before.items()) and all(sha(private/n)==digest for n,digest in private_before.items()),'Build inputs changed'
 metadata=load(a.capabilities_template);assert metadata['version']=='0.35.5'
 for key in ('sourceBuildReport','builtAtUtc','exeSha256','engineSha256','workerSha256','menuMusicWorkerSha256'):metadata.pop(key,None)
 metadata.update(platform='Linux x86-64',version='0.35.5',exeSha256=sha(game),engineSha256=sha(game),workerSha256=sha(worker),menuMusicWorkerSha256=sha(worker),menuMusicWorker='MusicRender0310',releaseReady=False,manualUserAccepted=False,personalProfileInstalled=False,originalRomRequired=True,standaloneMusicCpuVerified=False,standaloneMusicSourceIdentityVerified=True,freshMusicRomRenderVerified=False,qualification='Fresh Ubuntu22.04 Clang15 C17/C++ native build, source-identity-only music verification; original ROM and prepared content absent. CPU persistence qualification is reported separately. Human game/GPU/audio tests remain pending.',validationScope='Build and worker identity only; no fresh music render, gameplay, GPU, sound playback or private save accessed.',builtAtUtc=datetime.datetime.now(datetime.timezone.utc).isoformat())
 write(T/'engine-build.json',metadata)
 # Sanitized proof may be exported; no raw build log or generated source may be exported.
 write(O/'private-input-manifest.json',manifest)
 report.update(passed=True,inputsUnchanged=True,engineSha256=sha(game),workerSha256=sha(worker),engineBuildMetadataSha256=sha(T/'engine-build.json'),publicSourceManifestSha256=hashlib.sha256(canonical(before)).hexdigest(),privateInputs={'manifestPath':'pc/build/linux-mint-0355/private-input-manifest.json','manifestSha256':sha(O/'private-input-manifest.json'),'bundlePlaintextSha256':transport['bundlePlaintextSha256'],'encryptedBundleSha256':transport['encryptedBundleSha256'],'generatedCount':219,'generatedManifestSha256':manifest['generatedManifestSha256'],'rspSha256':manifest['rspSha256']},contentCompiled={'expectedSha256':manifest['contentSha256'],'fnv64':manifest['contentFnv64'],'placeholderOnly':True,'realContentUploaded':False,'placeholderRemovedFromRuntime':True},workerQualification={'identityExecuted':True,'actualRenderExecuted':False,'sourceIdentityVerified':True,'rendererIdentity':worker_identity,'priorAudioReference':{n:prior[n] for n in ('windows0355','linux0354','rendererIdentity','pcmSha256','wavSha256','loopBegin','loopEnd','nativeBodiesExecuted','rspTasks','romDmas','measuredNativeCalls')}},buildEnvironment={'runner':'GitHub Actions','os':'Ubuntu22.04','glibcMaximumAllowed':'2.35','compiler':subprocess.check_output(['clang++-15','--version'],text=True).splitlines()[0],'cLanguage':'C17','parallel':2,'sdlVersion':'2.30.3','sdlSourceSha256':'820440072f8f5b50188c1dae104f2ad25984de268785be40c41a099a510f0aec'},preservedRspCompiled=True,rspRegenerated=False,submoduleCommits=submodule_before,appliedPatchSeriesSha256=patch_series)
except Exception as error:
 report['error']=str(error);raise
finally:
 # Remove the one-byte placeholder even after compilation fails. Never upload it.
 if (T/'workshop-content.bin').exists():(T/'workshop-content.bin').unlink()
 if content.exists():content.unlink()
 write(O/'native-build-final.json',report)
print(json.dumps({n:report[n] for n in ('passed','version','engineSha256','workerSha256','engineBuildMetadataSha256')},indent=2))
