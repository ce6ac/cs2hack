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
let postKey = 'l33ts3cr3t'; // default hardcoded to l33ts3cr3t - create a better solution later
// this can be set on client with -ep argument when running cs2hack, it'll set the working url
let endpoint = 'secret'; // default hardcoded to be /secret

const args = process.argv.slice(2); // Skip the first two elements

const keyIndex = args.indexOf('-key');
if (keyIndex !== -1 && keyIndex + 1 < args.length) {
    postKey = args[keyIndex + 1];
}

let data = [];

app.use(express.json());
app.use(cors());

app.post('/receiver', (req, res) => {
    data = req.body;

    if (data.host && data.host.key === postKey) {
        io.emit('entityUpdate', data);
        if (endpoint != data.host.endpoint && data.host.endpoint != '') {
            endpoint = data.host.endpoint;
            console.log('server: valid endpoint updated');
        }
        res.status(200).send('data received');
    } else {
        data = [];
        res.status(401).send('unauthorized, make sure post key is correct');
        console.log('server: someone is trying to send data with a bad key');
    }
});

app.post('/testkey', (req, res) => {
    let data = req.body;

    if (data.key === postKey) {
        res.status(200).send('good key');
        console.log('server: someone tested key with result good');
    } else {
        res.status(401).send('bad');
        console.log('server: someone tested key with result bad');
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
    if (postKey) {
        console.log(`server: post key set to '${postKey}'`);
    }
});
