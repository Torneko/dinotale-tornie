"""Exercise the new0355 POSIX Bonus launcher with disposable fake processes only.

Runs on Linux after Root authorizes its CPU slot; never opens the native game,
GPU, browser, physical audio, ROM or a personal profile. Synthetic ROM bytes are
only a temporary hash-binding fixture. The real helper is used unchanged except
for its four expected input SHA placeholders.
"""
from pathlib import Path
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
import uuid

ROOT = Path(__file__).resolve().parents[2]
TEMPLATE = ROOT / 'pc/scripts/personal-bonus-linux0355.sh'
checks = 0


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def need(value, message):
    global checks
    checks += 1
    if not value:
        raise AssertionError(message)


def inventory(folder):
    return {str(path.relative_to(folder)): sha(path) for path in folder.rglob('*')
            if path.is_file() and not path.is_symlink()}


def put(path, content):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(content if isinstance(content, bytes) else content.encode('utf-8'))


def main():
    need(sys.platform.startswith('linux'), 'Run this shell verification on Linux, only after Root GO')
    source = TEMPLATE.read_text(encoding='utf-8')
    with tempfile.TemporaryDirectory(prefix='bonus0355-') as temporary:
        base = Path(temporary) / "Copie accents é ' $()"
        base.mkdir()
        normal = base / 'pc/profiles/content-personal'
        preview = base / 'pc/profiles/bonus0353-qa-linux'
        runtime = base / 'pc/build/native-special'
        fake_game = '#!/bin/sh\nprintf \'%s\\n\' "$PWD" "$@" > launch-evidence.txt\n'
        put(runtime / 'DinoTale', fake_game)
        os.chmod(runtime / 'DinoTale', 0o755)
        put(runtime / 'MusicRender0310', 'synthetic worker; not executed')
        put(runtime / 'assets/font.txt', 'synthetic asset')
        put(runtime / 'workshop-content.bin', 'synthetic prepared content')
        put(normal / 'yoshisstory.n64.us.z64', bytes(1024))
        put(normal / 'saves/yoshisstory.n64.us.bin', bytes([0xA5]) * 2048)
        put(normal / 'pc-boss-challenges.json', '{"schema":3,"mask18":65536,"counts18":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0],"runs":[{"token":"00000000000000000000000000000001","color":16}]}')
        put(normal / 'pc-metal-yoshis.json', '{"schema":1,"foundMask":1,"storyAliveMask":0}')
        put(normal / 'pc-hybrid-finds.json', '{"version":1,"foundMask":5,"legacyMigrated":true}')
        put(normal / 'sound.json', '{"main_volume":3}')
        elemental = normal / 'pc-elemental-yoshis.json'
        put(elemental, b'\xef\xbb\xbf{ "storyAliveMask":1, "schema":1, "foundMask":1 }\r\n')
        put(preview / 'pc-bonus-session0353.json', 'existing Bonus journal sentinel')
        put(preview / 'saves/yoshisstory.n64.us.bonus0353.bin', b'existing Bonus EEPROM sentinel')
        launcher = base / 'Bonus-en-test.sh'
        bound = source
        for key, path in [('ENGINE', runtime / 'DinoTale'), ('WORKER', runtime / 'MusicRender0310'),
                          ('ROM', normal / 'yoshisstory.n64.us.z64'), ('CONTENT', runtime / 'workshop-content.bin')]:
            bound = bound.replace('@' + key + '_SHA256@', sha(path))
        put(launcher, bound)
        os.chmod(launcher, 0o755)
        bonus = {name: sha(preview / name) for name in ['pc-bonus-session0353.json', 'saves/yoshisstory.n64.us.bonus0353.bin']}

        def run(want=True, prepare=True):
            result = subprocess.run(['/bin/sh', str(launcher)] + (['--prepare-only'] if prepare else []),
                                    cwd=temporary, text=True, capture_output=True, timeout=30)
            need((result.returncode == 0) == want, result.stdout + result.stderr)
            return result

        before = inventory(normal)
        run()
        need(inventory(normal) == before, 'Normal profile mutated')
        need(not (preview / 'launch-evidence.txt').exists(), 'Prepare-only launched a process')
        for name, digest in before.items():
            need(sha(preview / name) == digest, 'Initial snapshot differs: ' + name)
        need((preview / 'bonus-complete0353.txt').read_bytes() == b'DinoTale Bonus0353 complete route preview only\n', 'Wrong marker/protocol')
        need((preview / 'portable.txt').read_bytes() == b'', 'Portable marker has content or BOM')
        for raw in [b'{"schema":2,"foundMask":3,"storyAliveMask":1,"hybridAliveMask":2}',
                    b'{"schema":1,"foundMask":0,"storyAliveMask":0}',
                    b'\xef\xbb\xbf{ "hybridAliveMask":0,"foundMask":3,"schema":2,"storyAliveMask":3 }\r\n']:
            put(elemental, raw)
            before = inventory(normal)
            run()
            need((preview / elemental.name).read_bytes() == raw, 'Elemental journal rewritten rather than copied exactly')
            need(inventory(normal) == before, 'Genuine journal refresh mutated the source')
        valid = elemental.read_bytes()
        invalid = [b'{"schema":1,"foundMask":0,"storyAliveMask":0,"schema":1}',
                   b'{"schema":1,"foundMask":0,"storyAliveMask":0,"other":0}',
                   b'{"schema":2,"foundMask":1,"storyAliveMask":0,"hybridAliveMask":2}',
                   b'{"schema":2,"foundMask":0,"storyAliveMask":0}',
                   b'{"schema":1,"foundMask":4,"storyAliveMask":0}',
                   b'{"schema":1,"foundMask":1.0,"storyAliveMask":0}',
                   b'{"schema":1,"foundMask":true,"storyAliveMask":0}',
                   b'{"schema":1,"foundMask":-0,"storyAliveMask":0}',
                   b'{"schema":1,"foundMask":0,"storyAliveMask":0}\x00',
                   b'\xff' + valid, valid.decode('utf-8-sig').encode('utf-16'), b' ' * 4097]
        preview_before = inventory(preview)
        for raw in invalid:
            put(elemental, raw)
            normal_before = inventory(normal)
            run(False)
            need(inventory(preview) == preview_before, 'Invalid journal modified the existing Bonus preview')
            need(inventory(normal) == normal_before, 'Invalid journal rejection modified source')
        put(elemental, valid)
        put(normal / 'pc-metal-yoshis.json', '{"schema":1,"foundMask":3,"storyAliveMask":0}')
        (normal / 'pc-hybrid-finds.json').unlink()
        before = inventory(normal)
        run()
        need(not (preview / 'pc-hybrid-finds.json').exists(), 'Obsolete ordinary snapshot retained')
        need(sha(normal / 'pc-metal-yoshis.json') == sha(preview / 'pc-metal-yoshis.json'), 'Genuine new ownership not refreshed')
        # A missing canonical journal must remove only its stale ordinary snapshot.
        elemental.unlink()
        before = inventory(normal)
        run()
        need(not (preview / elemental.name).exists(), 'Missing canonical elemental journal kept stale rights')
        put(elemental, valid)
        before = inventory(normal)
        run(prepare=False)
        event = (preview / 'launch-evidence.txt').read_text().splitlines()
        need(event == [str(preview), '--workshop-main-menu', '--workshop-bonus-finales0353', '1', '24'], 'Wrong CWD/Bonus0353 arguments')
        for name, digest in bonus.items():
            need(sha(preview / name) == digest, 'Existing Bonus progress changed: ' + name)
        rom = normal / 'yoshisstory.n64.us.z64'
        rom.rename(normal / 'missing-rom')
        run(False)
        (normal / 'missing-rom').rename(rom)
        outside = Path(temporary) / 'outside.json'
        put(outside, 'outside preserved')
        elemental.unlink()
        elemental.symlink_to(outside)
        run(False)
        need(outside.read_text() == 'outside preserved', 'Linked elemental source followed')
        elemental.unlink()
        put(elemental, valid)
        stale = preview / 'pc-stale.json'
        stale.symlink_to(outside)
        run(False)
        need(outside.read_text() == 'outside preserved', 'Linked stale destination followed')
        stale.unlink()
        assets = preview / 'assets'
        shutil.rmtree(assets)
        assets.symlink_to(Path(temporary), target_is_directory=True)
        run(False)
        assets.unlink()
        prepared = runtime / 'workshop-content.bin'
        prepared.rename(runtime / 'missing-content')
        run(False)
        (runtime / 'missing-content').rename(prepared)
        for filename in ('DinoTale', 'MusicRender0310'):
            binary = runtime / filename
            original = binary.read_bytes()
            put(binary, 'tampered')
            run(False)
            put(binary, original)
        need(not list(preview.glob('.snapshot.*')), 'Temporary snapshot leaked')
        need(inventory(normal) == before, 'Final normal profile changed')
        for name, digest in bonus.items():
            need(sha(preview / name) == digest, 'Final Bonus progress changed: ' + name)
    report = {'passed': True, 'checks': checks, 'actualShellLauncherSha256': sha(TEMPLATE),
              'fakeProcessOnly': True, 'normalSnapshotAndBossLedger3Preserved': True,
              'elementalSchemas1And2ExactBytesCopied': True, 'invalidUtf8OrSchemaRejectedBeforePreviewMutation': True,
              'existingBonus0353ProgressPreserved': True, 'actualPersonalRightsRefreshedWithoutGrant': True,
              'wrongRomLinkedSourceAndDestinationAndRuntimeRejected': True, 'prepareOnlyDoesNotLaunch': True,
              'spacesAccentsQuoteDollarPathsPassed': True, 'nativeGameGpuBrowserOrPersonalUsed': False}
    output = ROOT / ('pc/build/linux-mint-0355/bonus-launcher-' + uuid.uuid4().hex[:12])
    output.mkdir(parents=True)
    (output / 'verification.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({'report': str(output / 'verification.json'), **report}, indent=2))


if __name__ == '__main__':
    main()
