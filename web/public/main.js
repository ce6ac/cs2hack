const socket = io();

function saveSettings() {
    const steam64Input = document.getElementById('steam64');
    if (steam64Input) {
        const steam64Value = steam64Input.value;
        if (steam64Value.trim() !== '') {
            localStorage.setItem('steam64', steam64Value);
        }
    }

    const playerlist = document.getElementById('playerlist');
    if (playerlist) {
        const position = {
            left: playerlist.style.left,
            top: playerlist.style.top,
            width: playerlist.style.width
        };
        localStorage.setItem('playerlist', JSON.stringify(position));
    }

    const radarbox = document.getElementById('radarbox');
    if (radarbox) {
        const position = {
            left: radarbox.style.left,
            top: radarbox.style.top,
            size: radarbox.style.width
        };
        localStorage.setItem('radarbox', JSON.stringify(position));
    }

    const settingsbox = document.getElementById('settingsbox');
    if (settingsbox) {
        const position = {
            left: settingsbox.style.left,
            top: settingsbox.style.top,
            width: settingsbox.style.width
        };
        localStorage.setItem('settingsbox', JSON.stringify(position));
    }
}

function getSettings() {
    const steam64Value = localStorage.getItem('steam64');
    if (steam64Value) {
        const steam64 = document.getElementById('steam64');
        if (steam64) steam64.value = steam64Value;
    }
}

window.onload = getSettings;

let topZIndex = 10;
let lastUpdateAt = 0;
let avgLatencyMs = 0;
let latencySamples = 0;
let offlineTimer = null;

function bringToFront(element) {
    if (!element) return;
    topZIndex += 1;
    element.style.zIndex = String(topZIndex);
}

function registerFloatingBox(container, handle = null) {
    if (!container) return;

    if (!container.style.zIndex) {
        bringToFront(container);
    }

    container.addEventListener('mousedown', () => bringToFront(container));

    if (handle && handle !== container) {
        handle.addEventListener('mousedown', () => bringToFront(container));
    }
}

function updateLatencyDisplay(text) {
    const el = document.getElementById('latencyValue');
    if (!el) return;
    el.textContent = text;
}

function resetOfflineTimer() {
    if (offlineTimer) clearTimeout(offlineTimer);

    offlineTimer = setTimeout(() => {
        updateLatencyDisplay('no connection');
        lastUpdateAt = 0;
        avgLatencyMs = 0;
        latencySamples = 0;
    }, 3000);
}

function assemble() {
    const existingPlayerList = document.getElementById('playerlist');
    if (existingPlayerList) return;

    const container = document.createElement('div');
    container.id = 'playerlist';
    container.style.position = 'absolute';

    const savedSettings = localStorage.getItem('playerlist');
    if (savedSettings) {
        const { left, top, width } = JSON.parse(savedSettings);
        if (left) container.style.left = left;
        if (top) container.style.top = top;
        if (width) container.style.width = width;
    }

    const title = document.createElement('div');
    title.className = 'title';
    title.textContent = 'playerlist';

    const resizer = document.createElement('div');
    resizer.className = 'resizer';

    const h1 = document.querySelector('h1');
    const paragraph = document.querySelector('p');

    if (h1) h1.remove();
    if (paragraph) paragraph.remove();

    const table = document.createElement('table');
    const thead = document.createElement('thead');
    const headerRow = document.createElement('tr');

    const headers = ['name', 'gun', 'hp', 'location', 'dist', 'flags'];
    headers.forEach(headerText => {
        const header = document.createElement('th');
        header.textContent = headerText;
        headerRow.appendChild(header);
    });

    thead.appendChild(headerRow);
    table.appendChild(thead);

    const tbody = document.createElement('tbody');
    tbody.id = 'entityTableBody';
    table.appendChild(tbody);

    container.appendChild(title);
    container.appendChild(table);
    container.appendChild(resizer);
    document.body.appendChild(container);

    makeDraggable(container, title);
    makeResizableHorizontal(container, resizer, 400);
    registerFloatingBox(container, title);
}

