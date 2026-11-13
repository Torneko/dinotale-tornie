"""Generate local audio RSP C++ from the verified prepared content, never the original ROM."""
from pathlib import Path
from datetime import datetime,timezone
import argparse,hashlib,json,os,re,subprocess,uuid

CONTENT_SHA='d5800ba53ceceb261048d72289805ed531e0f708916546febf7a41c22cdee87d'
KNOWN_RSP_SHA='eb8bec48c25b91f72e4ede6016cb6697b7d27d2701fd4dcf1ba2ab75a40d5de7'
TARGETS=[0x1F08,0x122C,0x1148,0x1EB8,0x1DCC,0x14F0,0x1E3C,0x1C7C,0x1178,0x17C8,0x11E4,0x1A70,0x1B00,0x118C,0x1C28,0x121C,0x1780,0x182C,0x19EC,0x1880,0x1A50,0x1A60,0x1A14,0x1B6C]
def need(ok,message):
 if not ok:raise ValueError(message)
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def normalized(path):return hashlib.sha256(Path(path).read_bytes().replace(b'\r\n',b'\n')).hexdigest()
def write(path,value):path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8')
def config_text(path):
 text=path.read_text(encoding='utf-8-sig')
 def value(name):
  rows=re.findall(r'^\s*'+re.escape(name)+r'\s*=\s*([^\r\n#]+)',text,re.M);need(len(rows)==1,'Expected one '+name);return rows[0].strip()
 for name,expected in [('text_offset',0x94E60),('text_size',0xF50),('text_address',0x04001000)]:need(int(value(name),0)==expected,'Unexpected RSP profile '+name)
 need(value('output_function_name')=='"aspMain"','Unexpected RSP function')
 branch=re.search(r'^\s*extra_indirect_branch_targets\s*=\s*\[([^\]]+)\]',text,re.M|re.S);need(branch is not None,'Branch targets missing')
 need([int(x.strip(),0) for x in branch[1].split(',') if x.strip()]==TARGETS,'Unexpected RSP branch targets')
 value('rom_file_path');value('output_file_path');return text
def validate_source(path):
 data=path.read_bytes();need(10000<len(data)<2_000_000,'Unexpected RSP source size');text=data.decode('utf-8')
 need('#include "librecomp/rsp.hpp"' in text and '#include "librecomp/rsp_vu_impl.hpp"' in text and len(re.findall(r'RspExitReason\s+aspMain\s*\(',text))==1,'Unexpected RSP output structure')
