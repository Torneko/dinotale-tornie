from pathlib import Path
from html.parser import HTMLParser
from urllib.parse import urlsplit,unquote
root=Path(__file__).resolve().parents[1]/'website'
class Page(HTMLParser):
 def __init__(self):super().__init__();self.ids=set();self.refs=[];self.sections=[]
 def handle_starttag(self,tag,attrs):
  a=dict(attrs)
  if 'id'in a:assert a['id'] not in self.ids,a['id'];self.ids.add(a['id'])
  if tag=='section':self.sections.append(a.get('id'))
  if tag=='img':assert a.get('alt') is not None
  for k in ('src','href'):
   if k in a:self.refs.append(a[k])
pages={}
for path in sorted(root.glob('*.html')):
 page=Page();page.feed(path.read_text(encoding='utf-8'));pages[path.name]=page
 for ref in page.refs:
  u=urlsplit(ref)
  if u.scheme or u.netloc:continue
  target=root/unquote(u.path) if u.path else path
  assert target.is_file(),(path,ref)
  if u.fragment and target.suffix=='.html':
   other=Page();other.feed(target.read_text(encoding='utf-8'));assert u.fragment in other.ids,(path,ref)
assert pages['index.html'].sections==pages['index-en.html'].sections
assert len(pages['index.html'].sections)==12
assert all((root/'assets/fruits'/f'{n}-24x21.png').is_file() for n in ('peche','mure','bleuet','poire-verte','prune','kiwi'))
print('FR/EN guide: local links, unique IDs, 12 matching sections and six fruit assets verified.')
