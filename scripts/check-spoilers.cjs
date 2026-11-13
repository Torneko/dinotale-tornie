'use strict';
// Runs the actual site script against a small DOM/event fixture; no renderer or browser.
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../website');
const code = fs.readFileSync(path.join(root, 'assets/guide.js'), 'utf8');
let checks = 0;
const check = (condition, message) => { checks++; assert.ok(condition, message); };
const equal = (a, b, message) => { checks++; assert.deepEqual(a, b, message); };
const plain = html => html.replace(/<[^>]*>/g, ' ').replace(/\s+/g, ' ').trim();

class Element {
  constructor(id, attrs = {}) {
    this.id = id;
    this.attrs = {...attrs};
    this.hidden = false;
    this.textContent = '';
    this.value = '';
    this.listeners = new Map();
    this.classes = new Set();
    this.classList = {
      remove: name => this.classes.delete(name),
      toggle: (name, force) => {
        const enabled = force === undefined ? !this.classes.has(name) : !!force;
        if (enabled) this.classes.add(name); else this.classes.delete(name);
        return enabled;
      }
    };
  }
  addEventListener(type, callback) {
    const callbacks = this.listeners.get(type) || [];
    callbacks.push(callback);
    this.listeners.set(type, callbacks);
  }
  dispatch(type) {
    const event = {target: this, prevented: false, preventDefault() {this.prevented = true;}};
    for (const callback of this.listeners.get(type) || []) callback(event);
    return event;
  }
  setAttribute(name, value) {this.attrs[name] = value;}
  getAttribute(name) {return this.attrs[name] ?? null;}
  focus() {this.focused = true;}
  scrollIntoView() {this.scrolled = true;}
  get hash() {return this.attrs.href?.startsWith('#') ? this.attrs.href : '';}
}

function boot(file, hash = '') {
  const html = fs.readFileSync(path.join(root, file), 'utf8');
  const gated = file.startsWith('guide');
  const lang = html.match(/<html lang="(.*?)"/)[1];
  const templateHTML = gated ? html.match(/<template id="spoiler-content">([\s\S]*?)<\/template>/)[1] : html;
  const sectionData = [...templateHTML.matchAll(/<section class="section" id="([^"]+)">([\s\S]*?)<\/section>/g)]
    .map(match => ({id: match[1], text: plain(match[2])}));
  const ids = new Map();
  const permanent = new Set();
  const put = (id, attrs) => {const element = new Element(id, attrs); ids.set(id, element); return element;};
  const menu = put('menu', {'aria-expanded': 'false'});
  menu.hidden = gated;
  const href = html.match(/id="language" href="([^"]+)"/)[1];
  const language = put('language', {href});
  permanent.add('menu'); permanent.add('language');
  let sections = [], links = [], sidebar, observerCount = 0, disconnects = 0;
  function uninstall() {
    for (const id of ids.keys()) if (!permanent.has(id)) ids.delete(id);
    sections = []; links = []; sidebar = undefined;
  }
  function install() {
    sidebar = put('sidebar');
    put('search'); put('search-count'); const empty = put('empty'); empty.hidden = true;
    sections = sectionData.map(data => {const node = put(data.id); node.textContent = data.text; return node;});
    links = sections.map(section => new Element('', {href: '#' + section.id}));
    if (gated) put('spoiler-hide');
  }
  let gate, body, template, reveal;
  if (gated) {
    gate = put('spoiler-gate');
    const title = put('spoiler-title'); title.textContent = lang === 'en' ? 'Full guide — spoilers' : 'Guide complet — spoilers';
    body = put('spoiler-body'); body.hidden = true;
    reveal = put('spoiler-reveal', {'aria-expanded': 'false'});
    template = put('spoiler-content');
    [gate, title, body, reveal, template].forEach(node => permanent.add(node.id));
    template.content = {cloneNode(deep) {check(deep, 'Copy the complete inert guide'); return {guideClone: true};}};
    body.replaceChildren = clone => {uninstall(); if (clone) {check(clone.guideClone, 'Insert only the inert guide'); install();}};
    body.contains = node => sections.includes(node);
    body.querySelector = selector => selector === 'h2' ? sections[0] : null;
  } else install();
  const windowEvents = new Map();
  const location = {hash, href: file + hash};
  const document = {
    documentElement: {lang},
    querySelector(selector) {return selector === '.sidebar' ? sidebar ?? null : ids.get(selector.slice(1)) ?? null;},
    querySelectorAll(selector) {return selector === '.section' ? sections : selector === '.sidebar nav a' ? links : [];},
    getElementById(id) {return ids.get(id) ?? null;}
  };
  class Observer {
    constructor() {observerCount++;}
    observe() {}
    disconnect() {disconnects++;}
  }
  const window = {
    location,
    matchMedia() {return {matches: true};},
    IntersectionObserver: Observer,
    addEventListener(type, callback) {
      const callbacks = windowEvents.get(type) || [];
      callbacks.push(callback); windowEvents.set(type, callbacks);
    }
  };
  vm.runInNewContext(code, {document, window, IntersectionObserver: Observer}, {filename: 'guide.js', timeout: 1000});
  return {
    file, lang, ids, menu, language, gate, body, reveal, location,
    get sections() {return sections;}, get links() {return links;},
    get sidebar() {return sidebar;}, get disconnects() {return disconnects;},
    get observerCount() {return observerCount;},
    event(type, values = {}) {for (const callback of windowEvents.get(type) || []) callback(values);}
  };
}

