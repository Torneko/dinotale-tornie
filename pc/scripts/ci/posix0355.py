"""Execute only the targeted0355 persistence CPU proof on an isolated Linux ext4 fixture."""
from pathlib import Path
import hashlib,json,os,subprocess,sys,tempfile
R=Path(__file__).resolve().parents[3];E=R/'pc/engine';O=R/'pc/build/linux-mint-0355/posix-ci';assert sys.platform=='linux' and os.environ.get('GITHUB_ACTIONS')=='true'
os.sched_setaffinity(0,set(sorted(os.sched_getaffinity(0))[:2]));O.mkdir(parents=True,exist_ok=False)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
inputs=[*E.glob('*.hpp'),E/'elemental_yoshis0355.cpp',E/'story_hard_access0330.cpp',E/'story_hard0330.cpp',E/'tests/LinuxPosix0355Checks.cpp',Path(__file__).resolve()]
before={str(p.relative_to(R)):sha(p) for p in inputs};report={'passed':False,'gameGpuOrAudioDeviceUsed':False,'personalDataUsed':False,'maximumBuildJobs':1}
try:
 with tempfile.TemporaryDirectory(prefix='dinotale0355-posix-') as name:
  work=Path(name);binary=work/'checks';vendor=R/'pc/dinotale/lib/N64ModernRuntime'
  command=['clang++-15','-std=c++20','-O1','-Wall','-Wextra','-Werror','-pthread','-I'+str(vendor/'thirdparty'),'-I'+str(vendor/'N64Recomp/include'),E/'tests/LinuxPosix0355Checks.cpp',E/'elemental_yoshis0355.cpp',E/'story_hard_access0330.cpp',E/'story_hard0330.cpp','-o',binary]
  with (O/'build.private.log').open('wb') as log:result=subprocess.run([str(v) for v in command],stdout=log,stderr=subprocess.STDOUT)
  assert result.returncode==0,'POSIX compilation failed; diagnostic log stays private'
  result=subprocess.run([str(binary),str(work/'fixtures')],capture_output=True,text=True,timeout=30);(O/'run.private.log').write_text(result.stdout+result.stderr)
  assert result.returncode==0,'Targeted POSIX fixture failed; diagnostic log stays private'
  execution=json.loads(result.stdout.strip().splitlines()[-1]);assert execution['passed']
  hashes={p.relative_to(work/'fixtures').as_posix():sha(p) for p in (work/'fixtures').rglob('*') if p.is_file() and not p.is_symlink()}
  (O/'fixture-hashes.json').write_text(json.dumps(hashes,indent=2)+'\n')
  assert all(sha(R/n)==digest for n,digest in before.items()),'POSIX source inputs changed'
  report.update(passed=True,execution=execution,inputSha256=before,inputsUnchanged=True,qualification='Actual0355 persistence services on isolated ext4, synthetic caller evidence only. No played medals, unchanged0354 route sweep, game/GPU, audio playback or personal files.')
except Exception as error:report['error']=str(error)
(O/'verification.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'passed':report['passed'],'checks':report.get('execution',{}).get('checks')},indent=2));raise SystemExit(0 if report['passed'] else 1)
