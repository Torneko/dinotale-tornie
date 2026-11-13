[CmdletBinding()]
param([switch]$PrepareOnly)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$expectedRuntimeHash = '08ffb814ce95703f248ec50000f7ad9b6c3023ff572c74150a2633ff650c5d75'
$expectedWorkerHash = '2b7498e161eaa5638321c07e36a433477335f202335f7e23f48efedffe8e55e6'
$expectedRomHash = 'e8a63388c38f8f0bea37e9b13a97f0898ac5496e08ca2028afb8db9a866e0ce9'
$root = [IO.Path]::GetFullPath($PSScriptRoot)
$destination = [IO.Path]::GetFullPath((Join-Path $root 'pc\profiles\bonus0353-qa-portable'))
$source = Join-Path $root 'pc\profiles\content-personal'
$runtime = Join-Path $root 'pc\build\native-special'
$executable = Join-Path $runtime 'DinoTale.exe'
$worker = Join-Path $runtime 'MusicRender0310.exe'
$stage = $null
$mutex = $null
$held = $false

# No PowerShell module autoload is needed for hashing: a Windows PowerShell
# child may inherit PSModulePath from PowerShell Core through the installer.
function Get-BonusFileHash0353 {
    param([Parameter(Mandatory=$true)][string]$LiteralPath,[ValidateSet('SHA256')][string]$Algorithm='SHA256')
    $stream=$null;$sha=$null
    try {
        $stream=[IO.File]::OpenRead($LiteralPath)
        $sha=[Security.Cryptography.SHA256]::Create()
        $digest=$sha.ComputeHash($stream)
        return [pscustomobject]@{Hash=([BitConverter]::ToString($digest)).Replace('-','');Algorithm=$Algorithm}
    } finally {
        if($stream){$stream.Dispose()}
        if($sha){$sha.Dispose()}
    }
}
function Assert-Under([string]$Path, [string]$Base) {
    $full = [IO.Path]::GetFullPath($Path)
    $parent = [IO.Path]::GetFullPath($Base).TrimEnd('\') + '\'
    if (-not $full.StartsWith($parent, [StringComparison]::OrdinalIgnoreCase)) { throw 'A destination escaped its dedicated Bonus folder.' }
    return $full
}
function Assert-Destination([string]$Path) {
    $full = [IO.Path]::GetFullPath($Path)
    if ($full -ne $destination) { $null = Assert-Under $full $destination }
    $walk = $full
    while ($walk) {
        if (Test-Path -LiteralPath $walk) {
            $item = Get-Item -LiteralPath $walk -Force
            if (($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0 -or (($item.PSObject.Properties.Name -contains 'LinkType') -and $item.LinkType)) { throw 'A Bonus destination is a link or reparse point.' }
        }
        $parent = [IO.Directory]::GetParent($walk)
        if ($null -eq $parent) { break }
        $walk = $parent.FullName
    }
    return $full
}
function Assert-Source([string]$Path) {
    $full = Assert-Under $Path $root
    $walk = $full
    while ($walk.Length -ge $root.Length) {
        if (Test-Path -LiteralPath $walk) {
            $item = Get-Item -LiteralPath $walk -Force
            # Cloud placeholders can carry ReparsePoint without being links.
            # Read them normally, but never follow explicit junction/symlink targets.
            if (($item.PSObject.Properties.Name -contains 'LinkType') -and $item.LinkType -in @('SymbolicLink','Junction')) { throw 'A personal source is an explicit symbolic link or junction.' }
        }
        if ($walk -eq $root) { break }
        $walk = [IO.Directory]::GetParent($walk).FullName
    }
    return $full
}
function Assert-NoGame {
    $running = @(Get-CimInstance -ClassName Win32_Process -Filter "Name = 'DinoTale.exe'")
    if ($running.Count) { throw 'Close DinoTale before preparing or opening the personal Bonus preview. Only one game window is allowed.' }
}
function File-Fingerprint([string]$Path, [string]$Relative) {
    $item = Get-Item -LiteralPath $Path -Force
    if ($item.PSIsContainer) { throw 'A snapshot source is not a regular file.' }
    return [pscustomobject]@{ Relative=$Relative; Length=$item.Length; Hash=(Get-BonusFileHash0353 -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant() }
}
function Snapshot-Inventory {
    $relative = @('yoshisstory.n64.us.z64','saves\yoshisstory.n64.us.bin','controls.json','general.json','graphics.json','sound.json','book-menu.json')
    if (Test-Path -LiteralPath $source -PathType Container) {
        foreach ($item in @(Get-ChildItem -LiteralPath $source -Force -File)) {
            if ($item.Name -like 'pc-*.json' -and $item.Name -notlike 'pc-bonus*') { $relative += $item.Name }
        }
    }
    $result = @()
    foreach ($name in @($relative | Sort-Object -Unique)) {
        $path = Assert-Source (Join-Path $source $name)
        if (Test-Path -LiteralPath $path -PathType Leaf) { $result += File-Fingerprint $path $name }
    }
    return @($result)
}
# Validate the native writer's deliberately flat journal without lossy JSON
# reserialization. Duplicate/extra/escaped keys, signed/fractional numbers and
# unearned temporary lives are rejected; schema1 remains byte-exact/read-only.
function Read-ElementalSnapshot0355 {
    $path=Assert-Source (Join-Path $source 'pc-elemental-yoshis.json')
    if (-not (Test-Path -LiteralPath $path)) { return [pscustomobject]@{Present=$false;Schema=0;FoundCount=0} }
    $item=Get-Item -LiteralPath $path -Force
    if ($item.PSIsContainer -or $item.Length -gt 4096) { throw 'The canonical elemental journal is not a bounded regular file. Nothing was imported.' }
    $bytes=[IO.File]::ReadAllBytes($path)
    if($bytes.Length -gt 4096){throw 'The canonical elemental journal changed size; nothing was imported.'}
    $offset=0
    if($bytes.Length -ge 3 -and $bytes[0] -eq 239 -and $bytes[1] -eq 187 -and $bytes[2] -eq 191){$offset=3}
    try { $text=(New-Object Text.UTF8Encoding($false,$true)).GetString($bytes,$offset,$bytes.Length-$offset) }
    catch { throw 'The canonical elemental journal is not valid UTF8. Nothing was imported.' }
    $white='[ \t\r\n]'
    $pair='"(?<key>schema|foundMask|storyAliveMask|hybridAliveMask)"'+$white+'*:'+ $white+'*(?<value>0|[1-9][0-9]*)'
    $pattern='\A'+$white+'*\{'+$white+'*'+$pair+'(?:'+$white+'*,'+$white+'*'+$pair+')*'+$white+'*\}'+$white+'*\z'
    $match=[regex]::Match($text,$pattern,[Text.RegularExpressions.RegexOptions]::CultureInvariant)
    if (-not $match.Success) { throw 'The canonical elemental journal format is invalid. Nothing was imported.' }
    $values=New-Object 'System.Collections.Generic.Dictionary[string,uint64]' ([StringComparer]::Ordinal)
    $keys=$match.Groups['key'].Captures;$numbers=$match.Groups['value'].Captures
    for($index=0;$index -lt $keys.Count;++$index){
        $key=$keys[$index].Value;[uint64]$number=0
        if($values.ContainsKey($key) -or -not [uint64]::TryParse($numbers[$index].Value,[Globalization.NumberStyles]::None,[Globalization.CultureInfo]::InvariantCulture,[ref]$number)) { throw 'The canonical elemental journal has duplicate or invalid values. Nothing was imported.' }
        $values.Add($key,$number)
    }
    if(-not $values.ContainsKey('schema') -or -not $values.ContainsKey('foundMask') -or -not $values.ContainsKey('storyAliveMask')) { throw 'The canonical elemental journal is incomplete. Nothing was imported.' }
    $schema=$values['schema'];$found=$values['foundMask'];$story=$values['storyAliveMask'];[uint64]$hybrid=0
    if(($schema -ne 1 -and $schema -ne 2) -or $values.Count -ne (2+$schema) -or ($schema -eq 1 -and $values.ContainsKey('hybridAliveMask')) -or ($schema -eq 2 -and -not $values.ContainsKey('hybridAliveMask'))) { throw 'The canonical elemental journal schema is invalid. Nothing was imported.' }
    if($schema -eq 2){$hybrid=$values['hybridAliveMask']}
    if($found -gt 3 -or $story -gt 3 -or $hybrid -gt 3 -or (($story -bor $hybrid) -band (3 -bxor $found))) { throw 'The canonical elemental journal contains unearned ownership or lives. Nothing was imported.' }
    $count=[int](($found -band 1) -ne 0)+[int](($found -band 2) -ne 0)
    return [pscustomobject]@{Present=$true;Schema=[int]$schema;FoundCount=$count}
}
function Inventory-Json($Value) { return ConvertTo-Json -InputObject @($Value) -Depth 6 -Compress }
function Safe-Tree([string]$Base, [bool]$IsDestination) {
    $pending = New-Object 'System.Collections.Generic.Queue[string]'
    $pending.Enqueue($Base)
    $files = @()
    while ($pending.Count) {
        $directory = $pending.Dequeue()
        if ($IsDestination) { $null=Assert-Destination $directory } else { $null=Assert-Source $directory }
        if (-not (Test-Path -LiteralPath $directory -PathType Container)) { throw 'An asset directory is missing.' }
        foreach ($item in @(Get-ChildItem -LiteralPath $directory -Force)) {
            if ($IsDestination) { $null=Assert-Destination $item.FullName } else { $null=Assert-Source $item.FullName }
            if ($item.PSIsContainer) { $pending.Enqueue($item.FullName) } else { $files += $item.FullName }
            if ($files.Count + $pending.Count -gt 4096) { throw 'The bounded asset inventory is unexpectedly large.' }
        }
    }
    return @($files)
}
function Bonus-Fingerprints {
    $paths = @()
    if (Test-Path -LiteralPath $destination -PathType Container) {
        $paths += @(Get-ChildItem -LiteralPath $destination -Force -File | Where-Object { $_.Name -like 'pc-bonus*' } | ForEach-Object { $_.FullName })
        $saves = Assert-Destination (Join-Path $destination 'saves')
        if (Test-Path -LiteralPath $saves -PathType Container) { $paths += @(Get-ChildItem -LiteralPath $saves -Force -File | Where-Object { $_.Name -like 'yoshisstory.n64.us.bonus0353*' } | ForEach-Object { $_.FullName }) }
    }
    $result = @()
    foreach ($path in @($paths | Sort-Object)) { $null=Assert-Destination $path; $result += File-Fingerprint $path $path.Substring($destination.Length+1) }
    return @($result)
}
function Write-Utf8([string]$Path,[string]$Value) {
    $full=Assert-Destination $Path
    [IO.File]::WriteAllText($full,$Value,(New-Object Text.UTF8Encoding($false)))
}
function Remove-SnapshotFile([string]$Path) {
    $full=Assert-Destination $Path
    $relative=$full.Substring($destination.Length+1)
    $fixed=@('yoshisstory.n64.us.z64','saves\yoshisstory.n64.us.bin','controls.json','general.json','graphics.json','sound.json','book-menu.json')
    $topPC=($relative -notmatch '[\\/]') -and ($relative -like 'pc-*.json') -and ($relative -notlike 'pc-bonus*')
    if (($fixed -notcontains $relative) -and -not $topPC) { throw 'Refusing to remove a file outside the ordinary personal snapshot allowlist.' }
    if (Test-Path -LiteralPath $full -PathType Leaf) { Remove-Item -LiteralPath $full -Force }
}
try {
    if (-not (Test-Path -LiteralPath (Join-Path $root 'pc\portable-package.json') -PathType Leaf)) { throw 'Run this Bonus launcher from a complete portable DinoTale package.' }
    $null = Assert-Under $destination $root
    $null=Assert-Source $source; $null=Assert-Source $executable; $null=Assert-Source $worker
    if (-not (Test-Path -LiteralPath $source -PathType Container)) { throw 'The personal content profile is missing.' }
    if (-not (Test-Path -LiteralPath $executable -PathType Leaf) -or (Get-BonusFileHash0353 -LiteralPath $executable -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expectedRuntimeHash) { throw 'The installed runtime is not the verified 0.35.5 executable.' }
    if (-not (Test-Path -LiteralPath $worker -PathType Leaf) -or (Get-BonusFileHash0353 -LiteralPath $worker -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expectedWorkerHash) { throw 'The installed music worker is not the verified 0.35.5 executable.' }
    $created=$false
    $mutex=New-Object Threading.Mutex($false,'Local\DinoTalePortableBonus0355',[ref]$created)
    try { $held=$mutex.WaitOne(0) } catch [Threading.AbandonedMutexException] { $held=$true }
    if (-not $held) { throw 'Another personal Bonus launcher is already running.' }
    Assert-NoGame
    $elementalState=Read-ElementalSnapshot0355
    $null=Assert-Destination $destination
    $elementalDestination=Assert-Destination (Join-Path $destination 'pc-elemental-yoshis.json')
    $elementalPreviouslyPresent=Test-Path -LiteralPath $elementalDestination -PathType Leaf
    if(Test-Path -LiteralPath $elementalDestination -PathType Container){throw 'The Bonus elemental snapshot is a directory; preserved without import.'}
    if (-not (Test-Path -LiteralPath $destination -PathType Container)) { $null=New-Item -ItemType Directory -Path $destination }
    $bonusBefore=Inventory-Json (Bonus-Fingerprints)
    $inventory=@(Snapshot-Inventory)
    $rom=@($inventory | Where-Object { $_.Relative -eq 'yoshisstory.n64.us.z64' })
    if ($rom.Count -and ($rom[0].Length -ne 16777216 -or $rom[0].Hash -ne $expectedRomHash)) { throw 'The personal source ROM is not the exact original USA ROM. Nothing was imported.' }
    $assetsSource=Assert-Source (Join-Path $runtime 'assets')
    $assets=@(Safe-Tree $assetsSource $false)
    $assetInventory=@($assets | ForEach-Object { File-Fingerprint $_ $_.Substring($assetsSource.Length+1) } | Sort-Object Relative)
    $stage=Assert-Destination (Join-Path $destination ('.snapshot-stage-'+[Guid]::NewGuid().ToString('N')))
    $null=New-Item -ItemType Directory -Path $stage
    foreach ($entry in $inventory) {
        $inputPath=Assert-Source (Join-Path $source $entry.Relative)
        $outputPath=Assert-Destination (Join-Path $stage ('profile\'+$entry.Relative))
        $null=New-Item -ItemType Directory -Path ([IO.Path]::GetDirectoryName($outputPath)) -Force
        Copy-Item -LiteralPath $inputPath -Destination $outputPath
        if ((Get-BonusFileHash0353 -LiteralPath $outputPath -Algorithm SHA256).Hash.ToLowerInvariant() -ne $entry.Hash) { throw 'A staged snapshot hash differs from its personal source.' }
    }
    foreach ($entry in $assetInventory) {
        $inputPath=Assert-Source (Join-Path $assetsSource $entry.Relative)
        $outputPath=Assert-Destination (Join-Path $stage ('assets\'+$entry.Relative))
        $null=New-Item -ItemType Directory -Path ([IO.Path]::GetDirectoryName($outputPath)) -Force
        Copy-Item -LiteralPath $inputPath -Destination $outputPath
        if ((Get-BonusFileHash0353 -LiteralPath $outputPath -Algorithm SHA256).Hash.ToLowerInvariant() -ne $entry.Hash) { throw 'A staged asset differs from the verified runtime.' }
    }
    Assert-NoGame
    if ((Inventory-Json (Snapshot-Inventory)) -ne (Inventory-Json $inventory)) { throw 'Personal progress changed during its read-only snapshot. Close the game and retry.' }
    $old=@('yoshisstory.n64.us.z64','saves\yoshisstory.n64.us.bin','controls.json','general.json','graphics.json','sound.json','book-menu.json')
    $old+=@(Get-ChildItem -LiteralPath $destination -Force -File | Where-Object { $_.Name -like 'pc-*.json' -and $_.Name -notlike 'pc-bonus*' } | ForEach-Object { $_.Name })
    $wanted=@($inventory | ForEach-Object { $_.Relative })
    $removed=0
    foreach ($name in @($old | Sort-Object -Unique)) { if ($wanted -notcontains $name) { $oldPath=Assert-Destination (Join-Path $destination $name);if(Test-Path -LiteralPath $oldPath -PathType Leaf){Remove-SnapshotFile $oldPath;++$removed} } }
    foreach ($entry in $inventory) {
        $target=Assert-Destination (Join-Path $destination $entry.Relative)
        $null=New-Item -ItemType Directory -Path ([IO.Path]::GetDirectoryName($target)) -Force
        Copy-Item -LiteralPath (Join-Path $stage ('profile\'+$entry.Relative)) -Destination $target -Force
    }
    $assetDestination=Assert-Destination (Join-Path $destination 'assets')
    if (Test-Path -LiteralPath $assetDestination) { $null=Safe-Tree $assetDestination $true } else { $null=New-Item -ItemType Directory -Path $assetDestination }
    foreach ($entry in $assetInventory) {
        $target=Assert-Destination (Join-Path $assetDestination $entry.Relative)
        $null=New-Item -ItemType Directory -Path ([IO.Path]::GetDirectoryName($target)) -Force
        Copy-Item -LiteralPath (Join-Path $stage ('assets\'+$entry.Relative)) -Destination $target -Force
    }
    Write-Utf8 (Join-Path $destination 'portable.txt') ''
    Write-Utf8 (Join-Path $destination 'bonus-complete0353.txt') "DinoTale Bonus0353 complete route preview only`n"
    Assert-NoGame
    if ((Inventory-Json (Snapshot-Inventory)) -ne (Inventory-Json $inventory)) { throw 'Personal progress changed while the preview was prepared; the game was not launched.' }
    foreach ($entry in $inventory) { if((Get-BonusFileHash0353 -LiteralPath (Assert-Destination (Join-Path $destination $entry.Relative)) -Algorithm SHA256).Hash.ToLowerInvariant() -ne $entry.Hash){throw 'A prepared personal snapshot hash differs.'} }
    if ((Inventory-Json (Bonus-Fingerprints)) -ne $bonusBefore) { throw 'Existing Bonus progress changed during preparation.' }
    if ((Get-BonusFileHash0353 -LiteralPath $executable -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expectedRuntimeHash) { throw 'The runtime changed during preparation.' }
    if ((Get-BonusFileHash0353 -LiteralPath $worker -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expectedWorkerHash) { throw 'The music worker changed during preparation.' }
    $missing=@('yoshisstory.n64.us.z64','saves\yoshisstory.n64.us.bin' | Where-Object { $wanted -notcontains $_ })
    $summary=[ordered]@{schema=1;prepared=($missing.Count -eq 0);prepareOnly=[bool]$PrepareOnly;profile='bonus0353-qa-portable';runtimeSha256=$expectedRuntimeHash;workerSha256=$expectedWorkerHash;supportedSoloColors=18;hardPrerequisiteColors=16;elementalSourcePresent=[bool]$elementalState.Present;elementalSourceSchema=$elementalState.Schema;elementalOwnedCount=$elementalState.FoundCount;elementalSnapshotCopied=[bool]$elementalState.Present;elementalStaleSnapshotRemoved=([bool]$elementalPreviouslyPresent -and -not [bool]$elementalState.Present);snapshotFiles=$inventory.Count;obsoleteSnapshotsRemoved=$removed;bonusProgressPreserved=$true;personalFilesWritten=$false;qaProgressImported=$false;unlocksForced=$false;launched=$false}
    if ($missing.Count) { Write-Utf8 (Join-Path $destination 'bonus-portable-prepare0353.json') (ConvertTo-Json $summary);throw 'The original ROM or normal personal save is missing. Stale preview copies were removed; Bonus progress was preserved.' }
    if (-not $PrepareOnly) {
        Assert-NoGame
        $child=Start-Process -FilePath $executable -ArgumentList @('--workshop-main-menu','--workshop-bonus-finales0353','1','24') -WorkingDirectory $destination -WindowStyle Normal -PassThru
        Start-Sleep -Milliseconds 500
        if (@(Get-CimInstance -ClassName Win32_Process -Filter "Name = 'DinoTale.exe'").Count -gt 1) { if(-not $child.HasExited){Stop-Process -Id $child.Id -ErrorAction SilentlyContinue};throw 'Another game started concurrently. Only the newly launched Bonus process was stopped.' }
        if ($child.HasExited) { throw 'The Bonus runtime closed before its menu opened. Existing progress was preserved.' }
        $summary.launched=$true
    }
    Write-Utf8 (Join-Path $destination 'bonus-portable-prepare0353.json') (ConvertTo-Json $summary)
    ConvertTo-Json $summary -Compress
} finally {
    if ($stage -and (Test-Path -LiteralPath $stage -PathType Container)) {
        # Resolve and verify this unique, dedicated staging directory before the
        # only recursive deletion. Never delete the profile or a linked tree.
        $resolved=(Resolve-Path -LiteralPath $stage).ProviderPath
        $null=Assert-Destination $resolved
        if ([IO.Path]::GetDirectoryName($resolved) -ne $destination -or [IO.Path]::GetFileName($resolved) -notmatch '^\.snapshot-stage-[0-9a-f]{32}$') { throw 'Refusing unsafe stage cleanup.' }
        $null=Safe-Tree $resolved $true
        Remove-Item -LiteralPath $resolved -Recurse -Force
    }
    if ($held -and $mutex) { $mutex.ReleaseMutex() }
    if ($mutex) { $mutex.Dispose() }
}