function expectClosed(page, reason) {
  check(page.body.hidden, reason + ': body hidden');
  equal(page.sections.length, 0, reason + ': no detailed sections in live document');
  check(!page.ids.has('search'), reason + ': spoiler text not searchable');
  check(!page.ids.has('yoshis'), reason + ': fragment cannot target a live spoiler');
  check(page.menu.hidden, reason + ': hidden guide menu');
  equal(page.reveal.getAttribute('aria-expanded'), 'false', reason + ': collapsed semantics');
  check(!page.reveal.hidden, reason + ': explicit action remains available');
}

for (const file of ['index.html', 'index-en.html']) {
  const page = boot(file, '#downloads');
  equal(page.sections.length, 8, 'Public sections work without opening a guide');
  check(!page.gate, 'No detailed guide present on homepage');
  page.menu.dispatch('click');
  equal(page.menu.getAttribute('aria-expanded'), 'true', 'Mobile menu opens');
  page.links[0].dispatch('click');
  equal(page.menu.getAttribute('aria-expanded'), 'false', 'Mobile navigation closes menu');
  const search = page.ids.get('search');
  search.value = file === 'index.html' ? 'ÉTAT' : 'STATUS'; search.dispatch('input');
  check(!page.ids.get('updates').hidden, 'Case and accents ignored by section search');
  check(page.ids.get('install').hidden, 'Nonmatching section filtered');
  search.value = 'no-such-rubric-zzzz'; search.dispatch('input');
  check(!page.ids.get('empty').hidden, 'No-results status shown');
  search.value = ''; search.dispatch('input');
  check(page.sections.every(section => !section.hidden), 'Clearing search restores sections');
  check(page.ids.get('empty').hidden, 'Empty status reset');
  equal(page.ids.get('search-count').textContent, '', 'Search count resets');
  check(page.language.dispatch('click').prevented, 'Language navigation preserves explicit destination');
  equal(page.location.href, (file === 'index.html' ? 'index-en.html' : 'index.html') + '#downloads');
}

for (const file of ['guide.html', 'guide-en.html']) {
  for (const hash of ['', '#yoshis', '#roadmap', '#spoiler-title', '#%broken']) {
    const page = boot(file, hash);
    expectClosed(page, 'Initial visit ' + file + hash);
    page.event('pageshow', {persisted: true});
    expectClosed(page, 'Restored visit stays closed');
    page.event('hashchange');
    expectClosed(page, 'Hash is not consent');
    page.reveal.dispatch('click');
    check(!page.body.hidden && page.reveal.hidden, 'Explicit acknowledgement reveals the guide');
    equal(page.sections.length, 7, 'All detailed sections appear after consent');
    equal(page.reveal.getAttribute('aria-expanded'), 'true', 'Expanded semantics updated');
    check(!page.menu.hidden, 'Guide navigation becomes available');
    const target = hash === '#yoshis' ? page.ids.get('yoshis') : hash === '#roadmap' ? page.ids.get('roadmap') : page.sections[0];
    check(target.focused && target.scrolled, 'Requested section receives focus only after consent');
    page.ids.get('search').value = 'no-such-rubric-zzzz'; page.ids.get('search').dispatch('input');
    check(page.sections.every(section => section.hidden), 'Detailed search runs only after consent');
    page.ids.get('spoiler-hide').dispatch('click');
    expectClosed(page, 'User hides the guide');
    check(page.reveal.focused, 'Hide returns keyboard focus to acknowledgement button');
    page.reveal.dispatch('click');
    check(page.sections.every(section => !section.hidden), 'Reopening creates fresh search state');
    page.event('pagehide', {persisted: true});
    expectClosed(page, 'Clear before back-cache snapshot');
    page.reveal.dispatch('click');
    page.event('pageshow', {persisted: true});
    expectClosed(page, 'Clear on history restoration');
    page.reveal.dispatch('click');
    page.language.dispatch('click');
    equal(page.location.href, (file === 'guide.html' ? 'guide-en.html' : 'guide.html') + hash, 'Language link preserves requested section');
    const other = boot(file === 'guide.html' ? 'guide-en.html' : 'guide.html', hash);
    expectClosed(other, 'Other language requires a new acknowledgement');
    check(page.disconnects >= 3 && page.observerCount === 4, 'Observers are disconnected between guide visits');
  }
}
console.log(JSON.stringify({status: 'PASS', checks, scope: 'actual guide.js DOM-event fixture; no browser, GPU, network or storage', scenarios: ['FR/EN home/search/mobile', 'initial fragments', 'explicit reveal', 'keyboard focus', 'hide', 'pageshow/pagehide', 'language revisit']}));
