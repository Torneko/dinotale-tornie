"""Windows/stdlib fixture checks for authenticated native-input transport; no real private inputs."""
from pathlib import Path
from types import SimpleNamespace
import contextlib,hashlib,hmac,importlib.util,io,json,os,secrets,tarfile,tempfile,uuid
R=Path(__file__).resolve().parents[3];source=Path(__file__).with_name('private-inputs0355.py')
spec=importlib.util.spec_from_file_location('ci_transport0355',source);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
O=R/('pc/build/linux-mint-0355/ci-transport-selftest-'+uuid.uuid4().hex[:12]);O.mkdir(parents=True,exist_ok=False)
secret_before=os.environ.get('DINOTALE_CI_PRIVATE_KEY');test_secret=secrets.token_hex(32);os.environ['DINOTALE_CI_PRIVATE_KEY']=test_secret
checks=[]
def rejected(label,action):
 try:action()
 except (AssertionError,ValueError,KeyError):checks.append(label);return
 raise AssertionError('Expected rejection: '+label)
try:
 with tempfile.TemporaryDirectory(prefix='dinotale0355-transport-fixture-') as temp:
  base=Path(temp);native=base/'native';native.mkdir()
  for name in m.EXPECTED:(native/name).write_text('// synthetic host transport fixture '+name+'\n',encoding='utf-8')
  rsp=base/'aspMain.cpp';rsp.write_text('// synthetic RSP transport fixture\n');frozen={n:m.normalized(native/n) for n in sorted(m.EXPECTED)};freeze=base/'frozen.json';freeze.write_text(json.dumps(frozen))
  encrypted=base/'fixture.enc';pack=SimpleNamespace(root=base,native=native,rsp=rsp,generated_manifest=freeze,rsp_normalized_sha256=m.normalized(rsp),source_commit='1'*40,output=encrypted)
  with contextlib.redirect_stdout(io.StringIO()):m.pack(pack)
  receipt=json.loads(encrypted.with_suffix('.enc.json').read_text());extract=SimpleNamespace(input=encrypted,output=base/'unpacked',encrypted_sha256=receipt['encryptedBundleSha256'],plaintext_sha256=receipt['bundlePlaintextSha256'],source_commit='1'*40)
  with contextlib.redirect_stdout(io.StringIO()):m.unpack(extract)
  assert all((extract.output/'RecompiledFuncs'/n).read_bytes()==(native/n).read_bytes() for n in m.EXPECTED);assert (extract.output/'rsp/aspMain.cpp').read_bytes()==rsp.read_bytes();checks.append('actual-OpenSSL-encrypt-decrypt-exact219-plus-RSP')
  duplicate=SimpleNamespace(**vars(extract));rejected('never-merge-existing-private-directory',lambda:m.unpack(duplicate))
  tampered=base/'tampered.enc';raw=bytearray(encrypted.read_bytes());raw[-1]^=1;tampered.write_bytes(raw);args=SimpleNamespace(**vars(extract));args.input=tampered;args.output=base/'tampered-output';args.encrypted_sha256=m.sha(raw);rejected('MAC-rejects-changed-ciphertext-even-with-matching-outer-SHA',lambda:m.unpack(args));assert not args.output.exists()
  args=SimpleNamespace(**vars(extract));args.output=base/'wrong-hash';args.encrypted_sha256='0'*64;rejected('wrong-encrypted-SHA-refused',lambda:m.unpack(args));assert not args.output.exists()
  args=SimpleNamespace(**vars(extract));args.output=base/'wrong-key';os.environ['DINOTALE_CI_PRIVATE_KEY']=secrets.token_hex(32);rejected('wrong-secret-refused-before-decrypt',lambda:m.unpack(args));assert not args.output.exists();os.environ['DINOTALE_CI_PRIVATE_KEY']=test_secret
  args=SimpleNamespace(**vars(extract));args.output=base/'wrong-commit';args.source_commit='2'*40;rejected('wrong-frozen-source-commit-refused',lambda:m.unpack(args));assert not args.output.exists()
  (native/'unapproved.z64').write_bytes(b'not-a-ROM');rejected('unapproved-input-filename-refused',lambda:m.pack(pack));(native/'unapproved.z64').unlink()
  # Authenticated attacker-controlled tar traversal must still be rejected before extraction.
  cipher=encrypted.read_bytes()[len(m.MAGIC)+32:];plain=m.crypt(cipher,test_secret,True);payload=io.BytesIO()
  with tarfile.open(fileobj=io.BytesIO(plain),mode='r:gz') as prior,tarfile.open(fileobj=payload,mode='w:gz') as archive:
   for member in prior.getmembers():
    content=prior.extractfile(member).read();entry=tarfile.TarInfo('../escape' if member.name=='rsp/aspMain.cpp' else member.name);entry.size=len(content);archive.addfile(entry,io.BytesIO(content))
  unsafe=payload.getvalue();cipher=m.crypt(unsafe,test_secret);bad=base/'unsafe.enc';bad.write_bytes(m.MAGIC+hmac.new(m.mac_key(test_secret),cipher,hashlib.sha256).digest()+cipher)
  args=SimpleNamespace(**vars(extract));args.input=bad;args.output=base/'unsafe-output';args.encrypted_sha256=m.sha(bad.read_bytes());args.plaintext_sha256=m.sha(unsafe);rejected('authenticated-tar-traversal-refused',lambda:m.unpack(args));assert not args.output.exists() and not (base/'escape').exists()
  clear=base/'diagnostic';clear.write_bytes(b'host synthetic diagnostic only');sealed=base/'diagnostic.enc';opened=base/'diagnostic.opened'
  with contextlib.redirect_stdout(io.StringIO()):m.seal_file(SimpleNamespace(input=clear,output=sealed));m.seal_file(SimpleNamespace(input=sealed,output=opened),True)
  assert clear.read_bytes()==opened.read_bytes();checks.append('encrypted-diagnostics-round-trip')
 report={'passed':True,'checks':len(checks),'cases':checks,'transportScriptSha256':m.sha(source.read_bytes()),'syntheticOnly':True,'realGeneratedSourcesOrRomUsed':False,'secretEmitted':False,'wslOrLinuxProcessLaunched':False}
finally:
 if secret_before is None:os.environ.pop('DINOTALE_CI_PRIVATE_KEY',None)
 else:os.environ['DINOTALE_CI_PRIVATE_KEY']=secret_before
(O/'verification.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'passed':report['passed'],'checks':report['checks'],'report':str(O/'verification.json')},indent=2))
