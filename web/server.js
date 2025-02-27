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

const args = process.argv.slice(2); // Skip the first two elements

const keyIndex = args.indexOf('-key');
if (keyIndex !== -1 && keyIndex + 1 < args.length) {
    postKey = args[keyIndex + 1];
}

app.use(express.json());
app.use(cors());

app.post('/receiver', (req, res) => {
    let data = req.body;

    if (data.host && data.host.endpoint && data.host.key === postKey) {
        io.emit(data.host.endpoint, data);
        console.log(`server: data received for /${data.host.endpoint}`);
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
        res.status(401).send('bad key');
        console.log('server: someone tested key with result bad');
    }
});

app.get('/:ep', (req, res) => {
    res.sendFile(path.join(__dirname, 'html', 'info.html'));
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
