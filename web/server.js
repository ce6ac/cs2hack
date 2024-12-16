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

const port = 4000;

let currentEntities = [];
let lastUpdateTime = null;

// Middleware to parse JSON bodies
app.use(express.json());
app.use(cors());

app.post('/receiver', (req, res) => {
    currentEntities = req.body;

    io.emit('entityUpdate', currentEntities);

    console.log("data received");

    res.status(200).send("good");
});

app.get('/info', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'info.html'));
});

server.listen(port, () => {
    console.log(`Server running on port ${port}`);
});
