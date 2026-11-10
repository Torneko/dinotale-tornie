"""Apply the release hooks to verified locally-generated game functions."""
from pathlib import Path
import argparse,hashlib,json,subprocess
p=argparse.ArgumentParser();p.add_argument('output',type=Path);a=p.parse_args()
pc=Path(__file__).resolve().parents[1];generated=a.output.resolve()/'RecompiledFuncs'
def hashes():return {f.name:hashlib.sha256(f.read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in generated.iterdir() if f.is_file()}
base=json.loads((pc/'recompile/generated-sha256.json').read_text());target=json.loads((pc/'recompile/generated-0340-sha256.json').read_text())
assert hashes()==base,'Generated base differs; no patch applied'
for f in generated.iterdir():
 if f.is_file():f.write_bytes(f.read_bytes().replace(b'\r\n',b'\n'))
patch=(pc/'recompile/native-hooks-0340.patch').resolve()
subprocess.run(['git','apply','--check','--whitespace=nowarn',str(patch)],cwd=generated,check=True)
subprocess.run(['git','apply','--whitespace=nowarn',str(patch)],cwd=generated,check=True)
assert hashes()==target,'Patched output differs from release'
print('All219 generated sources match release0.34.0 after newline normalization.')
