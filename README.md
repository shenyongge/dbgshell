# gdb shell
a gdb shell that non-block. shell can call the function, display the gobal variable, read and update the memory for the debugee, and also execute command on the host.

# git clone
used the flowing command to clone project to local host.
```
git clone --recursive https://github.com/shenyongge/gdbshell.git
or
git clone https://github.com/shenyongge/gdbshell.git
git submodule update --init --recursive
```
use the flowing command to add a submodule.
```
git submodule add <remote-url> <local-path>
```

# make and run
## make
```
cd dir
make all
```
## run injecter
```
first:run dummy,and get the pid.
second: run injecter [pid of dummy], injiecter will inject dummy and patch function.
```
## run gdbshell
```
first: run gdbshell
second: telnet the listern port, and run commands!
```
