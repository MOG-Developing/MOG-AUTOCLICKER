document.addEventListener('DOMContentLoaded', () => {
    const navLinks = document.querySelectorAll('.nav-links a');
    const pages = document.querySelectorAll('.page');
    const latestMount = document.getElementById('latest-mount');
    const archiveMount = document.getElementById('archive-mount');
    const showMoreBtn = document.getElementById('show-more');
    const canvas = document.getElementById('canvas');
    const ctx = canvas.getContext('2d');
    
    let particles = [];
    function resize() {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    class Particle {
        constructor() { this.init(); }
        init() {
            this.x = Math.random() * canvas.width;
            this.y = Math.random() * canvas.height;
            this.size = Math.random() * 2;
            this.speedY = Math.random() * 0.3 + 0.1;
            this.opacity = Math.random() * 0.5;
        }
        update() {
            this.y -= this.speedY;
            if (this.y < -10) this.y = canvas.height + 10;
        }
        draw() {
            ctx.fillStyle = `rgba(255, 95, 95, ${this.opacity})`;
            ctx.beginPath();
            ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
            ctx.fill();
        }
    }
    function initBg() {
        particles = [];
        for (let i = 0; i < 50; i++) particles.push(new Particle());
    }
    function animate() {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        particles.forEach(p => { p.update(); p.draw(); });
        requestAnimationFrame(animate);
    }
    window.addEventListener('resize', () => { resize(); initBg(); });
    resize(); initBg(); animate();

    let allReleases = [];
    let displayedCount = 4;

    async function fetchReleases() {
        if (!latestMount) return;
        try {
            const response = await fetch('https://api.github.com/repos/MOG-Developing/MOG-AUTOCLICKER/releases');
            if (!response.ok) throw new Error('Repository unavailable');
            allReleases = await response.json();
            renderReleases();
        } catch (error) {
            if (latestMount) latestMount.innerHTML = `<div class="loading">Error: ${error.message}</div>`;
        }
    }

    function renderReleases() {
        if (allReleases.length === 0) {
            if (latestMount) latestMount.innerHTML = '<div class="loading">No releases found.</div>';
            return;
        }
        const latest = allReleases[0];
        if (latestMount) latestMount.innerHTML = createReleaseHTML(latest, true);
        const older = allReleases.slice(1);
        renderArchive(older);
    }

    function renderArchive(older) {
        const toDisplay = older.slice(0, displayedCount);
        if (archiveMount) {
            archiveMount.innerHTML = toDisplay.map(r => createReleaseHTML(r, false)).join('');
            showMoreBtn.style.display = (older.length > displayedCount) ? 'block' : 'none';
        }
    }

    function createReleaseHTML(release, isLatest) {
        const date = new Date(release.published_at).toLocaleDateString('en-GB', {
            day: 'numeric', month: 'short', year: 'numeric'
        });

        const assets = release.assets.map(a => `
            <a href="${a.browser_download_url}" class="asset-row" target="_blank">
                <span class="asset-name">${a.name}</span>
                <div class="asset-meta">
                    <span>Size: <span class="meta-val">${(a.size / 1024 / 1024).toFixed(2)} MB</span></span>
                    <span>Downloads: <span class="meta-val">${a.download_count}</span></span>
                </div>
            </a>
        `).join('');

        const sourceLinks = `
            <a href="${release.zipball_url}" class="asset-row" target="_blank">
                <span class="asset-name">Source Code (zip)</span>
                <div class="asset-meta"><span>Type: <span class="meta-val">Source</span></span></div>
            </a>
            <a href="${release.tarball_url}" class="asset-row" target="_blank">
                <span class="asset-name">Source Code (tar.gz)</span>
                <div class="asset-meta"><span>Type: <span class="meta-val">Source</span></span></div>
            </a>
        `;

        return `
            <div class="release-card ${isLatest ? 'latest-card' : ''}">
                <div class="release-info">
                    <span class="version-badge">${release.tag_name}</span>
                    <span class="release-date">${date}</span>
                </div>
                <div class="asset-list">
                    ${assets}
                    ${sourceLinks}
                </div>
                <a href="${release.html_url}" class="github-link" target="_blank">View on GitHub →</a>
            </div>
        `;
    }

    if (showMoreBtn) {
        showMoreBtn.addEventListener('click', () => {
            displayedCount += 5;
            renderReleases();
        });
    }

    if (latestMount) {
        fetchReleases();
    }
});
