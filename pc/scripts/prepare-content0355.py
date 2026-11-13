"""Prepare the0355 game functions locally from an original USA ROM; input stays read only."""
from pathlib import Path
import argparse,hashlib,json,subprocess,sys
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def need(ok,message):
 if not ok:raise ValueError(message)
def main():
 root=Path(__file__).resolve().parents[2];pc=root/'pc';p=argparse.ArgumentParser(description=__doc__);p.add_argument('--rom',type=Path,required=True);p.add_argument('--recompiler',type=Path,required=True);a=p.parse_args()
 source=a.rom.resolve();tool=a.recompiler.resolve();need(source.is_file() and tool.is_file(),'Original ROM and recompiler files are required');before=sha(source);before_tool=sha(tool);out=pc/'build/special-port';need(not out.exists(),'Use a clean output; existing prepared files are preserved')
 subprocess.run([sys.executable,str(pc/'scripts/prepare-content.py'),'--rom',str(source),'--recompiler',str(tool)],check=True,cwd=root)
 subprocess.run([sys.executable,str(pc/'scripts/apply-native-hooks0355.py'),str(out)],check=True,cwd=root)
 expected=json.loads((pc/'recompile/generated-0355-sha256.json').read_text(encoding='utf-8'));actual={f.name:hashlib.sha256(f.read_bytes().replace(b'\r\n',b'\n')).hexdigest()for f in (out/'RecompiledFuncs').iterdir()if f.is_file()}
 need(len(actual)==219 and actual==expected,'Prepared0355 output differs from its published manifest');need(sha(source)==before and sha(tool)==before_tool,'Preparation input changed')
 report={'version':'0.35.5','passed':True,'generatedFiles':219,'inputFileSha256':before,'recompilerSha256':before_tool,'originalInputModified':False,'preparedContentSha256':sha(out/'content.z64'),'audioPreparationRequired':True}
 (out/'preparation0355.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
