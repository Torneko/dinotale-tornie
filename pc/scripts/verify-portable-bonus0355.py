"""Check portable Bonus PrepareOnly on isolated synthetic profiles; never launch a game."""
from pathlib import Path
import argparse,hashlib,json,os,re,shutil,subprocess,uuid
R=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--package',type=Path,required=True);a=p.parse_args();package=a.package.resolve()
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
meta=json.loads((package/'pc/portable-package.json').read_text());script=package/'Tester-Bonus.ps1';body=script.read_text(encoding='utf-8');source_sha=sha(script)
need=lambda ok,message:None if ok else (_ for _ in ()).throw(AssertionError(message))
need(meta['engineSourceVersion']=='0.35.5' and meta['soloYoshiColors']==18,'Qualified0355 required')
out=R/'pc/build/publication-0355'/('bonus-portable-'+uuid.uuid4().hex[:12]);out.mkdir(parents=True)
fixture=out/'Copie portable é0355';fixture.mkdir();pc=fixture/'pc';profile=pc/'profiles/content-personal';profile.mkdir(parents=True);runtime=pc/'build/native-special';runtime.mkdir(parents=True)
original=R/'work/yoshis-story-decomp/baseroms/us/baserom.z64';need(sha(original)=='e8a63388c38f8f0bea37e9b13a97f0898ac5496e08ca2028afb8db9a866e0ce9','Internal original source differs')
for name in ['DinoTale.exe','MusicRender0310.exe']:shutil.copy2(package/'pc/build/native-special'/name,runtime/name)
shutil.copy2(package/'pc/profiles/content-personal/saves/yoshisstory.n64.us.bin',out/'seed.bin');(profile/'saves').mkdir();shutil.copy2(out/'seed.bin',profile/'saves/yoshisstory.n64.us.bin');shutil.copy2(original,profile/'yoshisstory.n64.us.z64')
(pc/'portable-package.json').write_text('{}');(runtime/'assets').mkdir();(runtime/'assets/fixture.txt').write_text('Synthetic asset, no game data.\n')
(profile/'pc-wallet.json').write_text('{"fixture":"readonly canonical wallet"}');(profile/'sound.json').write_text('{"main_volume":3}')
dest=pc/'profiles/bonus0353-qa-portable';dest.mkdir();(dest/'pc-bonus-progress0353.json').write_text('{"fixture":"preserve independent Bonus progress"}')
bonus_sha=sha(dest/'pc-bonus-progress0353.json')
# The real helper already refuses any concurrent game. This isolated fixture must
# not query or affect an unrelated user's running game. Only that process-query
# boundary is replaced; every path/hash/snapshot/cleanup operation stays real.
pattern=r'function Assert-NoGame \{\n.*?^\}'
fixture_body,count=re.subn(pattern,'function Assert-NoGame { }',body,count=1,flags=re.S|re.M);need(count==1,'Expected one process-query boundary')
fixture_body=fixture_body.replace("$stage = $null", "function Start-Process { throw 'Fixture forbids any process launch' }\n$stage = $null",1)
fixture_script=fixture/'Tester-Bonus.ps1';fixture_script.write_text(fixture_body,encoding='utf-8',newline='\n')
records=[];checks=0
def inventory(root):return {f.relative_to(root).as_posix():sha(f)for f in root.rglob('*')if f.is_file()}
def run(name,content,expect=0):
 global checks
 elemental=profile/'pc-elemental-yoshis.json'
 if content is None:
  if elemental.exists():elemental.unlink()
 else:elemental.write_bytes(content if isinstance(content,bytes)else content.encode())
 before=inventory(profile);bonus_before=sha(dest/'pc-bonus-progress0353.json')
 result=subprocess.run(['powershell.exe','-NoProfile','-NonInteractive','-ExecutionPolicy','Bypass','-File',str(fixture_script),'-PrepareOnly'],cwd=out,capture_output=True,text=True,encoding='utf-8',errors='replace',timeout=45,creationflags=subprocess.CREATE_NO_WINDOW)
 (out/(name+'.log')).write_text(result.stdout+result.stderr,encoding='utf-8');need((result.returncode==0)==(expect==0),name+' unexpected exit: '+str(result.returncode));checks+=1
 need(inventory(profile)==before,name+' mutated source');checks+=1;need(sha(dest/'pc-bonus-progress0353.json')==bonus_before==bonus_sha,name+' changed independent Bonus progress');checks+=1
 if expect==0:
  summary=json.loads(result.stdout.strip().splitlines()[-1]);need(summary['prepared'] and summary['prepareOnly'] and not summary['launched'] and not summary['unlocksForced'] and not summary['qaProgressImported'] and not summary['personalFilesWritten'],name+' wrong summary');checks+=1
  need(summary['profile']=='bonus0353-qa-portable' and summary['supportedSoloColors']==18 and summary['hardPrerequisiteColors']==16,name+' wrong portable support');checks+=1
  if content is not None:need((dest/'pc-elemental-yoshis.json').read_bytes()==elemental.read_bytes(),name+' lossy journal copy');checks+=1
 else:summary=None
 need(not list(dest.glob('.snapshot-stage-*')),name+' left a staging tree');checks+=1;records.append({'case':name,'passed':True,'exit':result.returncode,'summary':summary})
run('schema1-byte-exact','{"schema":1,"foundMask":1,"storyAliveMask":1}\n')
run('schema2-distinct-lives','{"schema":2,"foundMask":3,"storyAliveMask":1,"hybridAliveMask":2}\n')
run('schema2-bom-zero',b'\xef\xbb\xbf{"schema":2,"foundMask":0,"storyAliveMask":0,"hybridAliveMask":0}')
run('missing-source-removes-stale',None);need(not (dest/'pc-elemental-yoshis.json').exists(),'Stale elemental snapshot remained');checks+=1
for name,data in [('unearned-lives','{"schema":2,"foundMask":1,"storyAliveMask":2,"hybridAliveMask":0}'),('duplicate-key','{"schema":1,"foundMask":1,"foundMask":1,"storyAliveMask":1}'),('unknown-key','{"schema":1,"foundMask":1,"storyAliveMask":1,"extra":1}'),('boolean','{"schema":1,"foundMask":true,"storyAliveMask":1}'),('invalid-utf8',b'{"schema":1,"foundMask":1,"storyAliveMask":1}\xff')]:run(name,data,1)
need(sha(original)=='e8a63388c38f8f0bea37e9b13a97f0898ac5496e08ca2028afb8db9a866e0ce9' and sha(script)==source_sha,'Qualified input changed')
report={'passed':True,'checks':checks,'cases':records,'productionHelperSha256':source_sha,'fixtureHelperSha256':sha(fixture_script),'fixtureOverrides':['Process-query boundary returns no games','Start-Process always throws'],'realPathHashJournalSnapshotAndCleanupCode':True,'syntheticProfileOnly':True,'personalFilesAccessed':False,'gameGpuUiLaunched':False,'userProcessesQueriedOrClosed':False,'packageUnchanged':True,'originalRomUnchanged':True,'profile':str(fixture),'qualification':'Real portable0355 helper PrepareOnly operations on isolated source/Bonus profiles. Process-query boundary is synthetic to leave the user game untouched; native game launch is impossible in fixtures.'}
(out/'verification.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'passed':True,'checks':checks,'cases':len(records),'report':str(out/'verification.json')},indent=2))
