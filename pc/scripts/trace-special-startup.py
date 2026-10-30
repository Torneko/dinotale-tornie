"""Optional bounded startup diagnostics; ordinary play never enables tracing."""
from pathlib import Path
import re
import sys
out=Path(sys.argv[1])/'RecompiledFuncs'
count=0
for path in out.glob('*.c'):
    source=path.read_text()
    source,n=re.subn(r'(RECOMP_FUNC void \w+\(uint8_t\* rdram, recomp_context\* ctx\) \{\n)',
        r'\1#ifdef WORKSHOP_SPECIAL_CONTENT\n    workshop_trace_entry(rdram,ctx,__func__);\n#endif\n',source)
    path.write_text(source);count+=n
print(f'Prepared opt-in diagnostics at {count} game entries.')
