"""Reconstruct and recompile locally; no input game image is modified."""
from pathlib import Path
import argparse,hashlib,json,shutil,struct,subprocess,sys,zlib
p=argparse.ArgumentParser();p.add_argument('--rom',type=Path,required=True);p.add_argument('--recompiler',type=Path,required=True);a=p.parse_args()
pc=Path(__file__).resolve().parents[1];dist=pc/'distribution';out=pc/'build/special-port'
m=json.loads((dist/'content-patch.json').read_text(encoding='utf-8'));sha=lambda b:hashlib.sha256(b).hexdigest()
assert a.rom.stat().st_size==m['sourceSize']==16777216
source=a.rom.read_bytes();magic=source[:4]
if magic==bytes.fromhex('37804012'):source=b''.join(source[i:i+2][::-1] for i in range(0,len(source),2))
elif magic==bytes.fromhex('40123780'):source=b''.join(source[i:i+4][::-1] for i in range(0,len(source),4))
assert sha(source)==m['sourceSha256'],'Original USA ROM required'
patch=(dist/'content.bps').read_bytes();assert 16<len(patch)<4_194_304 and patch[:4]==b'BPS1' and sha(patch)==m['patchSha256']
src_crc,dst_crc,patch_crc=struct.unpack('<III',patch[-12:]);assert zlib.crc32(source)==src_crc and zlib.crc32(patch[:-4])==patch_crc
pos=4;end=len(patch)-12
# byuu BPS1 varints; bounded by SHA-verified release metadata.
def number():
 global pos
 n=0;shift=1
 for _ in range(10):
  assert pos<end,'Truncated BPS';b=patch[pos];pos+=1;n+=(b&127)*shift
  if b&128:return n
  shift<<=7;n+=shift
 raise ValueError('BPS integer overflow')
assert number()==len(source) and number()==m['targetSize'];metadata=number();assert metadata<=end-pos;pos+=metadata
result=bytearray();sr=tr=0
while pos<end:
 cmd=number();length=(cmd>>2)+1;mode=cmd&3;assert length<=m['targetSize']-len(result)
 if mode==0:assert len(result)+length<=len(source);result.extend(source[len(result):len(result)+length])
 elif mode==1:assert length<=end-pos;result.extend(patch[pos:pos+length]);pos+=length
 else:
  offset=number();delta=-(offset>>1) if offset&1 else offset>>1
  if mode==2:
   sr+=delta;assert 0<=sr<=len(source)-length;result.extend(source[sr:sr+length]);sr+=length
  else:
   tr+=delta;assert 0<=tr<len(result)
   for _ in range(length):result.append(result[tr]);tr+=1
assert len(result)==m['targetSize'] and zlib.crc32(result)==dst_crc and sha(result)==m['targetSha256']
assert not out.exists(),'Choose a clean checkout; generated output already exists'
recompiler=a.recompiler.resolve();assert recompiler.is_file();out.mkdir(parents=True)
(out/'content.z64').write_bytes(result)
for n in ('content.toml','content.syms.toml','content_config.hpp','function-sections.json'):shutil.copy2(pc/'recompile'/n,out/n)
subprocess.run([str(recompiler),str(out/'content.toml')],cwd=out,check=True)
for script in ('fix-content-link-register.py','trace-special-startup.py'):subprocess.run([sys.executable,str(pc/'scripts'/script),str(out)],check=True)
expected=json.loads((pc/'recompile/generated-sha256.json').read_text(encoding='utf-8'))
actual={f.name:sha(f.read_bytes().replace(b'\r\n',b'\n')) for f in (out/'RecompiledFuncs').iterdir() if f.is_file()}
assert actual==expected,'Generated functions differ from this release'
print('Content and all generated sources verified; ready for native CMake build.')