function assembleRadar() {
    const existingRadar = document.getElementById('radarbox');
    if (existingRadar) return;

    const radarbox = document.createElement('div');
    radarbox.id = 'radarbox';

    const savedSettings = localStorage.getItem('radarbox');
    if (savedSettings) {
        const { left, top, size } = JSON.parse(savedSettings);
        if (left) radarbox.style.left = left;
        if (top) radarbox.style.top = top;
        if (size) {
            radarbox.style.width = size;
            radarbox.style.height = size;
        }
    }

    const title = document.createElement('div');
    title.className = 'title';
    title.textContent = 'radar';

    const canvas = document.createElement('canvas');
    canvas.id = 'radarCanvas';

    const resizer = document.createElement('div');
    resizer.className = 'resizer';

    radarbox.appendChild(title);
    radarbox.appendChild(canvas);
    radarbox.appendChild(resizer);
    document.body.appendChild(radarbox);

    makeDraggable(radarbox, title);
    makeResizableSquare(radarbox, resizer, 180);
    registerFloatingBox(container, title);
    syncRadarCanvasSize();
}

function assembleSettings() {
    const existingSettings = document.getElementById('settingsbox');
    if (existingSettings) return;

    const settingsbox = document.createElement('div');
    settingsbox.id = 'settingsbox';
    settingsbox.style.position = 'absolute';
    settingsbox.style.width = '275px';
    settingsbox.style.left = 'calc(100% - 232px)';
    settingsbox.style.top = 'calc(100% - 90px)';

    const savedSettings = localStorage.getItem('settingsbox');
    if (savedSettings) {
        const { left, top, width } = JSON.parse(savedSettings);
        if (left) settingsbox.style.left = left;
        if (top) settingsbox.style.top = top;
        if (width) settingsbox.style.width = width;
    }

    const title = document.createElement('div');
    title.className = 'title';
    title.textContent = 'settings';

    const content = document.createElement('div');
    content.className = 'settings-content';
    content.style.padding = '8px';
    content.style.display = 'flex';
    content.style.gap = '8px';
    content.style.alignItems = 'center';

    const input = document.createElement('input');
    input.type = 'text';
    input.id = 'steam64';
    input.placeholder = 'steam64';
    input.style.flex = '1';
    input.style.minWidth = '0';

    const savedSteam64 = localStorage.getItem('steam64');
    if (savedSteam64) {
        input.value = savedSteam64;
    }

    const button = document.createElement('button');
    button.textContent = 'save';
    button.onclick = saveSettings;

    content.appendChild(input);
    content.appendChild(button);

    settingsbox.appendChild(title);
    settingsbox.appendChild(content);
    document.body.appendChild(settingsbox); assembleSettings();

    makeDraggable(settingsbox, title);
    registerFloatingBox(settingsbox, title);
}

function syncRadarCanvasSize() {
    const radarbox = document.getElementById('radarbox');
    const canvas = document.getElementById('radarCanvas');
    if (!radarbox || !canvas) return;

    const title = radarbox.querySelector('.title');
    const titleHeight = title ? title.offsetHeight : 31;
    const size = Math.max(100, radarbox.clientWidth);
    const canvasSize = Math.max(100, size - titleHeight);

    canvas.width = canvasSize;
    canvas.height = canvasSize;
}

