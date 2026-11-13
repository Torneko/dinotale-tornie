'use strict';
// Run the actual release renderer without a browser or network.
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const assert = require('node:assert/strict');
const code = fs.readFileSync(path.join(__dirname, '../website/assets/release.js'), 'utf8');
const version = '0.35.5-beta.3';
const base = 'https://github.com/Torneko/dinotale-tornie/releases/download/v' + version + '/';
const expected = ['DinoTale-Tornie-' + version + '-Windows-x64.zip', 'DinoTale-Tornie-' + version + '-Linux-x64.tar.gz', 'DinoTale-Tornie-' + version + '-Sources.zip', 'DinoTale-Tornie-Guide-FR-EN-' + version + '.zip', 'SHA256SUMS.txt'];
let checks = 0;
for (const lang of ['fr', 'en']) {
  for (const hasTarget of [true, false]) {
    const links = [], target = {style: {}, replaceChildren(...children) { links.push(...children); }};
    const window = {}, document = {documentElement: {lang}, querySelector(selector) { assert.equal(selector, '#release-downloads'); return hasTarget ? target : null; }, createElement(tag) { assert.equal(tag, 'a'); return {}; }};
    vm.runInNewContext(code, {window, document}, {filename: 'release.js', timeout: 1000});
    assert.equal(window.DINOTALE_RELEASE.version, version); checks++;
    for (const flag of ['available', 'prerelease', 'linuxExperimental']) { assert.equal(window.DINOTALE_RELEASE[flag], true); checks++; }
    assert.deepEqual(links.map(link => link.href), hasTarget ? expected.map(file => base + file) : []); checks++;
    for (const link of links) { assert.ok(link.textContent.trim()); assert.equal(link.className, 'button'); checks += 2; }
    if (hasTarget) {
      assert.match(links[1].textContent, /expérimental|experimental/); checks++;
      assert.match(links[2].textContent, /spoilers/); checks++;
      assert.match(links[3].textContent, /spoilers/); checks++;
      assert.match(target.style.cssText, /flex-wrap:wrap/); checks++;
    }
  }
}
console.log(JSON.stringify({status: 'PASS', checks, version, downloadsPerLanguage: 5, scope: 'actual release.js in FR/EN DOM fixture; no browser or network'}));
