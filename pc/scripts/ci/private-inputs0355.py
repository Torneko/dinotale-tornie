"""Authenticated private native-input transport. No ROM, content image or profile accepted."""
from pathlib import Path,PurePosixPath
import argparse,hashlib,hmac,io,json,os,re,shutil,subprocess,tarfile,tempfile
MAGIC=b'DINOTALE0355CBC1\n';COUNT=219
CONTENT='d5800ba53ceceb261048d72289805ed531e0f708916546febf7a41c22cdee87d'
EXPECTED={f'funcs_{i}.c' for i in range(216)}|{'lookup.cpp','funcs.h','recomp_overlays.inl'}
def sha(data):return hashlib.sha256(data).hexdigest()
def canonical(value):return json.dumps(value,sort_keys=True,separators=(',',':')).encode()
def normalized(path):return sha(path.read_text(encoding='utf-8').encode())
def key():
 value=os.environ.get('DINOTALE_CI_PRIVATE_KEY','').strip()
 assert re.fullmatch(r'[0-9a-f]{64}',value),'Supply DINOTALE_CI_PRIVATE_KEY as a random256-bit lower-casehex secret'
 return value

def openssl():
 configured=os.environ.get('DINOTALE_OPENSSL')
 candidate=configured or shutil.which('openssl') or r'C:\Program Files\Git\usr\bin\openssl.exe'
 assert Path(candidate).is_file(),'OpenSSL executable unavailable'
 return candidate

def crypt(raw,secret,decrypt=False):
 # Passphrase goes only through stdin, never command arguments or console.
 with tempfile.TemporaryDirectory(prefix='dinotale0355-crypto-') as folder:
  path=Path(folder)/'input';path.write_bytes(raw)
  command=[openssl(),'enc','-aes-256-cbc','-pbkdf2','-iter','200000','-md','sha256','-pass','stdin','-in',str(path)]
  if decrypt:command.append('-d')
  result=subprocess.run(command,input=(secret+'\n').encode(),capture_output=True)
  assert result.returncode==0,'OpenSSL authenticated-input operation failed'
  return result.stdout