function makeDraggable(container, handle) {
    let isDragging = false;

    handle.onmousedown = function (event) {
        event.preventDefault();
        isDragging = true;

        bringToFront(container);

        const rect = container.getBoundingClientRect();
        container.style.left = `${rect.left}px`;
        container.style.top = `${rect.top}px`;
        container.style.right = 'auto';
        container.style.bottom = 'auto';

        const header = document.querySelector('.bar');
        const headerHeight = header ? header.offsetHeight - 1 : 0;

        let shiftX = event.clientX - rect.left;
        let shiftY = event.clientY - rect.top;

        function moveAt(pageX, pageY) {
            let newX = pageX - shiftX;
            let newY = pageY - shiftY;

            newX = Math.max(0, Math.min(window.innerWidth - container.offsetWidth, newX));
            newY = Math.max(headerHeight, Math.min(window.innerHeight - container.offsetHeight, newY));

            container.style.left = `${newX}px`;
            container.style.top = `${newY}px`;
        }

        function onMouseMove(event) {
            if (!isDragging) return;
            moveAt(event.pageX, event.pageY);
        }

        function stopDragging() {
            isDragging = false;
            document.removeEventListener('mousemove', onMouseMove);
            document.removeEventListener('mouseup', stopDragging);
        }

        document.addEventListener('mousemove', onMouseMove);
        document.addEventListener('mouseup', stopDragging);

        document.addEventListener('keydown', function escHandler(event) {
            if (event.key === 'Escape') {
                stopDragging();
                document.removeEventListener('keydown', escHandler);
            }
        });
    };

    handle.ondragstart = function () {
        return false;
    };
}

function makeResizableHorizontal(container, handle, minWidth = 400) {
    let isResizing = false;

    handle.onmousedown = function (event) {
        event.preventDefault();
        isResizing = true;
        let startX = event.clientX;
        let startWidth = container.offsetWidth;

        function onMouseMove(event) {
            if (!isResizing) return;
            let newWidth = startWidth + (event.clientX - startX);

            const maxWidth = window.innerWidth - container.offsetLeft;
            if (newWidth >= minWidth && newWidth < maxWidth) {
                container.style.width = `${newWidth}px`;
            }
        }

        function stopResizing() {
            isResizing = false;
            document.removeEventListener('mousemove', onMouseMove);
            document.removeEventListener('mouseup', stopResizing);
        }

        document.addEventListener('mousemove', onMouseMove);
        document.addEventListener('mouseup', stopResizing);

        document.addEventListener('keydown', function escHandler(event) {
            if (event.key === 'Escape') {
                stopResizing();
                document.removeEventListener('keydown', escHandler);
            }
        });
    };
}

function makeResizableSquare(container, handle, minSize = 180) {
    let isResizing = false;

    handle.onmousedown = function (event) {
        event.preventDefault();
        event.stopPropagation();
        isResizing = true;

        const startX = event.clientX;
        const startY = event.clientY;
        const startSize = container.offsetWidth;

        function onMouseMove(event) {
            if (!isResizing) return;

            const dx = event.clientX - startX;
            const dy = event.clientY - startY;
            const delta = Math.max(dx, dy);
            let newSize = startSize + delta;

            const maxWidth = window.innerWidth - container.offsetLeft;
            const maxHeight = window.innerHeight - container.offsetTop;
            const maxSize = Math.min(maxWidth, maxHeight);

            newSize = Math.max(minSize, Math.min(maxSize, newSize));

            container.style.width = `${newSize}px`;
            container.style.height = `${newSize}px`;

            syncRadarCanvasSize();
        }

        function stopResizing() {
            isResizing = false;
            document.removeEventListener('mousemove', onMouseMove);
            document.removeEventListener('mouseup', stopResizing);
        }

        document.addEventListener('mousemove', onMouseMove);
        document.addEventListener('mouseup', stopResizing);

        document.addEventListener('keydown', function escHandler(event) {
            if (event.key === 'Escape') {
                stopResizing();
                document.removeEventListener('keydown', escHandler);
            }
        });
    };
}

