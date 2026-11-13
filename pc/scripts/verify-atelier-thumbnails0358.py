"""Actual loopback thumbnail API checks, Windows-hosted WorkshopLinux source.
No browser/game/Linux runtime/ROM/personal profile is used.
"""
from pathlib import Path
import hashlib,http.client,json,os,struct,subprocess,time,urllib.error,urllib.parse,urllib.request,uuid,zlib
R=Path('D:/Yoshi-Story');OUT=R/('pc/build/noncombat0358/api-check-'+uuid.uuid4().hex[:12]);OUT.mkdir(parents=True,exist_ok=False)
PC=OUT/'fixture Atélier/pc';PROFILE=PC/'profiles/content-personal';PROFILE.mkdir(parents=True)
(PROFILE/'keep-progress.txt').write_bytes(b'isolated progress sentinel\n');(PROFILE/'book-menu.json').write_text('{"language":"fr"}',encoding='utf-8')
(PC/'content/levels').mkdir(parents=True);(PC/'content/levels/keep.txt').write_bytes(b'creation sentinel\n')
APP=R/'pc/build/noncombat0358/api-build/YoshiStory-AtelierLinux.dll';LIB=PC/'content/bonus-thumbnails0358';checks=0;requests=0;server=None;log=None;report={'passed':False,'hostPlatform':os.name,'windowsHostedWorkshopLinuxApi':True,'actualLinuxExecution':False,'browserGpuOrGameUsed':False,'romOrPersonalDataUsed':False,'imagesAreSyntheticTestFixturesOnly':True}
def need(ok,message):
 global checks
 checks+=1
 if not ok:raise AssertionError(message)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def chunk(kind,payload):return struct.pack('>I',len(payload))+kind+payload+struct.pack('>I',zlib.crc32(kind+payload)&0xffffffff)
def png(color,width=320,height=180):
 row=b'\0'+bytes((*color,255))*width
 return b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',width,height,8,6,0,0,0))+chunk(b'IDAT',zlib.compress(row*height))+chunk(b'IEND',b'')
