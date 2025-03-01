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
let postKey = ''; // set this so only you and those you trust can send data to the server

const args = process.argv.slice(2); // skip the first two elements

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
        return res.status(200).send('data received');
    } else {
        data = [];
        console.log('server: someone is trying to send data with a bad key');
        return res.status(401).send('unauthorized, make sure post key is correct');
    }
});

app.post('/testkey', (req, res) => {
    let data = req.body;

    if (!postKey) {
        console.log('server: someone tested key but no key is set on server');
        return res.status(200).send('good, no key set on server');
    }

    if (data.key === postKey) {
        console.log('server: someone tested key with result good');
        return res.status(200).send('good key');
    } else {
        console.log('server: someone tested key with result bad');
        return res.status(401).send('bad key');
    }
});

app.get('/:ep', (req, res) => {
    return res.sendFile(path.join(__dirname, 'html', 'info.html'));
});

app.get('/', (req, res) => {
    return res.sendFile(path.join(__dirname, 'html', 'index.html'));
});

server.listen(port, () => {
    console.log(`server: running on port ${port}`);
    if (postKey) {
        console.log(`server: post key set to '${postKey}'`);
    }
});
