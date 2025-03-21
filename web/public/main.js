const socket = io();

function saveSettings() {
    const steam64Value = document.getElementById('steam64').value;
    if (steam64Value.trim() !== '') {
        localStorage.setItem('steam64', steam64Value);
    }

    const container = document.getElementById('playerlist');
    if (container) {
        const position = {
            left: container.style.left,
            top: container.style.top,
            width: container.style.width
        };
        localStorage.setItem('playerlist', JSON.stringify(position)); // Save position & width
    }
}

function getSettings() {
    const steam64Value = localStorage.getItem('steam64');
    if (steam64Value) {
        document.getElementById('steam64').value = steam64Value;
    }
}

window.onload = getSettings;

function assemble() {
    const existingPlayerList = document.getElementById('playerlist');

    if (existingPlayerList) {
        return;
    }

    const container = document.createElement('div');
    container.id = 'playerlist';

    const savedSettings = localStorage.getItem('playerlist');
    if (savedSettings) {
        const { left, top, width } = JSON.parse(savedSettings);
        if (left && top) {
            container.style.left = left;
            container.style.top = top;
        }
        if (width) {
            container.style.width = width;
        }
    }

    const title = document.createElement('div');
    title.className = 'title';
    title.textContent = 'playerlist';

    const resizer = document.createElement('div');
    resizer.className = 'resizer';

    const h1 = document.querySelector('h1');
    const paragraph = document.querySelector('p');

    if (h1) {
        h1.remove();
    }
    if (paragraph) {
        paragraph.remove();
    }

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
    makeResizable(container, resizer);
}

function makeDraggable(container, handle) {
    let isDragging = false;

    handle.onmousedown = function (event) {
        event.preventDefault();
        isDragging = true;

        const headerHeight = document.querySelector('.bar').offsetHeight - 1; // get header height

        let shiftX = event.clientX - container.getBoundingClientRect().left;
        let shiftY = event.clientY - container.getBoundingClientRect().top;

        function moveAt(pageX, pageY) {
            let newX = pageX - shiftX;
            let newY = pageY - shiftY;

            // limits
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
            if (event.key === "Escape") {
                stopDragging();
                document.removeEventListener('keydown', escHandler);
            }
        });
    };

    handle.ondragstart = function () {
        return false;
    };
}

function makeResizable(container, handle) {
    let isResizing = false;

    handle.onmousedown = function (event) {
        event.preventDefault();
        isResizing = true;
        let startX = event.clientX;
        let startWidth = container.offsetWidth;

        function onMouseMove(event) {
            if (!isResizing) return;
            let newWidth = startWidth + (event.clientX - startX);

            // limits
            let maxWidth = window.innerWidth - container.offsetLeft;
            if (newWidth >= 400 && newWidth < maxWidth) {
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
            if (event.key === "Escape") {
                stopResizing();
                document.removeEventListener('keydown', escHandler);
            }
        });
    };
}

function reset() {
    let container = document.getElementById('playerlist');

    if (container && window.innerHeight <= 400 && container.hasAttribute('style')) {
        container.removeAttribute('style');
    }
    if (container && window.innerWidth <= 940 && container.hasAttribute('style')) {
        container.removeAttribute('style');
    }
}

function dist(pos1, pos2) {
    const dx = pos2[0] - pos1[0];
    const dy = pos2[1] - pos1[1];
    const dz = pos2[2] - pos1[2];
    const distance = Math.sqrt(dx * dx + dy * dy + dz * dz) * 0.01905;
    return Math.round(distance * 10) / 10;
}

// get the page path and remove the slash
const ep = window.location.pathname.slice(1);

socket.on(ep, (data) => {
    const specSteam = document.getElementById('steam64').value;
    const entities = data.entities;
    const host = data.host;

    let specExists = false;
    let specTeam;
    let specHealth;
    let specPos;

    // find the spectator
    for (const entity of entities) {
        if (specSteam == entity.steam && specSteam != 0) {
            specExists = true;
            specTeam = entity.team;
            specHealth = entity.health;
            specPos = entity.pos;
            break;
        } else {
            specExists = false;
        }
    }

    assemble(); // assemble the table if it does not exist

    const tableBody = document.getElementById('entityTableBody');
    tableBody.innerHTML = '';

    let targets = 0;

    // find the targets
    entities.forEach(entity => {
        if (entity.health > 0 && (entity.team == 2 || entity.team == 3) && host.team != entity.team) {

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
            distCell.textContent = specExists ? ((specHealth > 0) ? dist(specPos, entity.pos) : '-') : '-';
            row.appendChild(distCell);

            const flagsCell = document.createElement('td');
            flagsCell.textContent = entity.flags || '-';
            row.appendChild(flagsCell);

            tableBody.appendChild(row);

            targets++;
        }
    });
    if (targets == 0) {
        const row = document.createElement('tr');
        const nameCell = document.createElement('td');
        nameCell.textContent = 'no alive players';
        row.appendChild(nameCell);
        tableBody.appendChild(row);
    }
});

window.addEventListener('resize', reset);