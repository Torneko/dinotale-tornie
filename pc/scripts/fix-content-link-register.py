"""Retain real MIPS link addresses for resident adapters using $ra as code data.

N64Recomp's normal direct host calls omit the MIPS link write because a normal
return does not need it. Cartridge adapters derive relocated callback addresses
from that link register, so this build must preserve the hardware value.
"""
from pathlib import Path
import json
import re
import sys

out=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else Path(__file__).resolve().parents[1]/'build/content-port'
sections=json.loads((out/'function-sections.json').read_text())
count=0
for path in (out/'RecompiledFuncs').glob('*.c'):
    lines=path.read_text().splitlines(keepends=True)
    current=None;result=[]
    for line in lines:
        match=re.match(r'RECOMP_FUNC void (\w+)\(',line)
        if match:
            if match[1] in sections:current=sections[match[1]]
            elif match[1].removesuffix('_recomp') in sections:current=sections[match[1].removesuffix('_recomp')]
            else:
                static=re.fullmatch(r'static_(\d+)_[0-9A-Fa-f]+',match[1])
                current=next(s for s in sections.values() if s['index']==int(static[1])) if static else None
        result.append(line)
        call=re.search(r'// 0x([0-9A-Fa-f]+): (jal|jalr)\s',line)
        if not call:continue
        if current is None:raise ValueError(f'No section for MIPS call in {path.name}: {line}')
        address=int(call[1],16)+8
        if current['relocatable']:
            value=f'ADD32(section_addresses[{current["index"]}], 0x{address-current["base"]:X})'
        else:value=f'S32(0x{address:08X}u)'
        result.append(f'    ctx->r31 = {value}; /* workshop: preserve MIPS link before delay slot */\n')
        count+=1
    path.write_text(''.join(result))
(out/'link-register-verification.json').write_text(json.dumps({'callSitesPreserved':count,'overlayLinksUseLoadedSectionAddress':True},indent=2)+'\n')
print(f'Preserved MIPS link register at {count} native call sites.')
