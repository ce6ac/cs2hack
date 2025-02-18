## cs2hack

cs2 game hack including info esp and aim/trigger powered by input through qmp

a lot could have been better, but since it works just fine and does not get detected on certain 3rd party platforms depending on qemu/kvm setup, i decided to just throw this project on github.

### credits
- [ko1N](https://github.com/ko1N), [h33p](https://github.com/h33p) and [emesare](https://github.com/emesare) - memflow
- [MisterY52](https://github.com/MisterY52) - apex kvm ware
- [AtomicBool](https://github.com/AtomicBool) - cs2 kvm ware (his implementation of memflow)
- [nlohmann](https://github.com/nlohmann) - json
- [a2x](https://github.com/a2x) - cs2 offset dumper
- h3rx - qemouse (qmp wrapper)
- [horror](https://github.com/horrified-dev) - all inspiration to get into qemu/kvm gaming ;)

## setup 
to get setup, we're assuming you have a qemu/kvm gaming setup already

#### web
go into the "web" folder and run the following to get all packages required
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
head into build folder, modify run.sh to configure
```
cd build
nano run.sh
```
(for aimbot and triggerbot) configure qmp for mouse input you'll need to modify your vm xml
```
<domain>
  ...
  <qemu:commandline>
    <qemu:arg value="-qmp"/>
    <qemu:arg value="tcp:127.0.0.1:6448,server,nowait"/>
  </qemu:commandline>
</domain>
```
finally, run "run.sh" script
```
./run.sh
```
triggerbot key is your use key in-game, to bind it use cs2 command
```
bind X +use
```