def mac_key(secret):return hashlib.pbkdf2_hmac('sha256',bytes.fromhex(secret),b'dinotale0355-transport-auth-v1',200000)
def pack(args):
 root=args.root.resolve();native=args.native.resolve();rsp=args.rsp.resolve();freeze=args.generated_manifest.resolve()
 assert root.is_dir() and native.is_dir() and rsp.is_file()
 assert set(p.name for p in native.iterdir())==EXPECTED,'Native folder must contain exactly the approved219 files'
 assert not any(p.is_symlink() or not p.is_file() for p in native.iterdir())
 frozen=json.loads(freeze.read_text(encoding='utf-8'));assert set(frozen)==EXPECTED,'Frozen normalized219manifest mismatch'
 assert all(normalized(native/name)==digest for name,digest in frozen.items()),'Native sources differ from final public frozen manifest'
 assert re.fullmatch(r'[0-9a-f]{40}',args.source_commit)
 assert normalized(rsp)==args.rsp_normalized_sha256,'PreservedRSP differs from final frozen hash'
 mapping={f'RecompiledFuncs/{n}':sha((native/n).read_bytes()) for n in sorted(EXPECTED)};mapping['rsp/aspMain.cpp']=sha(rsp.read_bytes())
 manifest={'schema':1,'version':'0.35.5','sourceCommit':args.source_commit,'files':mapping,'generatedCount':COUNT,'generatedNormalizedSha256':frozen,'generatedManifestSha256':sha(canonical(frozen)),'rspSha256':sha(rsp.read_bytes()),'rspNormalizedSha256':normalized(rsp),'contentSha256':CONTENT,'contentFnv64':'5E83770716DC3E31','originalRomIncluded':False,'preparedContentIncluded':False,'saveIncluded':False}
 data=io.BytesIO()
 with tarfile.open(fileobj=data,mode='w:gz',format=tarfile.PAX_FORMAT) as archive:
  payloads={'manifest.json':canonical(manifest)}
  payloads.update({name:(native/PurePosixPath(name).name).read_bytes() if name.startswith('RecompiledFuncs/') else rsp.read_bytes() for name in mapping})
  for name,payload in sorted(payloads.items()):
   entry=tarfile.TarInfo(name);entry.size=len(payload);entry.mode=0o600;entry.mtime=0;archive.addfile(entry,io.BytesIO(payload))
 plain=data.getvalue();secret=key();cipher=crypt(plain,secret);transport=MAGIC+hmac.new(mac_key(secret),cipher,hashlib.sha256).digest()+cipher
 assert not args.output.exists(),'Refuse to overwrite encrypted input'
 args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_bytes(transport)
 report={'schema':1,'version':'0.35.5','sourceCommit':args.source_commit,'encryptedBundleSha256':sha(transport),'bundlePlaintextSha256':sha(plain),'manifestSha256':sha(canonical(manifest)),'generatedCount':COUNT,'generatedManifestSha256':manifest['generatedManifestSha256'],'rspSha256':manifest['rspSha256'],'originalRomIncluded':False,'preparedContentIncluded':False,'saveIncluded':False,'encryption':'AES-256-CBC PBKDF2-SHA256200000 plus independent HMAC-SHA256 encrypt-then-MAC','secretEmitted':False}
 args.output.with_suffix(args.output.suffix+'.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report,indent=2))

def unpack(args):
 encrypted=args.input.read_bytes();assert sha(encrypted)==args.encrypted_sha256,'Encrypted input SHA mismatch'
 assert encrypted.startswith(MAGIC),'Unexpected private-input container'
 secret=key();tag=encrypted[len(MAGIC):len(MAGIC)+32];cipher=encrypted[len(MAGIC)+32:]
 assert hmac.compare_digest(tag,hmac.new(mac_key(secret),cipher,hashlib.sha256).digest()),'Private-input authentication failed'
 plain=crypt(cipher,secret,True);assert sha(plain)==args.plaintext_sha256,'Plaintext input SHA mismatch'
 assert not args.output.exists(),'Refuse to merge private input into existing directory'
 with tarfile.open(fileobj=io.BytesIO(plain),mode='r:gz') as archive:
  members=archive.getmembers();names=[m.name for m in members]
  expected={f'RecompiledFuncs/{n}' for n in EXPECTED}|{'rsp/aspMain.cpp','manifest.json'}
  assert len(names)==len(set(names)) and set(names)==expected,'Input tar must contain exactly219native+RSP+manifest'
  assert all(m.isfile() and not m.issym() and not m.islnk() and 0<m.size<128*1024*1024 for m in members),'Unsafe private-input member'
  payloads={m.name:archive.extractfile(m).read() for m in members}
  manifest=json.loads(payloads['manifest.json']);assert manifest['schema']==1 and manifest['version']=='0.35.5' and manifest['sourceCommit']==args.source_commit
  assert set(manifest['files'])==expected-{'manifest.json'} and manifest['generatedCount']==COUNT
  assert manifest['originalRomIncluded'] is False and manifest['preparedContentIncluded'] is False and manifest['saveIncluded'] is False
  assert manifest['contentSha256']==CONTENT and manifest['contentFnv64']=='5E83770716DC3E31'
  assert all(sha(payloads[n])==digest for n,digest in manifest['files'].items())
  assert sha(canonical(manifest['generatedNormalizedSha256']))==manifest['generatedManifestSha256']
  for name in EXPECTED:assert sha(payloads['RecompiledFuncs/'+name].decode('utf-8').replace('\r\n','\n').encode())==manifest['generatedNormalizedSha256'][name]
 args.output.mkdir(parents=True)
 for name,payload in payloads.items():
  dest=args.output/name;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(payload);dest.chmod(0o600)
 receipt={'bundlePlaintextSha256':sha(plain),'encryptedBundleSha256':sha(encrypted),'manifestSha256':sha(payloads['manifest.json']),'sourceCommit':args.source_commit,'originalRomIncluded':False,'preparedContentIncluded':False,'saveIncluded':False}
 (args.output/'transport-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(receipt,indent=2))

def seal_file(args,decrypt=False):
 raw=args.input.read_bytes();secret=key()
 if decrypt:
  assert raw.startswith(MAGIC) and hmac.compare_digest(raw[len(MAGIC):len(MAGIC)+32],hmac.new(mac_key(secret),raw[len(MAGIC)+32:],hashlib.sha256).digest()),'Authentication failed'
  result=crypt(raw[len(MAGIC)+32:],secret,True)
 else:
  cipher=crypt(raw,secret);result=MAGIC+hmac.new(mac_key(secret),cipher,hashlib.sha256).digest()+cipher
 assert not args.output.exists(),'Do not overwrite an output'
 args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_bytes(result)
 print(json.dumps({'outputSha256':sha(result),'inputSha256':sha(raw),'decrypted':decrypt}))

def main():
 p=argparse.ArgumentParser(description=__doc__);sub=p.add_subparsers(dest='command',required=True)
 create=sub.add_parser('pack');create.add_argument('--root',type=Path,required=True);create.add_argument('--native',type=Path,required=True);create.add_argument('--rsp',type=Path,required=True);create.add_argument('--generated-manifest',type=Path,required=True);create.add_argument('--rsp-normalized-sha256',required=True);create.add_argument('--source-commit',required=True);create.add_argument('--output',type=Path,required=True)
 extract=sub.add_parser('unpack');extract.add_argument('--input',type=Path,required=True);extract.add_argument('--output',type=Path,required=True);extract.add_argument('--encrypted-sha256',required=True);extract.add_argument('--plaintext-sha256',required=True);extract.add_argument('--source-commit',required=True)
 for name in ('seal','open'):
  operation=sub.add_parser(name);operation.add_argument('--input',type=Path,required=True);operation.add_argument('--output',type=Path,required=True)
 a=p.parse_args()
 if a.command=='pack':pack(a)
 elif a.command=='unpack':unpack(a)
 else:seal_file(a,a.command=='open')
if __name__=='__main__':main()
