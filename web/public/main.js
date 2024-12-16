const socket = io();

function saveSteam() {
    const steam64Value = document.getElementById('steam64').value;
    if (steam64Value.trim() !== "") {
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
    const existingTable = document.querySelector('table');

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

socket.on('entityUpdate', (entities) => {
    const showall = document.getElementById('showall').checked;
    const spec_steam = document.getElementById('steam64').value;
    let spec_exists = false;
    let spec_team;
    let spec_health;
    let spec_pos;

    // find the spectator
    for (const entity of entities) {
        if (spec_steam == entity.steam && spec_steam != 0) {
            spec_exists = true;
            spec_team = entity.team;
            spec_health = entity.health;
            spec_pos = entity.pos;
            break;
        }
        else
            spec_exists = false;
    }

    if (spec_exists) {
        assemble();
        const tableBody = document.getElementById('entityTableBody');
        tableBody.innerHTML = '';

        // find the targets
        entities.forEach(entity => {
            if (entity.health > 0 && (entity.team == 2 || entity.team == 3)) {

                if (!showall && spec_team == entity.team) 
                    return;

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
                distCell.textContent = (spec_health > 0) ? dist(spec_pos, entity.pos) : '-';
                row.appendChild(distCell);

                const flagsCell = document.createElement('td');
                flagsCell.textContent = entity.flags || '-';
                row.appendChild(flagsCell);

                tableBody.appendChild(row);
            }
        });
    } else {
        const h1 = document.querySelector('h1');
        const paragraph = document.querySelector('p');
        if (h1 && paragraph) {
            h1.textContent = "online";
            paragraph.textContent = "steam64 from the game required to see info."
        }
    }
});
