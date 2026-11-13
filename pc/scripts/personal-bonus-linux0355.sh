#!/bin/sh
# Optional preview only. Copies actual local progress; never grants unlocks.
set -eu
umask 077
base=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
source="$base/pc/profiles/content-personal"
preview="$base/pc/profiles/bonus0353-qa-linux"
runtime="$base/pc/build/native-special"
expected_engine='@ENGINE_SHA256@'
expected_worker='@WORKER_SHA256@'
expected_rom='@ROM_SHA256@'
expected_content='@CONTENT_SHA256@'
prepare_only=false
if [ "${1-}" = '--prepare-only' ]; then prepare_only=true; shift; fi
[ "$#" -eq 0 ] || { printf '%s\n' 'Usage: Bonus-en-test.sh [--prepare-only]' >&2; exit 1; }
fail() { printf '%s\n' "$1" >&2; exit 1; }
sha() { digest=$(sha256sum < "$1") || return 1; printf '%.64s\n' "$digest"; }
validate_elemental() {
    file=$1
    [ -f "$file" ] && [ ! -L "$file" ] || return 1
    [ "$(wc -c < "$file")" -le 4096 ] || return 1
    # ELEMENTAL_AWK_BEGIN0355: exact parser also exercised on Windows via Git awk.
    LC_ALL=C awk '
        { text = text $0 "\n" }
        END {
            bom = sprintf("%c%c%c",239,187,191)
            if (substr(text,1,3) == bom) text = substr(text,4)
            gsub(/^[ \t\r\n]+|[ \t\r\n]+$/, "", text)
            if (substr(text,1,1) != "{" || substr(text,length(text),1) != "}") exit 1
            text = substr(text,2,length(text)-2)
            n = split(text,parts,",")
            if (n != 3 && n != 4) exit 1
            for (i=1;i<=n;i++) {
                if (split(parts[i],pair,":") != 2) exit 1
                key=pair[1]; value=pair[2]
                gsub(/^[ \t\r\n]+|[ \t\r\n]+$/, "", key)
                gsub(/^[ \t\r\n]+|[ \t\r\n]+$/, "", value)
                if (key != "\"schema\"" && key != "\"foundMask\"" && key != "\"storyAliveMask\"" && key != "\"hybridAliveMask\"") exit 1
                key=substr(key,2,length(key)-2)
                if (++seen[key] != 1 || value !~ /^(0|[1-9][0-9]*)$/ || length(value) != 1) exit 1
                field[key]=value+0
            }
            if (!("schema" in field) || !("foundMask" in field) || !("storyAliveMask" in field)) exit 1
            schema=field["schema"]; found=field["foundMask"]; story=field["storyAliveMask"]
            hybrid=("hybridAliveMask" in field)?field["hybridAliveMask"]:0
            if (schema != 1 && schema != 2) exit 1
            if (n != schema+2 || (schema == 1 && ("hybridAliveMask" in field)) || (schema == 2 && !("hybridAliveMask" in field))) exit 1
            if (found > 3 || story > 3 || hybrid > 3) exit 1
            if (story%2 > found%2 || int(story/2) > int(found/2) || hybrid%2 > found%2 || int(hybrid/2) > int(found/2)) exit 1
            exit 0
        }
    ' "$file"
    # ELEMENTAL_AWK_END0355
}
no_game() { if pgrep -x DinoTale >/dev/null 2>&1; then fail 'Fermer DinoTale avant les Bonus. / Close DinoTale before opening Bonus.'; fi; }
for tool in sha256sum flock pgrep find sort cmp readlink mktemp grep awk wc; do command -v "$tool" >/dev/null 2>&1 || fail "Missing standard Linux tool: $tool"; done
for dir in "$base/pc" "$base/pc/profiles" "$source" "$source/saves" "$runtime"; do [ -d "$dir" ] && [ ! -L "$dir" ] || fail 'A required directory is missing or linked.'; done
[ "$(readlink -f "$source")" = "$source" ] && [ "$(readlink -f "$runtime")" = "$runtime" ] || fail 'Unexpected linked package path.'
[ -f "$runtime/DinoTale" ] && [ ! -L "$runtime/DinoTale" ] && [ "$(sha "$runtime/DinoTale")" = "$expected_engine" ] || fail 'Unexpected 0.35.5 runtime.'
[ -f "$runtime/MusicRender0310" ] && [ ! -L "$runtime/MusicRender0310" ] && [ "$(sha "$runtime/MusicRender0310")" = "$expected_worker" ] || fail 'Unexpected0.35.5 music worker.'
[ -f "$runtime/workshop-content.bin" ] && [ ! -L "$runtime/workshop-content.bin" ] && [ "$(sha "$runtime/workshop-content.bin")" = "$expected_content" ] || fail 'Ouvrir Jouer.sh pour préparer le contenu de cette copie avant les Bonus. / Open Jouer.sh to prepare this copy before Bonus.'
lock="$base/pc/profiles/.bonus0355-launch.lock"
[ ! -L "$lock" ] || fail 'Linked launcher lock refused.'
exec 9>"$lock"
flock -n 9 || fail 'Another Bonus launcher is running.'
no_game
[ -f "$source/yoshisstory.n64.us.z64" ] && [ -f "$source/saves/yoshisstory.n64.us.bin" ] || fail 'Ouvrir Jouer.sh et importer la ROM avant les Bonus. / Open Jouer.sh and import the ROM first.'
elemental="$source/pc-elemental-yoshis.json"
if [ -e "$elemental" ] || [ -L "$elemental" ]; then validate_elemental "$elemental" || fail 'Invalid canonical elemental journal; no progress imported.'; fi
[ ! -L "$preview" ] || fail 'Linked preview profile refused.'
mkdir -p "$preview"
[ "$(readlink -f "$preview")" = "$preview" ] || fail 'Unexpected preview path.'
for dir in "$preview/saves" "$preview/assets"; do [ ! -L "$dir" ] || fail 'Linked preview directory refused.'; mkdir -p "$dir"; done
stage=$(mktemp -d "$preview/.snapshot.XXXXXXXXXX")
cleanup() {
    if [ -n "${stage-}" ] && [ -d "$stage" ]; then
        [ "$(dirname -- "$stage")" = "$preview" ] && [ "$(readlink -f "$stage")" = "$stage" ] || return 1
        case "${stage##*/}" in .snapshot.*) rm -rf -- "$stage" ;; *) return 1 ;; esac
    fi
}
trap cleanup EXIT HUP INT TERM
mkdir -p "$stage/profile/saves"
names() {
    for name in yoshisstory.n64.us.z64 saves/yoshisstory.n64.us.bin controls.json general.json graphics.json sound.json book-menu.json; do
        [ ! -L "$source/$name" ] || fail 'Linked snapshot source refused.'
        if [ -f "$source/$name" ]; then printf '%s\n' "$name"; fi
    done
    for file in "$source"/pc-*.json; do
        [ -e "$file" ] || continue
        name=${file##*/}
        case "$name" in pc-bonus*) continue ;; *'
'*) fail 'Newline in snapshot file name.' ;; esac
        [ -f "$file" ] && [ ! -L "$file" ] || fail 'Invalid snapshot source.'
        printf '%s\n' "$name"
    done
}
bonus_hashes() {
    for file in "$preview"/pc-bonus* "$preview"/saves/yoshisstory.n64.us.bonus0353*; do
        [ -e "$file" ] || continue
        [ -f "$file" ] && [ ! -L "$file" ] || fail 'Invalid Bonus progress file.'
        printf '%s %s\n' "$(sha "$file")" "${file##*/}"
    done
}
bonus_hashes > "$stage/bonus.before.raw"
LC_ALL=C sort "$stage/bonus.before.raw" > "$stage/bonus.before"
names > "$stage/names.before.raw"
LC_ALL=C sort -u "$stage/names.before.raw" > "$stage/names.before"
while IFS= read -r name; do
    cp -- "$source/$name" "$stage/profile/$name"
    cmp -s -- "$source/$name" "$stage/profile/$name" || fail 'Snapshot changed during copy.'
