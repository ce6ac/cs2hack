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

// set these two when setting up
let key = "set_a_key"; // use -key in run.sh on the client to authorize
let password = "set_a_pw"; // this can be set on client with -pw param, it'll set the working url

let data = [];

app.use(express.json());
app.use(cors());

app.post('/receiver', (req, res) => {
    data = req.body;

    if (data.host && data.host.key === key) {
        io.emit('entityUpdate', data);
        if (password != data.host.password) {
            password = data.host.password;
            console.log("pw changed");
        }
        res.status(200).send("data received\n");
    } else {
        data = [];
        res.status(401).send("unauthorized\n");
    }
});

app.get('/:pw', (req, res) => {
    const { pw } = req.params;
    if (pw === password) {
        res.sendFile(path.join(__dirname, 'html', 'info.html'));
    } else {
        res.sendFile(path.join(__dirname, 'html', '404.html'));
    }
});

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'html', 'index.html'));
});

server.listen(port, () => {
    console.log(`server running on port ${port}`);
});
