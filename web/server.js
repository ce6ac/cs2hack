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

let entities = [];

app.use(express.json());
app.use(cors());

app.post('/receiver', (req, res) => {
    entities = req.body;

    io.emit('entityUpdate', entities);

    console.log("data received");

    res.status(200).send("good");
});

app.get('/info', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'info.html'));
});

server.listen(port, () => {
    console.log(`server running on port ${port}`);
});
