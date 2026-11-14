"""Check the buildless FR/EN site without loading a browser or remote assets."""
from pathlib import Path
from html.parser import HTMLParser
from urllib.parse import urlsplit, unquote
import hashlib
import json
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
WEB = ROOT / 'website'


class Page(HTMLParser):
    def __init__(self):
        super().__init__()
        self.ids = set()
        self.active_ids = set()
        self.refs = []
        self.sections = []
        self.inert_sections = []
        self.template_depth = 0
        self.images = []
        self.controls = []
        self.labels = []
        self.visible_h1 = 0
        self.main_count = 0
        self.lang = None
        self.attrs_by_id = {}

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if tag == 'template':
            self.template_depth += 1
        inert = self.template_depth > 0
        if tag == 'html':
            self.lang = attrs.get('lang')
        if 'id' in attrs:
            assert attrs['id'] not in self.ids, attrs['id']
            self.ids.add(attrs['id'])
            self.attrs_by_id[attrs['id']] = attrs
            if not inert:
                self.active_ids.add(attrs['id'])
        if tag == 'section' and 'section' in attrs.get('class', '').split():
            (self.inert_sections if inert else self.sections).append(attrs.get('id'))
        if tag == 'h1' and not inert:
            self.visible_h1 += 1
        if tag == 'main' and not inert:
            self.main_count += 1
        if tag == 'img':
            assert attrs.get('alt'), 'Images require useful alternative text'
            self.images.append((attrs, inert))
        for key in ('src', 'href'):
            if key in attrs:
                self.refs.append(attrs[key])
        for key in ('aria-controls', 'aria-labelledby'):
            self.controls.extend(attrs.get(key, '').split())
        if tag == 'label':
            self.labels.append(attrs.get('for'))

    def handle_endtag(self, tag):
        if tag == 'template':
            self.template_depth -= 1
            assert self.template_depth >= 0


pages = {}
for path in sorted(WEB.glob('*.html')):
    source = path.read_text(encoding='utf-8')
    page = Page()
    page.feed(source)
    assert page.template_depth == 0
    assert page.visible_h1 == 1 and page.main_count == 1, path
    assert page.lang in ('fr', 'en')
    assert '<meta name="viewport"' in source
    assert all(ref in page.ids for ref in page.controls + page.labels), path
    assert '0.35.5-beta.4' in source, 'Current prerelease must be identified'
    assert not re.search(r'Canti|OneDrive|[CD]:[\\/]', source, re.I), 'Personal installation details must stay out of the public site'
    pages[path.name] = page
    for ref in page.refs:
        url = urlsplit(ref)
        if url.scheme or url.netloc:
            assert url.scheme in ('https', 'mailto'), (path, ref)
            continue
        target = WEB / unquote(url.path) if url.path else path
        assert target.is_file(), (path, ref)
        if url.fragment and target.suffix == '.html':
            other = Page()
            other.feed(target.read_text(encoding='utf-8'))
            assert unquote(url.fragment) in other.ids, (path, ref)

assert set(pages) == {'index.html', 'index-en.html', 'guide.html', 'guide-en.html'}
assert pages['index.html'].sections == pages['index-en.html'].sections
assert len(pages['index.html'].sections) == 8
assert pages['guide.html'].inert_sections == pages['guide-en.html'].inert_sections
assert len(pages['guide.html'].inert_sections) == 7

# A fragment never finds the detailed sections in the initial active document.
spoiler_terms = re.compile(r'\b(hybrid\w*|turquoise\w*|seize|sixteen|eighteen|elemental\w*|\u00e9l\u00e9mentair\w*)\b|gold and silver|or et argent|Boss Battle|Story difficile|Hard Story|dix-huit|\b(?:Feu|Eau|Fire|Water|2-2|3-4)\b', re.I)
for name in ('index.html', 'index-en.html'):
    source = (WEB / name).read_text(encoding='utf-8')
    assert not spoiler_terms.search(source), (name, spoiler_terms.search(source).group() if spoiler_terms.search(source) else '')
    assert not pages[name].inert_sections
    assert all(image['src'] == 'assets/egg-book.svg' and not inert for image, inert in pages[name].images)
    for required in ('DinoTale.exe', 'Jouer.sh', 'Atelier.sh', '.z64', '.v64', '.n64', '0.35.5-beta.4'):
        assert required in source, (name, required)
    assert 'USA' in source
    assert ('3 %' if name == 'index.html' else '3%') in source

for name in ('guide.html', 'guide-en.html'):
    page = pages[name]
    assert page.sections == []
    assert not set(page.inert_sections) & page.active_ids
    assert 'hidden' in page.attrs_by_id['spoiler-body']
    assert page.attrs_by_id['spoiler-reveal']['aria-expanded'] == 'false'
    assert page.attrs_by_id['spoiler-reveal']['aria-controls'] == 'spoiler-body'
    assert all(inert and image.get('loading') == 'lazy' for image, inert in page.images)
    source = (WEB / name).read_text(encoding='utf-8')
    assert '<noscript>' in source and '<template id="spoiler-content">' in source
    assert '0.35.5-beta.4' in source and '0.34.0-beta.1' in source
    assert ('seize couleurs historiques' if name == 'guide.html' else 'original sixteen') in source
    assert ('douze couleurs' if name == 'guide.html' else 'twelve egg colours') in source

release = (WEB / 'assets/release.js').read_bytes()
assert b'0.35.5-beta.4' in release and b'0.34.0-beta.1' not in release, 'Release metadata must target current prerelease'
css = (WEB / 'assets/guide.css').read_text(encoding='utf-8')
assert '[hidden] { display: none !important; }' in css
assert ':focus-visible' in css and 'prefers-reduced-motion' in css
assert '@media (max-width: 720px)' in css
js = (WEB / 'assets/guide.js').read_text(encoding='utf-8')
assert not re.search(r'localStorage|sessionStorage|document\.cookie|\.innerHTML\s*=', js)
node = shutil.which('node')
assert node, 'Node.js is required for the DOM-event regression checks'
subprocess.run([node, '--check', str(WEB / 'assets/guide.js')], check=True, timeout=20)
subprocess.run([node, str(ROOT / 'scripts/check-spoilers.cjs')], check=True, timeout=20)
subprocess.run([node, str(ROOT / 'scripts/check-release.cjs')], check=True, timeout=20)
print(json.dumps({'status': 'PASS', 'pages': 4, 'public_sections_per_language': 8,
                  'inert_spoiler_sections_per_language': 7,
                  'published_release': '0.35.5-beta.4', 'local_links': 'PASS',
                  'semantic_html': 'PASS', 'browser_gpu_used': False}))
