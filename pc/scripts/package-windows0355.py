"""Stage the qualified0355 Windows programs as a fresh public test archive.

No compilation, game launch, personal profile access or export overwrite occurs.
"""
from pathlib import Path, PurePosixPath
from datetime import datetime, timezone
import argparse,hashlib,importlib.util,json,os,re,shutil,subprocess,uuid,zipfile

PC=Path(__file__).resolve().parents[1];ROOT=PC.parent
spec=importlib.util.spec_from_file_location('old_windows0340',Path(__file__).with_name('package-windows0340.py'));old=importlib.util.module_from_spec(spec);spec.loader.exec_module(old)
need=old.need;sha=old.sha;read=old.read;write=old.write;copy=old.copy
ROM_SHA=old.ROM_SHA;CONTENT_SHA=old.CONTENT_SHA;FRESH_SHA=old.FRESH_SHA
ENGINE_SHA='08ffb814ce95703f248ec50000f7ad9b6c3023ff572c74150a2633ff650c5d75'
WORKER_SHA='2b7498e161eaa5638321c07e36a433477335f202335f7e23f48efedffe8e55e6'
EDITOR_SHA='6d9717aa97e6c87e69bf0e23901b7520d0a3decd79ac7bf8ca7c90b4cc5faa16'
BOOT_SHA='53726155ed4eacb10c509ab110451a3a397b71944eabe3a650d5a9723406c087'
compile_native=old.compile_native
RUNTIME_FILES=old.RUNTIME_FILES;CRT_FILES=old.CRT_FILES;WORKSHOP_DIR=old.WORKSHOP_DIR
ROOT_EXTRA={'Bonus-en-test.cmd','Tester-Bonus.ps1','SPOILERS-OPTIONNELS.txt'}
VERSION='0.35.5-beta.1'

def allowed_file(name):return name in ROOT_EXTRA or old.allowed_file(name)
def features(meta):
 need(meta.get('version')=='0.35.5' and meta.get('soloYoshiColors')==18 and meta.get('editorSpecialEggCount')==12,'Wrong0355 capabilities')
 need(meta.get('bossChallengeSchema')==3 and meta.get('storyHardElementalPrerequisite') is False,'Wrong Boss/Hard prerequisite schema')
 need(meta.get('storyHardPublicGate')=='all16owned-and-genuineBoss16','Wrong original16 Hard gate')
 need(meta.get('bossChallengeCount')==18,'Wrong0355 Boss challenge count')
 for key in ['editorElementalPlacements','unknownYoshiMenuIdentitiesHidden','titleArtworkUserAccepted','bossCheckpointOrdinaryPlacementReplay','bossCheckpointChainedRetryReplay','bossInitialRaw52CacheVerified','bossCheckpoint0Cold162TransitionVerified','bossChallengeTitleUsesCatalogCount']:need(meta.get(key) is True,'Missing feature '+key)
