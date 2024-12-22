## cs2-knower

cs2 project for getting game data and displaying it on a web page

a lot could have been better, but since it works just fine and does not get detected on certain 3rd party platforms depending on qemu/kvm setup, i decided to just throw this project on github

### credits
- [ko1N](https://github.com/ko1N), [h33p](https://github.com/h33p) and [emesare](https://github.com/emesare) - memflow
- [MisterY52](https://github.com/MisterY52) - apex kvm ware
- [AtomicBool](https://github.com/AtomicBool) - cs2 kvm ware (his implementation of memflow)
- [nlohmann](https://github.com/nlohmann/json) - json
- viking - all inspiration to get into qemu/kvm gaming ;)

## setup 
to get setup, we're assuming you have a qemu/kvm gaming setup already.

#### web
didn't npm init so you'll just go into the "web" folder and run the following to get all packages required.
```
npm install express http socket.io cors path
```
then start the server
```
node server.js
```

#### client
to build the client, run the "build.sh" script to build everything including memflow
```
./build.sh
```
then head into the build folder and run "run.sh" script
```
cd build
./run.sh
```
(if needed, modify run.sh with ```-url``` parameter for cs2hack to use a custom url)
