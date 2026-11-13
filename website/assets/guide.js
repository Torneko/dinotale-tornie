(() => {
  'use strict';
  const en = document.documentElement.lang === 'en';
  const menu = document.querySelector('#menu');
  const language = document.querySelector('#language');
  const gate = document.querySelector('#spoiler-gate');
  const body = document.querySelector('#spoiler-body');
  const template = document.querySelector('#spoiler-content');
  const reveal = document.querySelector('#spoiler-reveal');
  let observer;
  const fold = text => text.normalize('NFD').replace(/[\u0300-\u036f]/g, '').toLowerCase();
  const closeMenu = () => {
    document.querySelector('.sidebar')?.classList.remove('open');
    menu?.setAttribute('aria-expanded', 'false');
  };

  function wireContent() {
    observer?.disconnect();
    const search = document.querySelector('#search');
    const counter = document.querySelector('#search-count');
    const empty = document.querySelector('#empty');
    const sections = [...document.querySelectorAll('.section')];
    const links = [...document.querySelectorAll('.sidebar nav a')];
    search?.addEventListener('input', () => {
      const words = fold(search.value).trim().split(/\s+/).filter(Boolean);
      let visible = 0;
      sections.forEach(section => {
        section.hidden = !words.every(word => fold(section.textContent).includes(word));
        if (!section.hidden) visible++;
      });
      links.forEach(link => {
        const section = document.getElementById(link.hash.slice(1));
        link.hidden = !!section?.hidden;
      });
      counter.textContent = words.length
        ? visible + (en ? ' sections found' : ' rubriques trouvées') : '';
      empty.hidden = visible !== 0;
    });
    links.forEach(link => link.addEventListener('click', () => {
      if (window.matchMedia('(max-width:720px)').matches) closeMenu();
    }));
    if ('IntersectionObserver' in window) {
      observer = new IntersectionObserver(entries => entries.forEach(entry => {
        if (entry.isIntersecting) links.forEach(link =>
          link.classList.toggle('active', link.hash === '#' + entry.target.id));
      }), {rootMargin: '-100px 0px -65%'});
      sections.forEach(section => observer.observe(section));
    }
  }

  menu?.addEventListener('click', () => {
    const sidebar = document.querySelector('.sidebar');
    if (!sidebar) return;
    menu.setAttribute('aria-expanded', String(sidebar.classList.toggle('open')));
  });
  language?.addEventListener('click', event => {
    event.preventDefault();
    // A fragment is only a requested destination; it never grants spoiler consent.
    window.location.href = language.getAttribute('href') + window.location.hash;
  });

  if (gate && body && template && reveal) {
    function closeGuide(returnFocus = false) {
      observer?.disconnect();
      body.replaceChildren();
      body.hidden = true;
      reveal.hidden = false;
      reveal.setAttribute('aria-expanded', 'false');
      menu.hidden = true;
      closeMenu();
      if (returnFocus) reveal.focus();
    }
    reveal.addEventListener('click', () => {
      // No saved consent, cookies or browser storage: every visit starts closed.
      body.replaceChildren(template.content.cloneNode(true));
      body.hidden = false;
      reveal.hidden = true;
      reveal.setAttribute('aria-expanded', 'true');
      menu.hidden = false;
      wireContent();
      document.querySelector('#spoiler-hide').addEventListener('click', () => closeGuide(true));
      let requestedId;
      try { requestedId = decodeURIComponent(window.location.hash.slice(1)); } catch { requestedId = ''; }
      const requested = document.getElementById(requestedId);
      const target = requested && body.contains(requested) ? requested : body.querySelector('h2');
      if (target) {
        target.setAttribute('tabindex', '-1');
        target.focus({preventScroll: true});
        target.scrollIntoView({block: 'start'});
      }
    });
    closeGuide();
    // Clear both fresh pages and browser-back snapshots. Hash links stay inert.
    window.addEventListener('pagehide', () => closeGuide());
    window.addEventListener('pageshow', () => closeGuide());
  } else {
    wireContent();
  }
})();