function reset() {
    const container = document.getElementById('playerlist');
    const radarbox = document.getElementById('radarbox');
    const settingsbox = document.getElementById('settingsbox');

    const header = document.querySelector('.bar');
    const headerHeight = header ? header.offsetHeight - 1 : 0;

    if (container) {
        const currentLeft = parseInt(container.style.left || 0, 10);
        const currentTop = parseInt(container.style.top || 0, 10);

        const maxLeft = Math.max(0, window.innerWidth - container.offsetWidth);
        const maxTop = Math.max(headerHeight, window.innerHeight - container.offsetHeight);

        container.style.left = `${clamp(currentLeft, 0, maxLeft)}px`;
        container.style.top = `${clamp(currentTop, headerHeight, maxTop)}px`;
    }

    if (radarbox) {
        if (window.innerHeight <= 400) {
            radarbox.style.display = 'none';
        } else {
            radarbox.style.display = 'flex';

            const currentLeft = parseInt(radarbox.style.left || 0, 10);
            const currentTop = parseInt(radarbox.style.top || 0, 10);

            const maxLeft = Math.max(0, window.innerWidth - radarbox.offsetWidth);
            const maxTop = Math.max(headerHeight, window.innerHeight - radarbox.offsetHeight);

            radarbox.style.left = `${clamp(currentLeft, 0, maxLeft)}px`;
            radarbox.style.top = `${clamp(currentTop, headerHeight, maxTop)}px`;

            syncRadarCanvasSize();
        }
    }

    if (settingsbox) {
        const currentLeft = parseInt(settingsbox.style.left || 0, 10);
        const currentTop = parseInt(settingsbox.style.top || 0, 10);

        const maxLeft = Math.max(0, window.innerWidth - settingsbox.offsetWidth);
        const maxTop = Math.max(headerHeight, window.innerHeight - settingsbox.offsetHeight);

        settingsbox.style.left = `${clamp(currentLeft, 0, maxLeft)}px`;
        settingsbox.style.top = `${clamp(currentTop, headerHeight, maxTop)}px`;
    }
}

function dist(pos1, pos2) {
    const dx = pos2[0] - pos1[0];
    const dy = pos2[1] - pos1[1];
    const dz = pos2[2] - pos1[2];
    const distance = Math.sqrt(dx * dx + dy * dy + dz * dz) * 0.01905;
    return Math.round(distance * 10) / 10;
}

function clamp(value, min, max) {
    return Math.max(min, Math.min(max, value));
}

function drawRadar(centerEntity, entities) {
    const canvas = document.getElementById('radarCanvas');
    if (!canvas) return;

    syncRadarCanvasSize();

    const ctx = canvas.getContext('2d');
    const w = canvas.width;
    const h = canvas.height;
    const cx = w / 2;
    const cy = h / 2;
    const outerRadius = Math.min(w, h) / 2 - 12;

    ctx.clearRect(0, 0, w, h);

    ctx.fillStyle = 'rgb(44, 44, 44)';
    ctx.fillRect(0, 0, w, h);

    ctx.beginPath();
    ctx.arc(cx, cy, outerRadius, 0, Math.PI * 2);
    ctx.strokeStyle = 'rgb(85, 85, 85)';
    ctx.stroke();

    ctx.beginPath();
    ctx.arc(cx, cy, outerRadius * 0.33, 0, Math.PI * 2);
    ctx.arc(cx, cy, outerRadius * 0.66, 0, Math.PI * 2);
    ctx.strokeStyle = 'rgb(55, 55, 55)';
    ctx.stroke();

    ctx.beginPath();
    ctx.moveTo(cx, cy - outerRadius);
    ctx.lineTo(cx, cy + outerRadius);
    ctx.moveTo(cx - outerRadius, cy);
    ctx.lineTo(cx + outerRadius, cy);
    ctx.strokeStyle = 'rgb(65, 65, 65)';
    ctx.stroke();

    if (!centerEntity || !centerEntity.pos || centerEntity.health <= 0) {
        ctx.fillStyle = 'white';
        ctx.font = '14px Arial';
        ctx.fillText('dead/no player', 5, 20);
        return;
    }

    ctx.beginPath();
    ctx.arc(cx, cy, 5, 0, Math.PI * 2);
    ctx.fillStyle = '#ffffff';
    ctx.fill();

    const range = 2500.0;

    const yawDeg = Array.isArray(centerEntity.eyes) ? centerEntity.eyes[1] || 0 : 0;
    const yawRad = (yawDeg - 90) * Math.PI / 180;

    const cosYaw = Math.cos(-yawRad);
    const sinYaw = Math.sin(-yawRad);

    entities.forEach(entity => {
        if (!entity.pos || entity.health <= 0) return;
        if (entity.steam === centerEntity.steam) return;
        if (entity.team !== 2 && entity.team !== 3) return;

        const dx = entity.pos[0] - centerEntity.pos[0];
        const dy = entity.pos[1] - centerEntity.pos[1];

        // Rotate into centerEntity local view space
        const localX = dx * cosYaw - dy * sinYaw;
        const localY = dx * sinYaw + dy * cosYaw;

        let rx = (localX / range) * outerRadius;
        let ry = (localY / range) * outerRadius;

        const len = Math.sqrt(rx * rx + ry * ry);
        if (len > outerRadius - 4) {
            const scale = (outerRadius - 4) / len;
            rx *= scale;
            ry *= scale;
        }

        const x = cx + rx;
        const y = cy - ry;

        const isEnemy = entity.team !== centerEntity.team;

        ctx.beginPath();
        ctx.arc(x, y, 4, 0, Math.PI * 2);
        ctx.fillStyle = isEnemy ? '#ff4d4d' : '#4da6ff';
        ctx.fill();

        if (entity.flags && entity.flags.includes('spotted')) {
            ctx.beginPath();
            ctx.arc(x, y, 7, 0, Math.PI * 2);
            ctx.strokeStyle = '#ffd24d';
            ctx.stroke();
        }
    });
}

