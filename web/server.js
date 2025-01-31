const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const cors = require('cors');
const path = require('path');

const app = express();
const server = http.createServer(app);
const io = socketIo(server, {
    cors: {
        origin: '*',
    }
});

app.use(express.static('public'));

const port = 3000;

// use -key in run.sh on the client to authorize
let key = "l33ts3cr3t"; // default hardcoded to l33ts3cr3t
// this can be set on client with -pw param, it'll set the working url
let endpoint = "secret"; // default hardcoded to be /secret

const args = process.argv.slice(2); // Skip the first two elements

const keyIndex = args.indexOf("-key");
if (keyIndex !== -1 && keyIndex + 1 < args.length) {
    key = args[keyIndex + 1];
}

let data = [];

app.use(express.json());
app.use(cors());

app.post('/receiver', (req, res) => {
    data = req.body;

    if (data.host && data.host.key === key) {
        io.emit('entityUpdate', data);
        if (endpoint != data.host.endpoint && data.host.endpoint != "") {
            endpoint = data.host.endpoint;
            console.log("valid endpoint updated");
        }
        res.status(200).send("data received");
    } else {
        data = [];
        res.status(401).send("unauthorized");
    }
});

app.get('/:ep', (req, res) => {
    const { ep } = req.params;
    if (ep === endpoint) {
        res.sendFile(path.join(__dirname, 'html', 'info.html'));
    } else {
        res.sendFile(path.join(__dirname, 'html', '404.html'));
    }
});

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'html', 'index.html'));
});

server.listen(port, () => {
    console.log(`server: running on port ${port}`);
    console.log(`server: post key set to "${key}"`);
});
