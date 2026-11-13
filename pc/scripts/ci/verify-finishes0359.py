"""CPU-only preflight for the frozen 0358 Workshop sources, before a native build.

Default: print a plan. --run: use a fresh fixture under pc/build/linux-finishes0359.
Never starts a browser, native game, WSL, remote job or a personal profile. The
existing 0358 HTTP test is executed unchanged except for exact AST path/host
bindings. This is a host-framework check, not a published Linux binary test.
"""
from __future__ import annotations

import argparse
import ast
import contextlib
import datetime
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time
import uuid

ROOT = Path(__file__).resolve().parents[3]
BASE = ROOT / 'pc/build/linux-finishes0359'
API_SOURCE = ROOT / 'pc/scripts/verify-atelier-thumbnails0358.py'
API_SOURCE_CANONICAL_LF_SHA = '1d7b032ebf4967b5f75d216f7d0c45ce943019c58a16f831094aaa7bae8c9ca0'


def need(value, message):
    if not value:
        raise AssertionError(message)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def canonical_lf_sha(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def source_inventory():
    found = {}
    for name in ('Core', 'WorkshopLinux', 'BonusThumbnailChecks'):
        base = ROOT / 'pc/workshop' / name
        for folder, subdirs, files in os.walk(base):
            subdirs[:] = [n for n in subdirs if n not in {'bin', 'obj', 'build'}]
            for name in files:
                path = Path(folder) / name
                if path.suffix in {'.cs', '.csproj', '.html'}:
                    found[path.relative_to(ROOT).as_posix()] = sha(path)
    for name in ('verify-atelier-thumbnails0358.py',
                 'check-linux-bonus-thumbnails0358.cjs',
                 'check-linux-bubble-replacement0358.cjs',
                 'publish-workshop-linux-ci0355.py', 'verify-linux-atelier0355.py'):
        path = ROOT / 'pc/scripts' / name
        found[path.relative_to(ROOT).as_posix()] = sha(path)
    return found


def legacy_publisher_contract():
    html = (ROOT / 'pc/workshop/WorkshopLinux/index.html').read_text(encoding='utf-8')
    program = (ROOT / 'pc/workshop/WorkshopLinux/Program.cs').read_text(encoding='utf-8')
    checks = [text in html for text in (
        'id<=0x4f3b', "'#dfb94b','#bdc8d6','#252836','#fafaf3']", '·12 œufs')]
    checks += [text in program for text in (
        'editor0340=true,editor0355=true', 'knownYoshiMask18=KnownYoshis0355.ReadProfile')]
    need(all(checks), 'Frozen0355 publisher source contract no longer matches')
    return {'allFiveSourceAssertionsPass': True,
            'publisherAndLegacyVerifierExecuted': False,
            'oldOutputDirectoryWritten': False,
            'legacyVerifierDoesNotCoverThumbnails': True}


def run_command(argv, label, out, env, timeout=240, cwd=None):
    log = out / (label + '.log')
    started = time.monotonic()
    with log.open('w', encoding='utf-8') as stream:
        completed = subprocess.run(argv, cwd=cwd or ROOT, env=env, stdout=stream,
                                   stderr=subprocess.STDOUT, timeout=timeout,
                                   creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
    need(completed.returncode == 0, label + ' failed; inspect ' + str(log))
    return {'command': [str(x) for x in argv], 'exitCode': completed.returncode,
            'seconds': round(time.monotonic() - started, 2),
            'cwd': str(cwd or ROOT), 'log': log.relative_to(out).as_posix(), 'logSha256': sha(log)}


def actual_http_test(app, out):
    """Keep all original assertions; change only three paths and host annotations."""
    original_raw_sha = sha(API_SOURCE)
    original_canonical_sha = canonical_lf_sha(API_SOURCE)
    need(original_canonical_sha == API_SOURCE_CANONICAL_LF_SHA,
         'Frozen0358 API driver canonical-LF content changed; review adaptation')
    tree = ast.parse(API_SOURCE.read_text(encoding='utf-8-sig'), filename=str(API_SOURCE))
    changed = {'R': 0, 'OUT': 0, 'APP': 0, 'creationflags': 0, 'reportHost': 0}
    values = {'R': ROOT, 'OUT': out / 'api-fixture', 'APP': app}

    class Bindings(ast.NodeTransformer):
        def visit_Assign(self, node):
            if len(node.targets) == 1 and isinstance(node.targets[0], ast.Name):
                name = node.targets[0].id
                if name in values:
                    changed[name] += 1
                    node.value = ast.Call(func=ast.Name(id='Path', ctx=ast.Load()),
                                          args=[ast.Constant(str(values[name]))], keywords=[])
                elif name == 'report':
                    need(isinstance(node.value, ast.Dict), 'API report binding differs')
                    for key, value in zip(node.value.keys, node.value.values):
                        if isinstance(key, ast.Constant) and key.value in (
                                'windowsHostedWorkshopLinuxApi', 'actualLinuxExecution'):
                            need(isinstance(value, ast.Constant), 'API host annotation differs')
                            value.value = (os.name == 'nt' if key.value.startswith('windows')
                                           else sys.platform.startswith('linux'))
                            changed['reportHost'] += 1
            return self.generic_visit(node)

        def visit_Attribute(self, node):
            if (isinstance(node.value, ast.Name) and node.value.id == 'subprocess'
                    and node.attr == 'CREATE_NO_WINDOW'):
                changed['creationflags'] += 1
                return ast.copy_location(ast.Constant(getattr(subprocess, 'CREATE_NO_WINDOW', 0)), node)
            return self.generic_visit(node)

    adapted = Bindings().visit(tree)
    need(changed == {'R': 1, 'OUT': 1, 'APP': 1, 'creationflags': 1, 'reportHost': 2},
         'Unexpected0358 API binding shape')
    ast.fix_missing_locations(adapted)
    code = compile(adapted, str(API_SOURCE), 'exec')
    exit_code = None
    with (out / 'api-driver.log').open('w', encoding='utf-8') as log:
        with contextlib.redirect_stdout(log), contextlib.redirect_stderr(log):
            try:
                exec(code, {'__name__': '__main__', '__file__': str(API_SOURCE)})
            except SystemExit as result:
                exit_code = result.code
    need(exit_code == 0, 'Actual0358 HTTP assertions failed; inspect api-driver.log/fixture')
    proof = read_json(out / 'api-fixture/verification.json')
    need(proof.get('passed') is True and proof.get('checks') == 74
         and proof.get('requests') == 36, 'Actual0358 HTTP assertion count differs')
    after_raw_sha = sha(API_SOURCE)
    after_canonical_sha = canonical_lf_sha(API_SOURCE)
    need(after_raw_sha == original_raw_sha and after_canonical_sha == original_canonical_sha,
         'Original0358 API driver was modified')
    return {'originalDriverSha256': original_raw_sha, 'originalFileUnchanged': True,
            'rawDriverSha256Before': original_raw_sha, 'rawDriverSha256After': after_raw_sha,
            'canonicalLfSha256Before': original_canonical_sha,
            'canonicalLfSha256After': after_canonical_sha,
            'exactAstBindings': changed, 'allOriginalAssertionsRetained': True,
            'checks': proof['checks'], 'requests': proof['requests'],
            'proof': 'api-fixture/verification.json',
            'proofSha256': sha(out / 'api-fixture/verification.json')}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--run', action='store_true', help='Run isolated CPU checks')
    parser.add_argument('--output', type=Path, help='Fresh fast-* child of linux-finishes0359')
    args = parser.parse_args()
    if not args.run:
        print(json.dumps({'planOnly': True, 'root': str(ROOT), 'commands': [
            'Build and execute actual BonusThumbnailChecks0358 in isolated artifacts',
            'Execute actual0358 Node modal and bubble fixtures',
            'Build actual WorkshopLinux framework-dependent on this host',
            'Execute all74/36 original0358 HTTP assertions with bounded AST bindings'],
            'nativeGameWslRemoteBrowserUsed': False}, indent=2))
        return 0
    output = (args.output or BASE / ('fast-' + uuid.uuid4().hex[:12])).resolve()
    need(output.parent == BASE.resolve() and output.name.startswith('fast-'),
         'Output must be a fresh fast-* direct child of pc/build/linux-finishes0359')
    need(not output.exists(), 'Existing output refused')
    output.mkdir(parents=True, exist_ok=False)
    report = {'passed': False, 'createdUtc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
              'hostPlatform': sys.platform, 'hostOsName': os.name, 'maximumCpuJobs': 1,
              'nativeGameWslBrowserGpuRemoteUsed': False, 'romOrPersonalProfileUsed': False,
              'publishedLinuxBinaryBuiltOrQualified': False, 'nativeGameBuilt': False,
              'fixturesAreSyntheticOnly': True, 'driverSha256': sha(Path(__file__)), 'commands': []}
    before = source_inventory()
    try:
        report['sourceHashes'] = before
        report['legacyPublisherCompatibility'] = legacy_publisher_contract()
        dotnet = shutil.which('dotnet'); node = shutil.which('node')
        need(dotnet is not None and node is not None, 'Existing dotnet9.0.300 and Node required; no install attempted')
        env = os.environ.copy()
        env.update(DOTNET_PROCESSOR_COUNT='1', DOTNET_CLI_TELEMETRY_OPTOUT='1',
                   DOTNET_SKIP_FIRST_TIME_EXPERIENCE='1', DOTNET_EnableDiagnostics='0')
        for key in ('DISPLAY', 'WAYLAND_DISPLAY', 'DBUS_SESSION_BUS_ADDRESS', 'PULSE_SERVER'):
            env.pop(key, None)
        sdk_selection = {'sdk': {'version': '9.0.300', 'rollForward': 'disable', 'allowPrerelease': False}}
        (output / 'global.json').write_text(json.dumps(sdk_selection, indent=2) + '\n', encoding='utf-8')
        report['sdkSelection'] = sdk_selection
        report['sdkSelectionFileSha256'] = sha(output / 'global.json')
        sdk = subprocess.check_output([dotnet, '--version'], cwd=output, env=env,
                                      text=True, timeout=30).strip()
        need(sdk == '9.0.300', 'Expected existing pinned .NET SDK9.0.300')
        report['sdkVersion'] = sdk
        report['nodeVersion'] = subprocess.check_output([node, '--version'], text=True, timeout=30).strip()
        common = ['-c', 'Release', '-p:UseSharedCompilation=false', '-p:BuildInParallel=false',
                  '-p:RestoreIgnoreFailedSources=true', '-maxcpucount:1', '--disable-build-servers']
        for project, destination in [('BonusThumbnailChecks', 'core-app'), ('WorkshopLinux', 'api-app')]:
            command = [dotnet, 'build', str(ROOT / 'pc/workshop' / project / (project + '.csproj')),
                       *common, '--artifacts-path', str(output / ('artifacts-' + project)),
                       '-o', str(output / destination)]
            report['commands'].append(run_command(command, 'build-' + project, output, env, cwd=output))
        report['commands'].append(run_command(
            [dotnet, str(output / 'core-app/BonusThumbnailChecks.dll'), str(output / 'core-fixture')],
            'core-checks', output, env, cwd=output))
        core = read_json(output / 'core-fixture/verification.json')
        need(core.get('passed') is True and core.get('checks', 0) >= 100
             and core.get('rejected', 0) >= (40 if os.name == 'nt' else 39) and core.get('symlinkCases', 0) + core.get('junctionCases', 0) == 2,
             'Actual Core PNG/IO/link fixture assertions incomplete')
        report['coreChecks'] = core
        for name, filename in [('modal', 'check-linux-bonus-thumbnails0358.cjs'),
                               ('bubble', 'check-linux-bubble-replacement0358.cjs')]:
            proof_path = output / (name + '-verification.json')
            command = [node, str(ROOT / 'pc/scripts' / filename), str(proof_path)]
            report['commands'].append(run_command(command, name + '-checks', output, env))
            proof = read_json(proof_path)
            need(proof.get('passed') is True, name + ' fixture failed')
            report[name + 'Checks'] = proof
        need(report['modalChecks'].get('actualIife') is True and report['modalChecks'].get('checks') == 667,
             'Actual modal assertions changed')
        need(report['bubbleChecks'].get('actualProductionHandler') is True
             and report['bubbleChecks'].get('coreProbeCompared') is False,
             'Unexpected native-ROM probe or missing real bubble handler')
        report['bubbleChecks']['nativePlacementWitnessNotRun'] = True
        report['httpChecks'] = actual_http_test(output / 'api-app/YoshiStory-AtelierLinux.dll', output)
        report['actualLinuxHostFrameworkExecution'] = sys.platform.startswith('linux')
        report['windowsHostedWorkshopLinuxFrameworkExecution'] = os.name == 'nt'
        report['sourceInputsUnchanged'] = before == source_inventory()
        need(report['sourceInputsUnchanged'], 'Frozen source inputs changed during checks')
        report['passed'] = True
    except Exception as error:
        report['error'] = str(error)
        report['sourceInputsUnchanged'] = before == source_inventory()
    (output / 'verification.json').write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({key: report.get(key) for key in ('passed', 'hostPlatform', 'sdkVersion', 'sourceInputsUnchanged', 'error')}, ensure_ascii=False))
    print(output / 'verification.json')
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
