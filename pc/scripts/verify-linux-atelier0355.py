"""Headless actual0355 Linux Atelier API smoke; no ROM, game, GUI or audio is launched."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,stat,subprocess,sys,tempfile,time,urllib.error,urllib.parse,urllib.request,uuid
ROOT=Path(__file__).resolve().parents[2]
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--app',type=Path,required=True);parser.add_argument('--output',type=Path);parser.add_argument('--record-qualified-manifest',action='store_true');args=parser.parse_args()
 assert sys.platform=='linux','This proof requires actual Linux execution'
 app=args.app.resolve();manifest=app.parent/'publish-manifest0355.json';build=json.loads(manifest.read_text(encoding='utf-8'));assert sha(app)==build['exeSha256'] and build['version']=='0.35.5';assert all(build.get(name)is True for name in ('selfContained','publishSingleFile','includeNativeLibrariesForSelfExtract')) and build['runtimeVersion']=='9.0.5'
 out=(args.output or ROOT/'pc/build/linux-mint-0355'/('atelier-api-'+uuid.uuid4().hex[:12])).resolve();assert out.parent==(ROOT/'pc/build/linux-mint-0355').resolve() and out.name.startswith('atelier-api-');out.mkdir(exist_ok=False)
 fixture=tempfile.TemporaryDirectory(prefix='dinotale-atelier0355-');pc=Path(fixture.name)/'Atelier é0355 isolated/pc';profile=pc/'profiles/content-personal';profile.mkdir(parents=True);(pc/'portable-package.json').write_text('{}',encoding='utf-8')
 report={'passed':False,'hostPlatform':sys.platform,'appSha256':sha(app),'appBytes':app.stat().st_size,'manifestSha256':sha(manifest),'testScriptSha256':sha(__file__),'fixtureRoot':str(pc),'nativeGameLaunched':False,'gpuBrowserOrPhysicalSoundUsed':False,'romOrPersonalFilesAccessed':False,'syntheticDiscoveryFixturesOnly':True,'fixtureRightsMergedToRealProfile':False,'maximumDotnetProcessorCount':1};checks=0;requests=0;process=None;log=None;closed=False
 def need(ok,message):
  nonlocal checks
  checks+=1
  if not ok:raise AssertionError(message)
 def inventory():
  result={}
  for path in profile.rglob('*'):
   if path.is_symlink():result[str(path.relative_to(profile))]=['symlink',os.readlink(path)]
   elif path.is_file():result[str(path.relative_to(profile))]=['file',sha(path),path.stat().st_mode&0o777]
  return result
 def eeprom(found):
  allcopies=bytearray(2048)
  for copy in (0,1):
   block=memoryview(allcopies)[copy*1024:(copy+1)*1024];meta=[255,255,255,255,255,252|found]
   for row in range(48):block[6+8*row]=((meta[row//8]>>(7-row%8))&1)<<7
   block[1020:1024]=bytes.fromhex('81317539');checksum=0
   for value in block[:1018]:checksum^=value;checksum=((checksum<<1)|(checksum>>15))&65535
   block[1018:1020]=checksum.to_bytes(2,'big')
  return allcopies
 def journal(name,value):
  path=profile/name
  if path.is_symlink():path.unlink()
  path.write_text(value if isinstance(value,str) else json.dumps(value,separators=(',',':')),encoding='utf-8');path.chmod(0o600)
 try:
  deps=subprocess.run(['ldd',str(app)],capture_output=True,text=True,encoding='utf-8',timeout=15);(out/'ldd.log').write_text(deps.stdout+deps.stderr,encoding='utf-8');need(deps.returncode==0 and 'not found' not in deps.stdout+deps.stderr,'Apphost runtime dependency missing')
  env=os.environ.copy();env.update(DOTNET_PROCESSOR_COUNT='1',DOTNET_EnableDiagnostics='0',DOTNET_CLI_TELEMETRY_OPTOUT='1',DOTNET_MULTILEVEL_LOOKUP='0',DOTNET_ROLL_FORWARD='Disable',DOTNET_ROOT=str(out/'deliberately-absent-dotnet'),DOTNET_BUNDLE_EXTRACT_BASE_DIR=str(out/'bundle-extract'))
  for key in ('DISPLAY','WAYLAND_DISPLAY','PULSE_SERVER','DBUS_SESSION_BUS_ADDRESS'):env.pop(key,None)
  log=(out/'server.log').open('w',encoding='utf-8');process=subprocess.Popen([str(app),str(pc),'--no-browser'],cwd=out,env=env,stdin=subprocess.DEVNULL,stdout=log,stderr=log)
  session=pc/'build/atelier-linux-session.json';deadline=time.monotonic()+30;info=None
  while time.monotonic()<deadline:
   if process.poll() is not None:raise RuntimeError('Server exited: '+(out/'server.log').read_text(encoding='utf-8'))
   if session.exists():
    try:info=json.loads(session.read_text(encoding='utf-8'));break
    except (OSError,json.JSONDecodeError):pass
   time.sleep(.05)
  need(info is not None,'Atelier readiness timeout');url,token=info['url'],info['token'];uri=urllib.parse.urlparse(url);need(uri.hostname=='127.0.0.1' and uri.scheme=='http','Server is not loopback HTTP');need(info['pid']==process.pid and info['root']==str(pc),'Ready service identity mismatch');need(stat.S_IMODE(session.stat().st_mode)==0o600,'Session credentials not0600')
  def call(route,post=False,expected=200,auth=True):
   nonlocal requests
   requests+=1;headers={'X-Atelier-Token':token}if auth else {};request=urllib.request.Request(url+route,b'{}'if post else None,headers)
   try:
    with urllib.request.urlopen(request,timeout=15)as response:body=response.read();code=response.status
   except urllib.error.HTTPError as error:body=error.read();code=error.code
   need(code==expected,f'{route}: expected{expected}, got{code}: '+body[:160].decode(errors='replace'))
   return body
  call('/api/state',expected=403,auth=False);need(json.loads(call('/api/ping'))['pid']==process.pid,'Authenticated ping wrong process')
  html=call('/').decode('utf-8');need('id<=0x4f3b'in html and '·12 œufs'in html,'Embedded Linux egg editor missing0355 range');need('canvas'in html and 'Parties personnalisées'in html,'Embedded Atelier HTML missing')
  fixture_results=[]
  def state(name,expected):
   before=inventory();s=json.loads(call('/api/state'));need(s['knownYoshiMask18']==expected,f'{name}: wrong presentation mask{ s.get("knownYoshiMask18") }');need(inventory()==before,f'{name}: API wrote discovery/profile files')
   need(s['editor0340']is True and s['editor0355']is True and s['editorSourceVersion']=='0.35.5','Legacy/version capability contract changed');need(s['soloYoshiColors']==18 and s['editorSpecialEggCount']==12 and s['editorElementalPlacements']is True and s['unknownYoshiMenuIdentitiesHidden']is True,'0355 capabilities missing')
   need(s['setupRequired']is True and s['templates']==[] and s['catalog']==[] and s['running']is False,'Empty ROM fixture unexpectedly loaded game content');fixture_results.append({'name':name,'knownMask18':expected})
  ordinary=63;state('missing profile rights',ordinary)
  (profile/'saves').mkdir();save=profile/'saves/yoshisstory.n64.us.bin';save.write_bytes(bytes(2048));state('invalid EEPROM ignored read-only',ordinary)
  journal('pc-hybrid-finds.json',{'version':1,'foundMask':33,'legacyMigrated':True});state('permanent hybrid finds live read',ordinary|(33<<8))
  journal('pc-metal-yoshis.json',{'schema':1,'foundMask':3,'storyAliveMask':0});state('dead permanent metals stay known',ordinary|(33<<8)|(3<<14))
  journal('pc-elemental-yoshis.json',{'schema':2,'foundMask':3,'storyAliveMask':0,'hybridAliveMask':0});state('dead permanent elementals stay known',ordinary|(33<<8)|(15<<14))
  journal('pc-elemental-yoshis.json',{'schema':2,'foundMask':0,'storyAliveMask':3,'hybridAliveMask':0});state('invalid alive entitlement reveals nothing',ordinary|(33<<8)|(3<<14))
  journal('pc-elemental-yoshis.json',{'schema':2,'foundMask':0,'storyAliveMask':0,'hybridAliveMask':0,'localHardTeam':3});state('extra team field rejected',ordinary|(33<<8)|(3<<14))
  journal('pc-elemental-yoshis.json','{"schema":2,"foundMask":0,"foundMask":3,"storyAliveMask":0,"hybridAliveMask":0}');state('duplicate fields rejected',ordinary|(33<<8)|(3<<14))
  linked=out/'synthetic-linked-discovery.json';linked.write_text('{"schema":2,"foundMask":3,"storyAliveMask":0,"hybridAliveMask":0}',encoding='utf-8');path=profile/'pc-elemental-yoshis.json';path.unlink();path.symlink_to(linked);state('symlink discovery rejected',ordinary|(33<<8)|(3<<14))
  journal('pc-elemental-yoshis.json',{'schema':2,'foundMask':3,'storyAliveMask':0,'hybridAliveMask':0});journal('pc-hybrid-finds.json',{'version':1,'foundMask':63,'legacyMigrated':True});save.write_bytes(eeprom(3));state('valid canonical BW and all permanent finds',0x3ffff)
  broken=eeprom(3);broken[100]^=1;save.write_bytes(broken);state('canonical second EEPROM copy fallback',0x3ffff)
  both=eeprom(0);save.write_bytes(both);state('BW canonical found bits determine identity',0x3ffff^0xc0)
  reused=subprocess.run([str(app),str(pc),'--no-browser'],cwd=out,env=env,stdin=subprocess.DEVNULL,capture_output=True,text=True,encoding='utf-8',timeout=10);need(reused.returncode==0 and '"reused":true'in reused.stdout.replace(' ',''),'Second headless launch did not reuse authenticated own service');need(process.poll()is None,'Reused launcher stopped original service')
  call('/api/quit',post=True);process.wait(timeout=10);closed=True;need(process.returncode==0,'Server did not exit normally');need(sha(app)==report['appSha256'] and sha(manifest)==report['manifestSha256'],'Input app/manifest changed during qualification')
  report.update(passed=True,checks=checks,httpRequests=requests,fixtureResults=fixture_results,readonlyProfileApi=True,authRequired=True,authenticatedOwnServiceReuse=True,gracefulOwnServiceShutdown=True,installedDotnetNotRequired=True,qualification='Executed the actual self-contained0355 ELF under Linux using only headless HTTP. Legacy editor0340 contract retained;0355 capabilities,12egg embedded HTML, canonical EEPROM/permanent18mask live readonly projection and malformed/symlink rejection verified. All rights are synthetic isolated fixtures, never a real save or natural gameplay proof. No ROM, native game, browser, GPU or physical audio accessed.')
 except Exception as error:report['error']=str(error)
 finally:
  if process is not None and process.poll()is None:
   process.terminate()
   try:process.wait(timeout=5)
   except subprocess.TimeoutExpired:process.kill();process.wait(timeout=5)
  if log is not None:log.close()
  report['ownProcessExited']=process is None or process.poll()is not None;fixture.cleanup();report['ownSyntheticProfileRemoved']=not pc.exists();report['createdUtc']=datetime.datetime.now(datetime.timezone.utc).isoformat();(out/'verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps({key:report.get(key)for key in ('passed','checks','httpRequests','ownProcessExited','error')},indent=2));print('REPORT '+str(out/'verification.json'))
 if report['passed'] and args.record_qualified_manifest:
  need(sha(manifest)==report['manifestSha256'] and sha(app)==report['appSha256'],'Publication inputs changed before qualification annotation')
  proof=out/'verification.json';qualified=dict(build);qualified.update(nativeLinuxExecutionVerified=True,workshopApiVerified=True,apiProof=proof.relative_to(ROOT).as_posix(),apiProofSha256=sha(proof),qualification='Fresh0355 Core+Linux source publish, self-contained singlefile ELF verified with embedded9.0.5 runtimes. Exact ELF then executed under Linux headless with authenticated HTTP,12egg HTML and18color readonly canonical discovery API checks. No game/GPU/browser/physical audio or hardware Mint claim.')
  temporary=manifest.with_name(manifest.name+'.qualified-'+uuid.uuid4().hex+'.tmp')
  try:temporary.write_text(json.dumps(qualified,indent=2)+'\n',encoding='utf-8');os.replace(temporary,manifest)
  finally:
   if temporary.exists():temporary.unlink()
  print('QUALIFIED_MANIFEST '+str(manifest)+' SHA256 '+sha(manifest))
 return 0 if report['passed']else 1
if __name__=='__main__':raise SystemExit(main())