done < "$stage/names.before"
if [ -f "$stage/profile/pc-elemental-yoshis.json" ]; then validate_elemental "$stage/profile/pc-elemental-yoshis.json" || fail 'Invalid staged elemental journal; no progress imported.'; fi
[ "$(sha "$stage/profile/yoshisstory.n64.us.z64")" = "$expected_rom" ] || fail 'Original USA ROM hash mismatch.'
# Clear only obsolete ordinary snapshot files. Never remove Bonus journals or saves.
for file in "$preview"/pc-*.json "$preview"/controls.json "$preview"/general.json "$preview"/graphics.json "$preview"/sound.json "$preview"/book-menu.json; do
    [ -e "$file" ] || continue
    name=${file##*/};case "$name" in pc-bonus*) continue ;; esac
    [ -f "$file" ] && [ ! -L "$file" ] || fail 'Invalid obsolete snapshot.'
    if ! grep -Fqx -- "$name" "$stage/names.before"; then rm -f -- "$file"; fi
done
while IFS= read -r name; do
    [ ! -L "$preview/$name" ] || fail 'Linked snapshot destination refused.'
    cp -- "$stage/profile/$name" "$preview/$name"
done < "$stage/names.before"
[ -d "$runtime/assets" ] && [ ! -L "$runtime/assets" ] || fail 'Runtime assets missing.'
[ -z "$(find "$runtime/assets" "$preview/assets" -type l -print -quit)" ] || fail 'Linked asset refused.'
cp -R -- "$runtime/assets/." "$preview/assets/"
for file in "$preview/portable.txt" "$preview/bonus-complete0353.txt"; do [ ! -L "$file" ] || fail 'Linked preview marker refused.'; done
: > "$preview/portable.txt"
printf 'DinoTale Bonus0353 complete route preview only\n' > "$preview/bonus-complete0353.txt"
no_game
names > "$stage/names.after.raw"
LC_ALL=C sort -u "$stage/names.after.raw" > "$stage/names.after"
cmp -s "$stage/names.before" "$stage/names.after" || fail 'Personal snapshot inventory changed.'
while IFS= read -r name; do cmp -s -- "$source/$name" "$stage/profile/$name" && cmp -s -- "$preview/$name" "$stage/profile/$name" || fail 'Personal progress changed; preview not launched.'; done < "$stage/names.before"
bonus_hashes > "$stage/bonus.after.raw"
LC_ALL=C sort "$stage/bonus.after.raw" > "$stage/bonus.after"
cmp -s "$stage/bonus.before" "$stage/bonus.after" || fail 'Bonus progress changed during preparation.'
[ "$(sha "$runtime/DinoTale")" = "$expected_engine" ] || fail 'Runtime changed during preparation.'
[ "$(sha "$runtime/MusicRender0310")" = "$expected_worker" ] || fail 'Music worker changed during preparation.'
if [ "$prepare_only" = true ]; then printf '%s\n' 'Bonus snapshot ready; personal progress preserved; no unlocks forced; no game launched.'; exit 0; fi
export LD_LIBRARY_PATH="$runtime/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
cd -- "$preview"
"$runtime/DinoTale" --workshop-main-menu --workshop-bonus-finales0353 1 24
