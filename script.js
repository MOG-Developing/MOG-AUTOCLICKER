(() => {
    const n = document.querySelector.bind(document);
    const a = document.querySelectorAll.bind(document);
    
    let lastScroll = 0;
    const scrollThreshold = 50;
    
    const handleScroll = () => {
        const currentScroll = window.scrollY;
        const nav = n('nav');
        
        if (Math.abs(currentScroll - lastScroll) < scrollThreshold) return;
        
        if (currentScroll > lastScroll && currentScroll > 100) {
            nav.classList.add('hidden');
        } else {
            nav.classList.remove('hidden');
        }
        
        lastScroll = currentScroll;
        
        const s = window.scrollY;
        a('.nav-links a').forEach(l => {
            const h = n(l.getAttribute('href'));
            const o = h.offsetTop - 100;
            const b = o + h.offsetHeight;
            l.classList.toggle('active', s >= o && s < b);
        });
        
        updateScrollProgress();
    };
    
    const updateScrollProgress = () => {
        const winScroll = document.body.scrollTop || document.documentElement.scrollTop;
        const height = document.documentElement.scrollHeight - document.documentElement.clientHeight;
        const scrolled = (winScroll / height) * 100;
        n('.scroll-progress').style.transform = `scaleX(${scrolled / 100})`;
    };
    
    window.addEventListener('scroll', handleScroll);
    
    a('.nav-links a').forEach(l => {
        l.addEventListener('click', e => {
            e.preventDefault();
            const t = n(l.getAttribute('href'));
            t.scrollIntoView({ behavior: 'smooth' });
        });
    });
    
    const animateStats = () => {
        a('.stat-number').forEach(stat => {
            const target = parseInt(stat.getAttribute('data-target'));
            const duration = 2000;
            const step = target / (duration / 16);
            let current = 0;
            
            const updateStat = () => {
                current += step;
                if (current < target) {
                    stat.textContent = Math.floor(current).toLocaleString();
                    requestAnimationFrame(updateStat);
                } else {
                    stat.textContent = target.toLocaleString();
                }
            };
            
            updateStat();
        });
    };
    
    const o = new IntersectionObserver(
        es => es.forEach(e => {
            if (e.isIntersecting) {
                e.target.classList.add('visible');
                if (e.target.classList.contains('stats-container')) {
                    animateStats();
                }
            }
        }),
        { threshold: 0.1 }
    );
    
    [
        '.feature-card',
        '.step',
        '.version-card',
        '.stat-card',
        '.testimonial-card',
        '.changelog-item',
        '.faq-item',
        '.support-card'
    ].forEach(s => a(s).forEach(e => o.observe(e)));
    
    const faqItems = a('.faq-item');
    faqItems.forEach(item => {
        item.addEventListener('click', () => {
            const wasActive = item.classList.contains('active');
            faqItems.forEach(i => i.classList.remove('active'));
            if (!wasActive) {
                item.classList.add('active');
            }
        });
    });
    
    const createParticles = () => {
        const particles = document.createElement('div');
        particles.className = 'particles';
        document.body.appendChild(particles);
        
        for (let i = 0; i < 50; i++) {
            const particle = document.createElement('div');
            particle.className = 'particle';
            particle.style.setProperty('--x', `${Math.random() * 100}vw`);
            particle.style.setProperty('--y', `${Math.random() * 100}vh`);
            particle.style.setProperty('--duration', `${Math.random() * 20 + 10}s`);
            particle.style.setProperty('--delay', `-${Math.random() * 20}s`);
            particles.appendChild(particle);
        }
    };
    
    createParticles();
    
    const encodeSourceMap = () => {
        const scripts = a('script');
        scripts.forEach(script => {
            if (script.src) {
                const encoded = btoa(script.src);
                script.setAttribute('data-src', encoded);
                script.removeAttribute('src');
            }
        });
    };
    
    const obfuscateStyles = () => {
        const styles = a('link[rel="stylesheet"]');
        styles.forEach(style => {
            if (style.href) {
                const encoded = btoa(style.href);
                style.setAttribute('data-href', encoded);
                style.removeAttribute('href');
            }
        });
    };
    
    document.addEventListener('DOMContentLoaded', () => {
        encodeSourceMap();
        obfuscateStyles();
    });
    
    const preventInspect = () => {
        document.addEventListener('contextmenu', e => e.preventDefault());
        document.addEventListener('keydown', e => {
            if (e.ctrlKey && (e.key === 'u' || e.key === 's')) {
                e.preventDefault();
            }
        });
    };
    
    preventInspect();
    
    const shuffleText = text => {
        const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
        return text.split('').map(char => 
            chars[Math.floor(Math.random() * chars.length)]
        ).join('');
    };
    
    const obfuscateContent = () => {
        const elements = a('*');
        elements.forEach(el => {
            if (el.childNodes.length === 1 && el.childNodes[0].nodeType === 3) {
                const originalText = el.textContent;
                el.setAttribute('data-text', btoa(originalText));
                el.textContent = shuffleText(originalText);
                setTimeout(() => {
                    el.textContent = originalText;
                }, 100);
            }
        });
    };
    
    setInterval(obfuscateContent, 30000);
    
    const initializeAnimations = () => {
        const elements = a('.animate-on-scroll');
        elements.forEach(el => {
            el.style.opacity = '0';
            el.style.transform = 'translateY(20px)';
        });
        
        const animate = () => {
            elements.forEach(el => {
                const rect = el.getBoundingClientRect();
                const isVisible = rect.top < window.innerHeight - 100;
                
                if (isVisible) {
                    el.style.opacity = '1';
                    el.style.transform = 'translateY(0)';
                }
            });
        };
        
        window.addEventListener('scroll', animate);
        animate();
    };
    
    initializeAnimations();
})();