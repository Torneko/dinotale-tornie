(()=>{
  const release=window.DINOTALE_RELEASE={version:"0.34.0-beta.1",available:true,prerelease:true,linuxExperimental:true};
  const en=document.documentElement.lang==='en',target=document.querySelector('#release-downloads');
  if(!target)return;
  const base='https://github.com/Torneko/dinotale-tornie/releases/download/v'+release.version+'/';
  const links=[
    ['DinoTale-Tornie-0.34.0-beta.1-Windows-x64.zip',en?'Download Windows x64':'Télécharger Windows x64'],
    ['DinoTale-Tornie-0.34.0-beta.1-Linux-x64.tar.gz',en?'Linux x64 · experimental candidate':'Linux x64 · candidat expérimental'],
    ['DinoTale-Tornie-0.34.0-beta.1-Sources.zip',en?'Source code':'Code source'],
    ['DinoTale-Tornie-Guide-FR-EN-0.34.0-beta.1.zip',en?'Offline FR/EN guide':'Guide FR/EN hors ligne'],
    ['SHA256SUMS.txt',en?'SHA256 checksums':'Empreintes SHA256']
  ];
  target.replaceChildren(...links.map(([file,label])=>{const a=document.createElement('a');a.href=base+file;a.textContent=label;a.className='button';return a;}));
  target.style.cssText='display:flex;flex-wrap:wrap;gap:10px';
})();
