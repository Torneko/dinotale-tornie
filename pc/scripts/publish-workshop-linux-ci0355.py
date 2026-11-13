"""Publish actual0355 Linux Workshop on a Linux CI host; no game or GUI."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,re,shutil,struct,subprocess,sys,uuid
R=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--publish-go',action='store_true');p.add_argument('--verify-existing-build',type=Path);args=p.parse_args()
if not args.publish_go:p.error('Explicit publication GO is required')
assert sys.platform=='linux','This CI publisher runs only on Linux'
OUT=R/'pc/build/workshop-linux0355';assert not OUT.exists(),'Preserve existing candidate'
if args.verify_existing_build:
 O=args.verify_existing_build.resolve();assert O.parent==(R/'pc/build/linux-mint-0355').resolve() and O.name.startswith('workshop-') and O.is_dir(),'Existing build must be an owned Linux0355 workshop artifact'
 prior=json.loads((O/'verification.json').read_text(encoding='utf-8'));assert prior.get('error')=='Unexpected published sidecars' and prior.get('sourceHashes') and prior.get('command'),'Only resume the completed publish rejected for sidecars'
 assert (O/'publish/YoshiStory-AtelierLinux').is_file() and (O/'publish.log').is_file()
else:
 O=R/('pc/build/linux-mint-0355/workshop-'+uuid.uuid4().hex[:12]);O.mkdir(parents=True)
S=O/'sources';sha=lambda x:hashlib.sha256(Path(x).read_bytes()).hexdigest();env=os.environ.copy();env.update(DOTNET_PROCESSOR_COUNT='1',DOTNET_CLI_TELEMETRY_OPTOUT='1',DOTNET_SKIP_FIRST_TIME_EXPERIENCE='1',DOTNET_EnableDiagnostics='0')
report={'passed':False,'version':'0.35.5','crossPublishedOnWindows':False,'wslGameGpuBrowserOrPhysicalSoundUsed':False,'nativeLinuxBuildHost':True,'personalOrPublicInstalled':False,'maximumCpuJobs':1,'sourceHashes':{},'publisherScriptSha256':sha(Path(__file__).resolve())}
try:
 source_hashes={}
 for folder in ['Core','WorkshopLinux']:
  base=R/'pc/workshop'/folder
  for directory,folders,names in os.walk(base):
   folders[:]=[n for n in folders if n not in {'obj','bin','build'}]
   for name in names:
    f=Path(directory)/name
    if f.suffix not in {'.cs','.csproj','.html'}:continue
    d=S/folder/f.relative_to(base)
    if args.verify_existing_build:assert d.is_file() and sha(f)==sha(d),'Published source snapshot changed'
    else:d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(f,d)
    source_hashes[f.relative_to(R).as_posix()]=sha(d)
 report['sourceHashes']=source_hashes
 sdk_selection={'sdk':{'version':'9.0.300','rollForward':'disable','allowPrerelease':False}};sdk_file=S/'global.json'
 if args.verify_existing_build:assert json.loads(sdk_file.read_text(encoding='utf-8'))==sdk_selection,'SDK selection snapshot changed'
 else:sdk_file.write_text(json.dumps(sdk_selection,indent=2)+'\n',encoding='utf-8')
 report['sdkSelection']=sdk_selection
 capabilities=json.loads((R/'pc/release/engine-capabilities0355.json').read_text(encoding='utf-8'));assert capabilities['version']=='0.35.5' and capabilities['soloYoshiColors']==18 and capabilities['editorSpecialEggCount']==12 and capabilities['editorElementalPlacements']is True
 html=(S/'WorkshopLinux/index.html').read_text(encoding='utf-8');program=(S/'WorkshopLinux/Program.cs').read_text(encoding='utf-8');assert 'id<=0x4f3b' in html and "'#dfb94b','#bdc8d6','#252836','#fafaf3']" in html and '·12 œufs' in html;assert 'editor0340=true,editor0355=true' in program and 'knownYoshiMask18=KnownYoshis0355.ReadProfile' in program
 frozen={str(f):sha(f) for folder in ['workshop-linux0340','workshop-linux0354'] for f in ((R/'pc/build'/folder).iterdir() if (R/'pc/build'/folder).is_dir() else ()) if f.is_file()};report['frozenLinuxAtelierBefore']=frozen
 command=['dotnet','publish',str(S/'WorkshopLinux/WorkshopLinux.csproj'),'-c','Release','-r','linux-x64','--self-contained','true','-p:RuntimeFrameworkVersion=9.0.5','-p:Version=0.35.5','-p:AssemblyVersion=0.35.5.0','-p:FileVersion=0.35.5.0','-p:PublishSingleFile=true','-p:IncludeNativeLibrariesForSelfExtract=true','-p:PublishTrimmed=false','-p:UseAppHost=true','-p:DebugType=None','-p:DebugSymbols=false','-p:UseSharedCompilation=false','-p:RestoreIgnoreFailedSources=true','-maxcpucount:1','--disable-build-servers','--artifacts-path',str(O/'artifacts'),'-o',str(O/'publish')]
 report['command']=command;report['sdkVersion']=subprocess.check_output(['dotnet','--version'],cwd=S,env=env,text=True,timeout=30).strip();assert report['sdkVersion']=='9.0.300','Expected pinned .NET SDK9.0.300';print('PUBLISH '+str(O/'publish.log'),flush=True)
 if args.verify_existing_build:
  assert prior['command']==command and prior['sourceHashes']==source_hashes,'Prior publish inputs differ';report['revalidatedCompletedBuild']=True
  shutil.copy2(O/'verification.json',O/'verification-before-empty-sidecar-fix.json')
 else:
  with (O/'publish.log').open('w',encoding='utf-8')as log:cp=subprocess.run(command,cwd=S,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=900)
  assert cp.returncode==0,'Publish failed; see publish.log'
 exe=O/'publish/YoshiStory-AtelierLinux';assert exe.is_file()
 sidecar=O/'publish/YoshiStory-AtelierLinux.staticwebassets.endpoints.json';published={f.name for f in (O/'publish').iterdir()}
 assert published in ({exe.name},{exe.name,sidecar.name}),'Unexpected published sidecars'
 if sidecar.exists():
  assert json.loads(sidecar.read_text(encoding='utf-8'))=={'Version':1,'ManifestType':'Publish','Endpoints':[]},'Unexpected static web assets'
  assert 'MapStaticAssets' not in program and 'MapGet("/"' in program and 'GetManifestResourceStream' in program
 report['unusedEmptyStaticWebAssetsManifestExcluded']=sidecar.exists()
 data=exe.read_bytes();assert data[:7]==bytes([127,69,76,70,2,1,1]);assert struct.unpack_from('<H',data,18)[0]==62;assert struct.unpack_from('<H',data,16)[0]in(2,3)
 phoff=struct.unpack_from('<Q',data,32)[0];phsize,phcount=struct.unpack_from('<HH',data,54);interpreter=None
 for n in range(phcount):
  at=phoff+n*phsize;kind=struct.unpack_from('<I',data,at)[0]
  if kind==3:
   off=struct.unpack_from('<Q',data,at+8)[0];length=struct.unpack_from('<Q',data,at+32)[0];interpreter=data[off:off+length].rstrip(bytes([0])).decode()
 assert interpreter=='/lib64/ld-linux-x86-64.so.2'
 marker=bytes.fromhex('8b1202b96a612038727b930214d7a03213f5b9e6efae3318ee3b2dce24b36aae');where=data.find(marker);assert where>=8 and data.count(marker)==1;header=struct.unpack_from('<Q',data,where-8)[0];major,minor,count=struct.unpack_from('<III',data,header);assert (major,minor)==(6,0)and 100<count<1000;cursor=header+12
 def string():
  global cursor
  length=shift=0
  while True:
   value=data[cursor];cursor+=1;length|=(value&127)<<shift;shift+=7;assert shift<=35
   if value<128:break
  assert length<4096;value=data[cursor:cursor+length].decode();cursor+=length;return value
 bundle_id=string();deps_at,deps_size,config_at,config_size,flags=struct.unpack_from('<QQQQQ',data,cursor);cursor+=40;entries=[]
 for _ in range(count):
  off,size,compressed=struct.unpack_from('<QQQ',data,cursor);cursor+=24;kind=data[cursor];cursor+=1;name=string();assert off+(compressed or size)<=header;entries.append({'name':name,'kind':kind,'bytes':size})
 assert cursor==len(data);names={x['name']for x in entries};assert len(names)==count and {'Core.dll','YoshiStory-AtelierLinux.dll','System.Private.CoreLib.dll'}<=names
 configs=list((O/'artifacts/bin/WorkshopLinux').rglob('YoshiStory-AtelierLinux.runtimeconfig.json'));assert len(configs)==1;config=json.loads(configs[0].read_text(encoding='utf-8'));assert json.loads(data[config_at:config_at+config_size])==config
 frameworks=config['runtimeOptions']['includedFrameworks'];assert {x['name']for x in frameworks}=={'Microsoft.NETCore.App','Microsoft.AspNetCore.App'} and all(x['version']=='9.0.5'for x in frameworks);assert config['runtimeOptions']['configProperties']['System.Globalization.Invariant']is True
 hosts=[f for f in (O/'artifacts/obj/WorkshopLinux').rglob('singlefilehost*')if f.is_file()and f.suffix not in {'.pdb','.dbg'}];assert len(hosts)==1;host=hosts[0].read_bytes();assert data[:where-8]==host[:where-8]and data[where:len(host)]==host[where:],'Native host patched beyond manifest pointer'
 assert len(data)>80_000_000 and b'knownYoshiMask18'in data and 'editorElementalPlacements'.encode('utf-16le')in data
 assert all(sha(R/n)==h for n,h in source_hashes.items()),'Source changed during publish';assert all(sha(Path(n))==h for n,h in frozen.items()),'Old Linux Ateliers changed'
 OUT.mkdir();target=OUT/exe.name;shutil.copy2(exe,target);assert sha(target)==sha(exe)
 manifest={'version':'0.35.5','exeSha256':sha(target),'exeBytes':target.stat().st_size,'selfContained':True,'publishSingleFile':True,'includeNativeLibrariesForSelfExtract':True,'runtimeIdentifier':'linux-x64','framework':'net9.0','runtimeVersion':'9.0.5','sdkVersion':report['sdkVersion'],'publishTrimmed':False,'invariantGlobalization':True,'binaryReusedWithoutRebuild':False,'binaryBuildVersion':'0.35.5','sourceHashes':source_hashes,'createdUtc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'soloYoshiColors':18,'editorSpecialEggCount':12,'editorElementalPlacements':True,'unknownYoshiMenuIdentitiesHidden':True,'readonlyKnownMask18Api':True,'playerSelectionUi':'Native game menu; no separate web Yoshi picker','cpuJobsMaximum':1,'crossPublishedOnWindows':False,'apphostPatchedAfterBundling':False,'nativeLinuxExecutionVerified':False,'workshopApiVerified':False,'nativeLinuxBuildHost':True,'gameGpuBrowserOrPhysicalSoundUsed':False,'proof':str((O/'verification.json').relative_to(R)),'buildLog':str((O/'publish.log').relative_to(R)),'qualification':'Fresh Linux CI publish of actual0355 Core+Linux sources. ELF64/interpreter and complete6.0 .NET bundle verified, runtime9.0.5 and invariant globalization embedded. Native Linux/HTTP/runtime dependency qualification follows separately; no hardware Mint claim.'}
 (OUT/'publish-manifest0355.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
 report.update(passed=True,exeSha256=manifest['exeSha256'],exeBytes=manifest['exeBytes'],sourceInputsUnchanged=True,oldLinuxAteliersUnchanged=True,sourceCount=len(source_hashes),bundleEntries=count,bundleId=bundle_id,nativeEntries=sum(x['kind']==2 for x in entries),includedFrameworks=frameworks,elfInterpreter=interpreter,hostPrefixExactExceptManifestPointer=True,glibcVersionStringsObserved=sorted(set(x.decode()for x in re.findall(rb'GLIBC_[0-9]+\.[0-9]+',data))),output=str(OUT),manifestSha256=sha(OUT/'publish-manifest0355.json'))
except Exception as e:report['error']=str(e)
(O/'verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps({k:report.get(k)for k in ['passed','output','exeSha256','exeBytes','sourceCount','bundleEntries','error']},indent=2));print('REPORT '+str(O/'verification.json'));raise SystemExit(0 if report['passed']else 1)