def validate(folder):
 folder=Path(folder);files=sorted(p for p in folder.rglob('*') if p.is_file());need(files,'Missing package')
 for p in files:
  name=p.relative_to(folder).as_posix();need(allowed_file(name),'Unapproved file '+name);need(not p.is_symlink(),'Linked package input')
  need(p.suffix.lower() not in {'.z64','.n64','.v64','.eep','.rdram','.wav','.pcm','.pdb','.log','.bak','.dmp'},'Private/generated file '+name)
  need(p.read_bytes()[:4] not in old.MAGICS,'ROM magic '+name)
  if p.suffix.lower()=='.bin':need(name in {'pc/profiles/content-personal/saves/yoshisstory.n64.us.bin','pc/save-imports/content-default.bin'},'Unexpected binary data')
  if p.suffix.lower() in {'.json','.ps1','.cmd','.txt'}:need(not re.search(rb'canti|OneDrive|D:[/\\]Yoshi-Story|C:[/\\]Users',p.read_bytes(),re.I),'Personal path/name in public text '+name)
 actual={p.relative_to(folder).as_posix():sha(p) for p in files if p.name!='contenu-sha256.json'};need(read(folder/'contenu-sha256.json')==actual,'Inventory mismatch')
 profile=folder/'pc/profiles/content-personal';need(sha(profile/'saves/yoshisstory.n64.us.bin')==sha(folder/'pc/save-imports/content-default.bin')==FRESH_SHA,'Factory save differs')
 progress=read(profile/'menu-progress.json');need(progress.get('valid') is True and all(progress.get(k)==0 for k in ['storyMask','trialMask','extraYoshiMask','trialEggs']),'Forced progress')
 need(read(profile/'sound.json')=={'main_volume':3},'Volume differs')
 game=folder/'pc/build/native-special/DinoTale.exe';worker=game.with_name('MusicRender0310.exe');editor=folder/WORKSHOP_DIR/'YoshiStory-AtelierPC.exe'
 need(sha(game)==ENGINE_SHA and sha(worker)==WORKER_SHA and sha(editor)==EDITOR_SHA and sha(folder/'DinoTale.exe')==BOOT_SHA,'Qualified program changed')
 meta=read(folder/'pc/portable-package.json');patch=read(folder/'pc/distribution/content-patch.json');caps=read(game.with_name('engine-build.json'));features(caps)
 need(meta['version']==VERSION and meta['engineSourceVersion']=='0.35.5' and meta['workshopVersion']=='0.35.5','False version')
 need(meta['freshSave'] is True and meta['personalProgressIncluded'] is False and meta['romIncluded'] is False and meta['testUnlocksForced'] is False,'Personal/forced state')
 need(meta['engineSha256']==ENGINE_SHA and meta['workshopSha256']==EDITOR_SHA and meta['menuMusicWorkerSha256']==WORKER_SHA,'Unbound programs')
 need(meta['workshopSelfContained'] and meta['volumePercent']==3 and meta['coopAndBossBattleDeferred'] and not meta['releaseReady'],'False qualification')
 need(patch['engineSha256']==ENGINE_SHA and patch['patchSha256']==sha(folder/'pc/distribution/content.bps') and patch['sourceSha256']==ROM_SHA and patch['targetSha256']==CONTENT_SHA,'Importer binding differs')
 ps=(folder/'Tester-Bonus.ps1').read_text(encoding='utf-8');need("Join-Path $root 'pc\\profiles\\bonus0353-qa-portable'" in ps and 'DinoTale-Recent-Personnel' not in ps and 'D:\\' not in ps and 'C:\\' not in ps,'Nonportable Bonus launcher')
 need(ENGINE_SHA in ps and WORKER_SHA in ps and "if (-not $PrepareOnly)" in ps,'Bonus hash/PrepareOnly guard differs')
 native=game.parent;bundled={p.name.lower():p for p in native.glob('*.dll')};deps={}
 for p in [game,worker,editor,folder/'DinoTale.exe',*bundled.values()]:
  imports=old.pe_imports(p);deps[p.relative_to(folder).as_posix()]=imports
  for name in imports:need(name in bundled or name in old.SYS_DLLS or name.startswith(('api-ms-win-','ext-ms-win-')),'Unresolved DLL '+name)
 for p in (native/'assets').rglob('*'):
  if p.is_file():need(sha(p)==sha(profile/'assets'/p.relative_to(native/'assets')),'Bootstrap asset differs')
 return {'fileCount':len(files),'bytesUncompressed':sum(p.stat().st_size for p in files),'peImports':deps,'freshSave':True,'romIncluded':False,'volumePercent':3,'qualifiedProgramsUnchanged':True,'selfContainedWorkshop':True,'soloYoshiColors':18,'editorSpecialEggCount':12,'original16HardGate':True}

