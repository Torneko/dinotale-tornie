(()=>{
  const release=window.DINOTALE_RELEASE={version:"0.35.5-beta.4",available:true,prerelease:true,linuxExperimental:true};
  const en=document.documentElement.lang==='en',target=document.querySelector('#release-downloads');
  if(!target)return;
  const base='https://github.com/Torneko/dinotale-tornie/releases/download/v'+release.version+'/';
  const links=[
    [`DinoTale-Tornie-${release.version}-Windows-x64.zip`,en?'Download Windows x64':'Télécharger Windows x64'],
    [`DinoTale-Tornie-${release.version}-Linux-x64.tar.gz`,en?'Linux x64 · experimental candidate':'Linux x64 · candidat expérimental'],
    [`DinoTale-Tornie-${release.version}-Sources.zip`,en?'Source code · may contain spoilers':'Code source · peut contenir des spoilers'],
    [`DinoTale-Tornie-Guide-FR-EN-${release.version}.zip`,en?'Offline FR/EN guide · spoilers':'Guide FR/EN hors ligne · spoilers'],
    ['SHA256SUMS.txt',en?'SHA256 checksums':'Empreintes SHA256']
  ];
  target.replaceChildren(...links.map(([file,label])=>{const a=document.createElement('a');a.href=base+file;a.textContent=label;a.className='button';return a;}));
  target.style.cssText='display:flex;flex-wrap:wrap;gap:10px';
})();