def inventory(folder):return {p.relative_to(folder).as_posix():sha(p)for p in folder.rglob('*')if p.is_file()}
try:
 env=os.environ.copy();env.update(DOTNET_PROCESSOR_COUNT='1',DOTNET_CLI_TELEMETRY_OPTOUT='1')
 log=(OUT/'server.log').open('w',encoding='utf-8');server=subprocess.Popen(['dotnet',str(APP),str(PC),'--no-browser'],cwd=OUT,env=env,stdout=log,stderr=log,creationflags=subprocess.CREATE_NO_WINDOW)
 session=PC/'build/atelier-linux-session.json';deadline=time.monotonic()+30;info=None
 while time.monotonic()<deadline:
  if server.poll()is not None:raise RuntimeError('Isolated API stopped before readiness; inspect private server.log')
  if session.exists():
   try:info=json.loads(session.read_text(encoding='utf-8'));break
   except (OSError,json.JSONDecodeError):pass
  time.sleep(.05)
 need(info is not None,'Readiness timeout');url,token=info['url'],info['token'];uri=urllib.parse.urlparse(url);need(uri.hostname=='127.0.0.1'and info['pid']==server.pid,'Loopback service identity differs')
 def call(path,body=None,expected=200,auth=True,chunked=False):
  global requests
  requests+=1;headers={'X-Atelier-Token':token}if auth else {}
  if body is not None:headers['Content-Type']='image/png'
  if chunked:
   connection=http.client.HTTPConnection(uri.hostname,uri.port,timeout=15)
   try:
    connection.request('POST',path,body=(body[i:i+16384]for i in range(0,len(body),16384)),headers=headers,encode_chunked=True);response=connection.getresponse();code=response.status;data=response.read();response_headers=dict(response.getheaders())
   finally:connection.close()
  else:
   request=urllib.request.Request(url+path,data=body,headers=headers)
   try:
    with urllib.request.urlopen(request,timeout=15)as response:code=response.status;data=response.read();response_headers=dict(response.headers)
   except urllib.error.HTTPError as error:code=error.code;data=error.read();response_headers=dict(error.headers)
  need(code==expected,path+' expected'+str(expected)+' got'+str(code)+' '+data[:160].decode(errors='replace'));return data,response_headers
 valid=png((12,150,90));other=png((220,80,130));guards=inventory(PROFILE);creation=sha(PC/'content/levels/keep.txt')
 call('/api/bonus-thumbnails',expected=403,auth=False);call('/api/bonus-thumbnails/1',valid,expected=403,auth=False)
 data=json.loads(call('/api/bonus-thumbnails')[0]);need(data['language']=='fr'and len(data['slots'])==6 and all(not s['available']for s in data['slots']),'Initial six empty slots differ');need(not LIB.exists(),'Read-only list created thumbnail directory')
 call('/api/bonus-thumbnails/1',expected=404);call('/api/bonus-thumbnails/0',valid,expected=400);need(not LIB.exists(),'Rejected invalid slot created assets')
 for bonus in range(1,7):
  body=png((bonus*20,100,170));call('/api/bonus-thumbnails/'+str(bonus),body);stored,headers=call('/api/bonus-thumbnails/'+str(bonus));need(stored==body,'Stored screenshot bytes differ');need(headers.get('Content-Type','').startswith('image/png')and headers.get('Cache-Control')=='no-store','Image response metadata differs')
 data=json.loads(call('/api/bonus-thumbnails')[0]);need(all(s['available']and s['status']=='ready'for s in data['slots']),'Saved six-slot state differs');need(len(list(LIB.iterdir()))==6,'Unexpected sidecar/temporary assets')
 old=inventory(LIB)
 for bad in (b'bad PNG',valid[:-1],valid[:40]+b'\0'+valid[41:],png((1,2,3),319,180)):
  call('/api/bonus-thumbnails/1',bad,expected=400);need(inventory(LIB)==old,'Rejected image replaced old screenshot')
 call('/api/bonus-thumbnails/7',valid,expected=400);need(inventory(LIB)==old,'Invalid slot wrote file')
 call('/api/bonus-thumbnails/1',b'x'*(1048576+1),expected=400);need(inventory(LIB)==old,'Content-Length overflow mutated assets')
 call('/api/bonus-thumbnails/1',b'x'*(1048576+1),expected=400,chunked=True);need(inventory(LIB)==old,'Chunked overflow mutated assets')
 call('/api/bonus-thumbnails/1',other);need((LIB/'7-1.png').read_bytes()==other and len(list(LIB.iterdir()))==6,'Atomic reimport did not replace one slot cleanly')
 call('/api/bonus-thumbnails/6',png((20,30,40),640,360));need(json.loads(call('/api/bonus-thumbnails')[0])['slots'][5]['available'],'640x360 normalized screenshot rejected')
 need(inventory(PROFILE)==guards and sha(PC/'content/levels/keep.txt')==creation,'Thumbnail operations changed progress/creations')
 (PROFILE/'book-menu.json').write_text('{"language":"en"}',encoding='utf-8');need(json.loads(call('/api/bonus-thumbnails')[0])['language']=='en','English preference not read')
 (PROFILE/'book-menu.json').write_text('[]',encoding='utf-8');need(json.loads(call('/api/bonus-thumbnails')[0])['language']=='fr'and(PROFILE/'book-menu.json').read_text()=='[]','Invalid preference not preserved/fallback failed')
 (PROFILE/'book-menu.json').write_text('{"language":"fr"}',encoding='utf-8');need(inventory(PROFILE)==guards,'Explicit fixture preference restoration failed')
 (LIB/'7-2.png').write_bytes(b'broken saved image');data=json.loads(call('/api/bonus-thumbnails')[0]);need(not data['slots'][1]['available']and data['slots'][1]['status']=='unavailable','Corrupt saved image prevented slot management');need((LIB/'7-2.png').read_bytes()==b'broken saved image','Read deleted corrupt user file')
 call('/api/bonus-thumbnails/2',valid);need(json.loads(call('/api/bonus-thumbnails')[0])['slots'][1]['available'],'Corrupt thumbnail could not be replaced')
 html=call('/')[0];need(html== (R/'pc/workshop/WorkshopLinux/index.html').read_bytes(),'Server embedded HTML differs from tested source');need(json.loads(call('/api/state')[0])['setupRequired']is True,'Fixture unexpectedly prepared game content')
 call('/api/quit',b'{}');server.wait(timeout=15);need(server.returncode==0,'API did not exit cleanly');report.update(passed=True,checks=checks,requests=requests,appDllSha256=sha(APP),coreDllSha256=sha(APP.parent/'Core.dll'),htmlSha256=hashlib.sha256(html).hexdigest(),profileUnchanged=True,creationUnchanged=True,quitClean=True)
except Exception as error:report['error']=str(error)
finally:
 if server is not None and server.poll()is None:
  server.terminate()
  try:server.wait(timeout=10)
  except subprocess.TimeoutExpired:server.kill();server.wait(timeout=10)
 if log is not None:log.close()
 (OUT/'verification.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps({k:v for k,v in report.items()if k in('passed','checks','requests','error')},ensure_ascii=False));print(str(OUT/'verification.json'));raise SystemExit(0 if report['passed']else 1)