const ep = window.location.pathname.slice(1);

socket.on(ep, (data) => {
    assemble();
    assembleRadar();
    assembleSettings();

    const specSteam = document.getElementById('steam64').value.trim();
    const entities = Array.isArray(data.entities) ? data.entities : [];
    const host = data.host || {};

    let specExists = false;
    let specEntity = null;

    for (const entity of entities) {
        if (specSteam !== '' && specSteam !== '0' && String(specSteam) === String(entity.steam)) {
            specExists = true;
            specEntity = entity;
            break;
        }
    }

    const tableBody = document.getElementById('entityTableBody');
    tableBody.innerHTML = '';

    let targets = 0;

    entities.forEach(entity => {
        if (entity.health > 0 && (entity.team === 2 || entity.team === 3) && host.team !== entity.team) {
            const row = document.createElement('tr');

            const nameCell = document.createElement('td');
            if (entity.steam != 0) {
                const link = document.createElement('a');
                link.textContent = entity.name || '-';
                link.href = `https://steamcommunity.com/profiles/${entity.steam}`;
                link.target = '_blank';
                nameCell.appendChild(link);
            } else {
                nameCell.textContent = entity.name || '-';
            }
            row.appendChild(nameCell);

            const gunCell = document.createElement('td');
            gunCell.textContent = entity.gun || '-';
            row.appendChild(gunCell);

            const healthCell = document.createElement('td');
            healthCell.textContent = entity.health || '-';
            row.appendChild(healthCell);

            const locCell = document.createElement('td');
            locCell.textContent = entity.loc || '-';
            row.appendChild(locCell);

            const distCell = document.createElement('td');
            distCell.textContent = specExists
                ? ((specEntity.health > 0) ? dist(specEntity.pos, entity.pos) : '-')
                : '-';
            row.appendChild(distCell);

            const flagsCell = document.createElement('td');
            flagsCell.textContent = entity.flags || '-';
            row.appendChild(flagsCell);

            tableBody.appendChild(row);
            targets++;
        }
    });

    if (targets === 0) {
        const row = document.createElement('tr');
        const nameCell = document.createElement('td');
        nameCell.textContent = 'no alive players';
        row.appendChild(nameCell);
        tableBody.appendChild(row);
    }

    drawRadar(specExists ? specEntity : null, entities);

    const now = performance.now();

    if (lastUpdateAt !== 0) {
        const dt = now - lastUpdateAt;

        latencySamples = Math.min(latencySamples + 1, 200);
        avgLatencyMs = avgLatencyMs === 0
            ? dt
            : (avgLatencyMs * (latencySamples - 1) + dt) / latencySamples;

        updateLatencyDisplay(`${Math.round(avgLatencyMs)} ms`);
    }

    lastUpdateAt = now;
    resetOfflineTimer();
});

window.addEventListener('resize', () => {
    reset();
    syncRadarCanvasSize();
});