def main():
 global ENGINE_SHA,WORKER_SHA
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,required=True);ap.add_argument('--source-tag',default='v0.35.5-beta.1');ap.add_argument('--runtime',type=Path,default=PC/'build/native-bonus-0.35.5-retry2');ap.add_argument('--engine-sha256',default=ENGINE_SHA);ap.add_argument('--worker-sha256',default=WORKER_SHA);args=ap.parse_args();ENGINE_SHA=args.engine_sha256;WORKER_SHA=args.worker_sha256;need(re.fullmatch(r'[0-9a-f]{64}',ENGINE_SHA) and re.fullmatch(r'[0-9a-f]{64}',WORKER_SHA),'Explicit reviewed hashes required');need(os.name=='nt','Windows only');args.output=args.output.resolve();need(args.output.is_relative_to((ROOT/'dist').resolve()) and re.fullmatch(r'Windows-0[.]35[.]5-Public-Beta-[0-9]{2}',args.output.name) and not args.output.exists(),'New explicit output required')
 need(re.fullmatch(r'[A-Za-z0-9._-]+',args.source_tag),'Invalid tag')
 runtime=args.runtime.resolve();need(runtime.is_relative_to((PC/'build').resolve()) and runtime.name.startswith('native-bonus-0.35.5-'),'Use a separate qualified0355 frozen runtime');editor=PC/'build/workshop-0.35.5-discovery-candidate/YoshiStory-AtelierPC.exe';oldstage=ROOT/'dist/Windows-0.34.0-Public-Beta-02/DinoTale-Tornie-0.34.0-beta.1-Windows-x64';seed=PC/'profiles/portable-fresh-seed-20261028/saves/yoshisstory.n64.us.bin'
 protected={};inputs={}
 for p in [*runtime.rglob('*'),editor,editor.parent/'review-workshop.json',oldstage/'DinoTale.exe',*[(oldstage/'pc/build/native-special')/n for n in CRT_FILES],*[(oldstage/'Licences')/n for n in os.listdir(oldstage/'Licences')]]:
  if p.is_file():protected[str(p)]=sha(p)
 review=read(runtime/'review-engine.json');need(sha(runtime/'DinoTale.exe')==ENGINE_SHA and sha(runtime/'MusicRender0310.exe')==WORKER_SHA,'Qualifiedruntimehashpair differs');need(all(sha(runtime/n)==h for n,h in review['files'].items()),'Runtime review differs');need(sha(editor)==EDITOR_SHA and read(editor.parent/'review-workshop.json')['selfContained'],'Editor qualification differs');need(sha(seed)==FRESH_SHA and sha(oldstage/'DinoTale.exe')==BOOT_SHA,'Seed/bootstrap differs');features(read(runtime/'engine-build.json'))
 args.output.mkdir();report=PC/'build/publication-0355'/('windows-'+uuid.uuid4().hex[:12]);report.mkdir(parents=True);write(report/'protected-before.json',protected)
 folder=args.output/('DinoTale-Tornie-'+VERSION+'-Windows-x64');folder.mkdir();engine=folder/'pc/build/native-special';profile=folder/'pc/profiles/content-personal'
 for n in RUNTIME_FILES:copy(runtime/n,engine/n,inputs)
 for n in CRT_FILES:copy(oldstage/'pc/build/native-special'/n,engine/n,inputs)
 copy(editor,folder/WORKSHOP_DIR/editor.name,inputs);copy(oldstage/'DinoTale.exe',folder/'DinoTale.exe',inputs)
 caps=read(runtime/'engine-build.json');caps.pop('sourceBuildReport',None);caps.update(personalProfileInstalled=False,publicationAuthorized=True,releaseReady=False,manualUserAccepted=False,qualification='Windows0355 qualified native build plus targeted CPU checks; portable public test prerelease. Human gameplay of recent changes remains pending.',validationScope='Qualified0355 native build and targeted native CPU checks; package checks are separate. Natural game/GPU validation of recent changes remains pending.')
 write(engine/'engine-build.json',caps)
 for p in sorted((runtime/'assets').rglob('*')):
  if p.is_file():copy(p,engine/'assets'/p.relative_to(runtime/'assets'),inputs);copy(p,profile/'assets'/p.relative_to(runtime/'assets'),inputs)
 for n in ['peche','mure','bleuet','poire-verte','prune','kiwi']:copy(PC/'assets/editor'/(n+'-24x21.png'),folder/'pc/assets/editor'/(n+'-24x21.png'),inputs)
 copy(PC/'upstream-lock.json',folder/'pc/upstream-lock.json',inputs)
 dist=PC/'build/publication-0150/distribution';patch=read(dist/'content-patch.json');need(patch['sourceSha256']==ROM_SHA and patch['targetSha256']==CONTENT_SHA and patch['patchSha256']==sha(dist/'content.bps'),'BPS differs');copy(dist/'content.bps',folder/'pc/distribution/content.bps',inputs);patch.update(version=VERSION,engineSha256=ENGINE_SHA);write(folder/'pc/distribution/content-patch.json',patch)
 copy(seed,profile/'saves/yoshisstory.n64.us.bin',inputs);copy(seed,folder/'pc/save-imports/content-default.bin',inputs)
 write(folder/'pc/save-imports/content-import.json',{'seedSha256':FRESH_SHA,'testUnlocksUsed':False,'earnedUnlocksPreserved':True,'earnedYoshis':[]})
 write(profile/'menu-progress.json',{'version':1,'valid':True,'sourceSaveSha256':FRESH_SHA,'storyMask':0,'trialMask':0,'extraYoshiMask':0,'trialEggs':0})
 for n in ['general.json','graphics.json']:copy(oldstage/'pc/profiles/content-personal'/n,profile/n,inputs)
 write(profile/'sound.json',{'main_volume':3});write(profile/'book-menu.json',{'version':1,'language':'fr'});(profile/'portable.txt').write_text('',encoding='utf-8')
 for n in ['Jouer.cmd','Atelier.cmd']:copy(oldstage/n,folder/n,inputs)
 helper_template=PC/'scripts/portable-bonus0355.ps1';inputs[str(helper_template)]=sha(helper_template);helper=helper_template.read_text(encoding='utf-8');helper=helper.replace('08ffb814ce95703f248ec50000f7ad9b6c3023ff572c74150a2633ff650c5d75',ENGINE_SHA).replace('2b7498e161eaa5638321c07e36a433477335f202335f7e23f48efedffe8e55e6',WORKER_SHA);(folder/'Tester-Bonus.ps1').write_text(helper,encoding='utf-8',newline='\n');(folder/'Bonus-en-test.cmd').write_bytes(b'@echo off\r\npowershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Tester-Bonus.ps1"\r\nif errorlevel 1 pause\r\n')
 for p in (oldstage/'Licences').iterdir():
  if p.is_file() and p.name!='Sources.txt':copy(p,folder/'Licences'/p.name,inputs)
 (folder/'Licences/Sources.txt').write_text('DinoTale Tornie '+VERSION+'\nCorresponding source: https://github.com/Torneko/dinotale-tornie/tree/'+args.source_tag+'\nWindows0355 qualified native engine and0355 Workshop; exact qualified programs copied without rebuilding.\nPinned dependency versions and local hook patches are in the corresponding source tree.\nNo original or prepared game image is included.\n',encoding='utf-8')
 fr='''DinoTale Tornie 0.35.5-beta.1 — essais publics Windows 10/11 x64

Extraire TOUT le dossier, puis ouvrir DinoTale.exe. Atelier.cmd ouvre l’éditeur.
Au premier lancement, sélectionner une ROM originale USA de Yoshi’s Story.
Les formats .z64, .v64 et .n64 sont reconnus. Aucun émulateur, .NET, Python ou
compilateur à installer. La préparation reste locale ; aucune ROM incluse.
Sauvegarde vierge, monnaies à zéro, volume initial 3 %.

Cette version comprend les dernières découvertes, améliorations de l’éditeur,
récompenses et corrections de reprise. Les identités non découvertes restent
masquées. La coop et le Boss Battle restent en développement. Les vérifications
CPU et de paquet ne remplacent pas les essais en jeu des nouveautés.

Bonus-en-test.cmd prépare une partie Bonus séparée avec les droits déjà acquis.
Fermer le jeu avant d’utiliser ce lanceur. Aucun déblocage n’est accordé ; les
progrès Bonus restent dans pc/profiles/bonus0353-qa-portable, sans fusion inverse.

Pour conserver une ancienne partie : sauvegarder l’ancien dossier et extraire
cette version séparément. Copier TOUT pc/profiles/content-personal vers la nouvelle
copie, ainsi que pc/content pour les créations. Ne pas remplacer les anciennes
données par la sauvegarde vierge. Les réussites Bonus éventuelles sont séparées.
Pour découvrir les nouveautés sans révélation, ne pas ouvrir le fichier facultatif
SPOILERS-OPTIONNELS.txt. Guide : https://torneko.github.io/dinotale-tornie/
'''
 en='''DinoTale Tornie 0.35.5-beta.1 — Windows 10/11 x64 public test prerelease

Extract the COMPLETE folder, then open DinoTale.exe. Atelier.cmd opens the editor.
On first launch, select an original USA Yoshi’s Story ROM (.z64/.v64/.n64).
No emulator, .NET, Python or compiler installation is required. Content preparation
is local; no ROM is included. Factory-empty save, zero currency, initial volume 3%.

Includes recent discoveries, editor improvements, rewards and checkpoint fixes.
Undiscovered identities stay hidden. Co-op and Boss Battle remain in development.
CPU and package checks do not replace natural gameplay tests of recent changes.

Bonus-en-test.cmd prepares a separate Bonus profile using earned rights only.
Close the game first. Bonus progress stays in pc/profiles/bonus0353-qa-portable;
no unlocks are granted and no Bonus progress is merged into the normal profile.

Migration: back up the old folder and extract this version separately. Copy ALL
pc/profiles/content-personal into the new copy and pc/content for creations.
Never overwrite old progress with the factory seed. Bonus progress is separate.
SPOILERS-OPTIONNELS.txt is an optional unlock-information file.
Guide: https://torneko.github.io/dinotale-tornie/index-en.html
'''
 (folder/'LIRE-MOI.txt').write_text(fr,encoding='utf-8');(folder/'README.txt').write_text(en,encoding='utf-8');(folder/'SPOILERS-OPTIONNELS.txt').write_text('Informations facultatives / Optional spoilers\n\nLes détails de découvertes et de déblocage sont dans le guide facultatif :\nhttps://torneko.github.io/dinotale-tornie/guide.html\nEnglish: https://torneko.github.io/dinotale-tornie/guide-en.html\nLes objectifs normaux restent nécessaires. Le lanceur Bonus ne donne aucun droit.\n',encoding='utf-8')
 write(folder/'pc/portable-package.json',{'format':1,'version':VERSION,'distributionVersion':VERSION,'platform':'Windows x64','channel':'experimental-public-prerelease','engineSourceVersion':'0.35.5','engineRevision':caps.get('privateRevision','0355'),'engineSha256':ENGINE_SHA,'workshopVersion':'0.35.5','workshopSha256':EDITOR_SHA,'workshopSelfContained':True,'workshopRuntime':'.NET9.0.5 Desktop win-x64 bundled','workshopCompatibilityDirectory':'build/workshop-0.21.0','launcherSha256':BOOT_SHA,'menuMusicWorkerSha256':WORKER_SHA,'freshSave':True,'personalProgressIncluded':False,'testUnlocksForced':False,'volumePercent':3,'romIncluded':False,'originalRomRequired':True,'saveFolder':'pc/profiles/content-personal','coopAndBossBattleDeferred':True,'manualUserAccepted':False,'releaseReady':False,'publicationAuthorized':True,'sourceTag':args.source_tag,'soloYoshiColors':18,'editorSpecialEggCount':12,'storyHardElementalPrerequisite':False,'bonusPreview':True,'bonusProfile':'pc/profiles/bonus0353-qa-portable','pendingManualTests':['Natural gameplay of recent solo discoveries and powers','Editor and complete Bonus routes','Co-op and Boss Battle remain unfinished']})
 write(folder/'pc/windows-runtime-libraries.json',{'schema':1,'bundled':{n:sha(engine/n)for n in CRT_FILES+('SDL2.dll','dxcompiler.dll','dxil.dll')},'systemDependencies':'Windows10/11 x64 graphics/OS drivers; D3D12 by default','workshop':'Self-contained .NET9 Desktop win-x64','engineAndWorkerCopiedUnmodified':True})
 write(folder/'contenu-sha256.json',{p.relative_to(folder).as_posix():sha(p)for p in sorted(folder.rglob('*'))if p.is_file()});result=validate(folder)
 archive=args.output/(folder.name+'.zip')
 with zipfile.ZipFile(archive,'x',zipfile.ZIP_DEFLATED,compresslevel=6)as z:
  for p in sorted(folder.rglob('*')):
   if p.is_file():z.write(p,p.relative_to(args.output))
 with zipfile.ZipFile(archive)as z:need(z.testzip() is None,'ZIP CRC error')
 need(all(sha(Path(p))==h for p,h in inputs.items()),'Source changed during packaging');need(all(sha(Path(p))==h for p,h in protected.items()),'Frozen input changed')
 write(report/'sources.json',inputs);write(report/'protected-after.json',protected);result.update(archive=str(archive),archiveSha256=sha(archive),archiveBytes=archive.stat().st_size,package=str(folder),report=str(report),createdUtc=datetime.now(timezone.utc).isoformat(),nativeGameRebuilt=False,gameOrGpuStarted=False,frozenSourcesUnchanged=True,sourceTag=args.source_tag,version=VERSION,engineVersion='0.35.5',runtimeSource=str(runtime),engineSha256=ENGINE_SHA,workerSha256=WORKER_SHA,portableBonusHelperSha256=sha(folder/'Tester-Bonus.ps1'))
 write(args.output/'package-report.json',result);write(report/'package-report.json',result);print(json.dumps({k:result[k]for k in ['archive','archiveSha256','archiveBytes','fileCount','report','gameOrGpuStarted']},indent=2))
if __name__=='__main__':main()