def prepare(root,content,rsp_recompiler=None):
 root=Path(root).resolve();pc=root/'pc';content=Path(content).resolve();target=pc/'dinotale/rsp/aspMain.cpp';receipt=pc/'build/audio0355/receipt.json';profile=pc/'dinotale/aspMain.us.toml'
 need(content.is_file() and content.stat().st_size==16777216 and sha(content)==CONTENT_SHA,'Verified prepared D580 content is required; no source image is modified')
 text=config_text(profile);config_sha=sha(profile);tool=Path(rsp_recompiler).resolve() if rsp_recompiler else None
 if tool:need(tool.is_file(),'RSPRecomp tool missing')
 need(not target.is_symlink() and target.resolve().is_relative_to((pc/'dinotale').resolve()),'RSP destination must stay inside the source checkout')
 if target.exists():
  need(target.is_file(),'RSP output is not a regular file');validate_source(target);current=normalized(target)
  if receipt.exists():
   previous=json.loads(receipt.read_text(encoding='utf-8'));need(previous.get('schema')==1 and previous.get('contentSha256')==CONTENT_SHA and previous.get('configSha256')==config_sha and previous.get('rspNormalizedSha256')==current and previous.get('rspSha256')==sha(target),'Existing RSP receipt does not match; preserved without overwrite')
   need(re.fullmatch(r'[0-9a-f]{64}',previous.get('toolSha256','')) is not None or previous.get('knownReferenceReused') is True,'Existing tool receipt is incomplete')
   if tool and previous.get('toolSha256'):need(previous['toolSha256']==sha(tool),'Existing RSP was generated with a different tool; preserved without overwrite')
  else:need(current==KNOWN_RSP_SHA,'Unrecognized existing RSP source; preserved without overwrite')
  return {'passed':True,'version':'0.35.5','reused':True,'knownReferenceReused':current==KNOWN_RSP_SHA,'rspPath':str(target),'rspSha256':sha(target),'rspNormalizedSha256':current,'contentSha256':CONTENT_SHA,'configSha256':config_sha,'receipt':str(receipt) if receipt.exists() else None,'audioSemanticsTested':False,'workerIdentityRequiresQualification':True}
 need(not receipt.exists(),'A receipt exists without its RSP source; preserved without replacement');need(tool is not None,'Supply --rsp-recompiler built from the pinned N64Recomp source')
 work=pc/'build/audio0355'/('generation-'+uuid.uuid4().hex[:12]);work.mkdir(parents=True);generated=work/'aspMain.cpp'
 def replacement(name,path):
  nonlocal text
  text,count=re.subn(r'^(\s*'+re.escape(name)+r'\s*=\s*)[^\r\n]+',lambda m:m[1]+json.dumps(path.as_posix(),ensure_ascii=False),text,count=1,flags=re.M);need(count==1,'Could not prepare '+name)
 replacement('rom_file_path',content);replacement('output_file_path',generated.resolve());configuration=work/'aspMain.prepared0355.toml';configuration.write_text(text,encoding='utf-8',newline='\n')
 before_content=sha(content);before_tool=sha(tool);before_config=sha(profile)
 result=subprocess.run([str(tool),str(configuration.resolve())],cwd=work,capture_output=True,timeout=180);(work/'generation.log').write_bytes(result.stdout+result.stderr);need(result.returncode==0,'RSPRecomp failed; inspect the private local generation log')
 need(generated.is_file() and not generated.is_symlink(),'RSP source was not generated');validate_source(generated)
 need(sha(content)==before_content and sha(tool)==before_tool and sha(profile)==before_config,'An input changed during RSP generation')
 need(not target.exists() and not target.is_symlink(),'RSP destination appeared during generation; nothing overwritten');target.parent.mkdir(parents=True,exist_ok=True)
 # Exclusive creation prevents an output arriving concurrently from being overwritten.
 with target.open('xb') as stream:stream.write(generated.read_bytes())
 need(sha(target)==sha(generated),'Copied RSP differs')
 report={'schema':1,'passed':True,'version':'0.35.5','reused':False,'knownReferenceReused':False,'contentSha256':CONTENT_SHA,'contentBytes':content.stat().st_size,'configSha256':config_sha,'generationConfigSha256':sha(configuration),'toolSha256':before_tool,'toolPath':str(tool),'toolSource':'Build RSPRecomp from the pinned N64ModernRuntime/N64Recomp checkout','rspPath':str(target),'rspSha256':sha(target),'rspNormalizedSha256':normalized(target),'microcodeTextSha256':hashlib.sha256(content.read_bytes()[0x94E60:0x94E60+0xF50]).hexdigest(),'generationLogSha256':sha(work/'generation.log'),'audioSemanticsTested':False,'workerIdentityRequiresQualification':True,'caseOrderingMayDifferAcrossToolchains':True,'originalRomCopiedOrModified':False,'createdUtc':datetime.now(timezone.utc).isoformat()}
 write(receipt,report);return report
def main():
 root=Path(__file__).resolve().parents[2];p=argparse.ArgumentParser(description=__doc__);p.add_argument('--content',type=Path,default=root/'pc/build/special-port/content.z64');p.add_argument('--rsp-recompiler',type=Path);a=p.parse_args();result=prepare(root,a.content,a.rsp_recompiler);print(json.dumps(result,indent=2))
if __name__=='__main__':main()
