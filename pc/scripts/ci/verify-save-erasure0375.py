"""Portable actual193-check save-erasure fixture; synthetic fresh directory only.
Default prints a plan. --run is restricted to the Ubuntu22.04 Actions runner.
No game, ROM, real profile, private C/RSP source, key or graphics is accessed.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,os,subprocess,sys,uuid
R=Path(__file__).resolve().parents[3];FRESH='ccca4870f3b5f2d11519568c1f003387a62f5d13adbed9ed99c8089b74c68585'
def need(v,m):
 if not v:raise RuntimeError(m)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--run',action='store_true');a=p.parse_args()
 if not a.run:print(json.dumps({'planOnly':True,'actualCppFixture':'pc/engine/tests/SaveErasure0375Checks.cpp','compiler':'clang++-15 -std=c++20 -O2 -Wall -Wextra -Werror','gameRomProfilesWslOrNetworkUsed':False}));return 0
 need(sys.platform=='linux'and os.environ.get('GITHUB_ACTIONS')=='true'and 'VERSION_ID="22.04"'in Path('/etc/os-release').read_text(),'Dedicated Ubuntu22.04 Actions runner required')
 commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=R,text=True).strip();need(commit==os.environ.get('SOURCE_COMMIT'),'Frozen source commit differs');out=R/('pc/build/save-erasure0375/linux-cpu-'+uuid.uuid4().hex[:12]);out.mkdir(parents=True,exist_ok=False);fixture=out/'synthetic-profile-fixture';exe=out/'checks';source=R/'pc/engine/tests/SaveErasure0375Checks.cpp';header=R/'pc/engine/save_erasure0375.hpp';inputs={n.relative_to(R).as_posix():sha(n)for n in(source,header,Path(__file__).resolve())};report={'passed':False,'version':'0.35.5','privateRevision':'0355-saveerase1','sourceCommit':commit,'hostPlatform':sys.platform,'sourceHashes':inputs,'syntheticProfilesOnly':True,'gameGpuWslRomPersonalOrTest100Used':False,'maximumCompilerJobs':1}
 try:
  command=['clang++-15','-std=c++20','-O2','-Wall','-Wextra','-Werror',str(source),'-o',str(exe)];build=subprocess.run(command,cwd=out,capture_output=True,text=True,timeout=120);(out/'build.log').write_text(build.stdout+build.stderr,encoding='utf-8');need(build.returncode==0,'Portable fixture compilation failed; inspect build.log');need(not fixture.exists(),'Disposable fixture must be new');run=subprocess.run([str(exe),str(fixture)],cwd=out,capture_output=True,text=True,timeout=60);(out/'run.log').write_text(run.stdout+run.stderr,encoding='utf-8');need(run.returncode==0,'Actual native fixture failed; inspect run.log');execution=json.loads(run.stdout);need(execution.get('passed')is True and execution.get('checks')==193,'Actual193-check count differs')
  for flag in('actualFilesystemReset','waitBeforeFirstMutation','pendingRecovery','neighborProfilePreserved'):need(execution.get(flag)is True,'Missing actual filesystem assertion '+flag)
  need(execution.get('personalOrTest100Used')is False,'Real profile use forbidden');factory=fixture/'factory.bin';need(factory.stat().st_size==2048 and sha(factory)==FRESH,'Canonical factory seed differs');need(all(sha(R/n)==h for n,h in inputs.items()),'Portable source inputs changed');report.update(passed=True,sourceInputsUnchanged=True,actualPortableCppFixtureExecuted=True,execution=execution,factorySha256=sha(factory),factoryBytes=2048,compiler=subprocess.check_output(['clang++-15','--version'],text=True).splitlines()[0],command=command,executableSha256=sha(exe),buildLogSha256=sha(out/'build.log'),runLogSha256=sha(out/'run.log'),naturalGuiHandoffVerified=False)
 except Exception as error:report['error']=str(error)
 (out/'verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps({'passed':report['passed'],'report':str(out/'verification.json'),'checks':report.get('execution',{}).get('checks'),'error':report.get('error')}));return 0 if report['passed']else 1
if __name__=='__main__':raise SystemExit(main())
