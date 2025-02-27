const socket = io();

function saveSteam() {
    const steam64Value = document.getElementById('steam64').value;
    if (steam64Value.trim() !== '') {
        localStorage.setItem('steam64', steam64Value);
    }
}

function getSavedSteam() {
    const steam64Value = localStorage.getItem('steam64');
    if (steam64Value) {
        document.getElementById('steam64').value = steam64Value;
    }
}

window.onload = getSavedSteam;

function assemble() {
    const existingTable = document.body.querySelector('table');

    if (existingTable) {
        return;
    }

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

    document.body.appendChild(table);
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

    // find the targets
    entities.forEach(entity => {
        if (entity.health > 0 && (entity.team == 2 || entity.team == 3) && entity.team != host.team) {

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
            // there is likely a smarter way to do this .
            distCell.textContent = specExists ? ((specHealth > 0) ? dist(specPos, entity.pos) : '-') : ((host.health > 0) ? dist(host.pos, entity.pos) : '-');
            row.appendChild(distCell);

            const flagsCell = document.createElement('td');
            flagsCell.textContent = entity.flags || '-';
            row.appendChild(flagsCell);

            tableBody.appendChild(row);
        }
    });
});
