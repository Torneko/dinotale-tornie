"""Extract only original native audio closure into an isolated CPU-only probe."""
from pathlib import Path
import re,json,hashlib,sys,argparse
ROOT=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output-dir',type=Path,default=ROOT/'pc/build/menu-music-0310')
parser.add_argument('--native-dir',type=Path,default=ROOT/'pc/build/coop-0233-crash-fix/RecompiledFuncs')
args=parser.parse_args()
OUT=args.output_dir.resolve()
OUT.mkdir(parents=True,exist_ok=True)
SRC=args.native_dir.resolve()
funcs={}; origins={}
for p in SRC.glob('funcs_*.c'):
 s=p.read_text()
 for m in re.finditer(r'RECOMP_FUNC void (\w+)\([^\n]+\) \{[\s\S]*?\n;}',s):
  funcs[m[1]]=m[0];origins[m[1]]=p.name
roots=['func_80010520','func_80010558','func_800679B0','func_80009930']
seen=set();todo=roots[:];external=set()
while todo:
 n=todo.pop()
 if n in seen:continue
 if n not in funcs:external.add(n);continue
 seen.add(n)
 todo.extend(x for x in re.findall(r'^\s*(\w+)\(rdram, ctx\);',funcs[n],re.M) if x!='workshop_trace_entry')
assert len(seen)==339,(len(seen),external)
header='#include "recomp.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
for n in sorted(seen|external):header+=f'void {n}(uint8_t*,recomp_context*);\n'
header+='void music_native_call(uint32_t,uint8_t*,recomp_context*);\n#ifdef __cplusplus\n}\n#endif\n'
(OUT/'funcs.h').write_text(header)
body='#include "funcs.h"\n'
for n in sorted(seen):
 b=funcs[n]
 address=re.search(r'// 0x([0-9A-Fa-f]{8}):',b)[1]
 b=b.replace('{','{\n    music_native_call(0x'+address+'u,rdram,ctx);',1)
 body+=b+'\n'
(OUT/'audio_closure.c').write_text(body)
mp=''
for n in sorted(seen):
 address=re.search(r'// 0x([0-9A-Fa-f]{8}):',funcs[n])[1]
 mp+=f'case 0x{address}u:return {n};\n'
(OUT/'function_map.inc').write_text(mp)
manifest={'roots':roots,'bodies':len(seen),'external_services':sorted(external),'native_source_sha256':hashlib.sha256(body.encode()).hexdigest(),'origins':{n:origins[n] for n in sorted(seen)}}
(OUT/'extraction.json').write_text(json.dumps(manifest,indent=2))
print(json.dumps({k:v for k,v in manifest.items() if k!='origins'},indent=2))

identity_inputs={"closure":hashlib.sha256(body.encode()).hexdigest()}
for relative in ["pc/engine/menu_music_native0310.cpp","pc/engine/menu_music_native0310.hpp","pc/dinotale/rsp/aspMain.cpp","pc/dinotale/lib/N64ModernRuntime/librecomp/src/rsp.cpp","pc/dinotale/lib/N64ModernRuntime/librecomp/include/librecomp/rsp.hpp","pc/dinotale/lib/N64ModernRuntime/librecomp/include/librecomp/rsp_vu.hpp","pc/dinotale/lib/N64ModernRuntime/librecomp/include/librecomp/rsp_vu_impl.hpp","pc/dinotale/lib/N64ModernRuntime/N64Recomp/include/recomp.h"]:
 identity_inputs[relative]=hashlib.sha256((ROOT/relative).read_text().encode()).hexdigest()
identity=hashlib.sha256(json.dumps(identity_inputs,sort_keys=True,separators=(",",":")).encode()).hexdigest()
(OUT/"native_identity.inc").write_text('#define MENU_MUSIC_NATIVE0310_ID "'+identity+'"\n')
(OUT/"renderer-identity.json").write_text(json.dumps({"sha256":identity,"inputs":identity_inputs},indent=2))
print("renderer",identity)
