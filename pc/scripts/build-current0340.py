"""Configure the current native engine from locally prepared release sources."""
from pathlib import Path
import argparse,subprocess,os
p=argparse.ArgumentParser();p.add_argument('--platform',choices=['windows','linux'],required=True);p.add_argument('--sdl2-dir',type=Path);p.add_argument('--jobs',type=int,default=2);a=p.parse_args()
root=Path(__file__).resolve().parents[2];pc=root/'pc';generated=pc/'build/special-port'
assert (generated/'RecompiledFuncs/funcs.h').is_file(),'Run prepare-content.py first'
assert (generated/'content.z64').is_file(),'Prepared local content is missing'
assert 1<=a.jobs<=64
build=pc/('build/native-windows0340' if a.platform=='windows' else 'build/native-linux0340');runtime=pc/'build/native-special'
cc,cxx=('clang-cl','clang-cl') if a.platform=='windows' else ('clang-15','clang++-15')
cmd=['cmake','-S',str(pc/'dinotale'),'-B',str(build),'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_C_COMPILER='+cc,'-DCMAKE_CXX_COMPILER='+cxx,'-DWORKSHOP_DIR='+str(pc),'-DDINO_RECOMP_DIR='+str(generated/'RecompiledFuncs'),'-DWORKSHOP_CONTENT_ROM='+str(generated/'content.z64'),'-DWORKSHOP_RUNTIME_OUTPUT='+str(runtime),'-DWORKSHOP_SPECIAL_CONTENT=ON','-DWORKSHOP_HYBRID_FINDS=ON','-DWORKSHOP_NATIVE_COOP=ON']
if a.platform=='linux':
 cmd+=['-DCMAKE_BUILD_RPATH_USE_ORIGIN=ON']
 dxc=pc/'dinotale/lib/rt64/src/contrib/dxc/bin/x64/dxc-linux'
 if dxc.exists():dxc.chmod(dxc.stat().st_mode|0o111)
if a.sdl2_dir:cmd+=['-DSDL2_DIR='+str(a.sdl2_dir.resolve())]
subprocess.run(cmd,check=True);subprocess.run(['cmake','--build',str(build),'--target','DinoTale','--parallel',str(a.jobs)],check=True)
print('Native runtime and MusicRender0310 built; packaging/import metadata must match these executable hashes.')
