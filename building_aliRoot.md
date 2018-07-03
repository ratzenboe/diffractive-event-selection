# Building the Alice-Framwork
To read more about how to get updates for `AliRoot` and `AliPhysics` as well as information on how to make changes and on the other hand download other peoples changes go to [Darios tutorial page](https://dberzano.github.io/alice/alibuild/) where also most of this tutorial is from.
If we want to rebuild only what changed go to section see 
[here](Also://dberzano.github.io/alice/alibuild/#rebuild_using_alibuild)
## Getting Started
- First we have to install `alibuild` via pip
```
pip install alibuild
```
- Then create a directory where all the Alice software will be contained.
```
mkdir ~/aliSoftware
cd ~/aliSoftware
```
The advantage is now that if we want to completely get rid of this alice-framework we just remove this folder
- we now initialize a directory with an arbitrary name, here `ali-master`
```
aliBuild init AliRoot,AliPhysics -z ali-master
```
## Checking the system setup
> Packages installed on Ubuntu with `sudo apt-get install package-name`  
> However it is in this case where many packages have to be installed it is easier to use `synapitc`.   
> `synaptic` is a GUI program and also shows which libraries are installed alongside the desired program.

Here comes the fun part, yay. Before we acturally tell aliBuild to build the software we have to check that everything on our
machine is in order and that we can pick up as many externals as possible from the
system. From `~/aliSoftware` We first go 
to `ali-master` (or the name you put after
`-z`) and then invoke the `aliDoctor`
```
cd ali-master
aliDoctor AliPhysics
```
This will examine the system and provide suggestions (in the form of warnings) or actual errors about the system. We must fix
at least the errors before starting.
- Packages which i had to install
    - curl
    - opengl
    - yacc-like

If you scroll up to the `ERROR:` messages you have to search for these uninstalled
packages. There you then see a description
of what you have to do. Sometimes you are faced with two option
1. `RHEL-compatible systems`
2. `Ubuntu-compatible system`
Choose the packages corresponding to you operating system.
For me it was `Ubuntu` so the `ERROR` message for `curl` told me to install
    - `curl`
    - `libcurl4-openssl-dev` (or `libcurl4-gnutls-dev`)

The same procedure is true for `yacc`. The error message tells me that `yacc-like` cannot be found. Please install `bison` and `flex` development packages. Here we have only one option for all operating systems. **Always check the `ERROR` messages carefully!**
- so install
    - flex
    - bison

At last `opengl` is needed, here the `ERROR` provided again system specific
information what to install
    - libglu1-mesa-dev

If every error message is scanned and the
packages which are asked to install are
installed then we can try again
```
aliDoctor AliPhysics
```
If there are no more `ERROR` messages
appearing and we dont have a section
```
==> The following packages are system dependencies and could not be found:
```
we can start to build.

## Actually building the alice framework
To build AliRoot and AliPhysics we execute the following in the build directory `~/aliSoftware/ali-master`
```
aliBuild -z -w ../sw -d build AliPhysics --disable DPMJET,GEANT4_VMC,fastjet
```
- `-z` tells `alibuild` to append the name of this directory, here `ali-master` to AliRoot and AliPhysics. This way,
AliRoot/Physics are created using the source code contained in `ali-master`. Useful when loading the environment.
- `-w ../sw` is the shared `alibuild` working directory.
Everything will be installed there. **Do not manually change the contents of the directory.**
- `-d` debug mode, finding errors is easier
- `--disable` packages that we do not use and take forever to build

## Environment handling
After building is done we execute the following lines
which write to the `~/.bashrc` file to make loading the 
environment in the current shell more easily.
```
echo ALICE_WORK_DIR=$HOME/aliSoftware/sw >> ~/.bashrc
echo 'eval "`alienv shell-helper`"' >> ~/.bashrc
```
To enter a new shell with the correct environment loaded, do
```
alienv enter AliPhysics/latest-ali-master-release
```
Or better, set a convenient `alias` in your `~/.bashrc` we 
put these lines with a convenient name (here `alienter`) in there
```
alias alienter='alienv enter AliPhysics/latest-ali-master-release'
```

## Common errors
An error that occured nearly every time we build the framework is related to `autotools`. As soon as `autotools` is installed on the pc prior to building `AliPhysics` we get an error. An easy fix is to just remove `autotools` with all its dependancies and let `alibuild` install it alongside the whole build. (In my case this fixed the problem in 100% of all cases!)

## Update the software
To get the newest verion of the software we only have to pull the `HEAD` from the master branch. If we made changes to the software which we want to keep but don't want to influence the update we first `stash` them with
```
git stash
```
Now the working directory should be clean and we can fetch the newset changes
```
cd ~/aliSoftware/ali-master/AliPhysics
git pull --rebase
cd ../AliRoot
git pull --rebase
cd .. 
aliBuild -z -w ../sw -d build AliPhysics --disable DPMJET,GEANT4_VMC,fastjet
```
These few lines of code will update `AliRoot` and `AliPhysics` to the newest version and then build the new changes.

### Reverting to a certain version
We can list the availabe versions via `git tag -l`. If we have found the version we want to switch to *e.g* 20180222 
we do
```
cd ~/aliSoftware/ali-master/AliPhysics
git pull --rebase origin vAN-20180222
cd .. 
aliBuild -z -w ../sw -d build AliPhysics --disable DPMJET,GEANT4_VMC,fastjet
```

### Updating local changes
If local chages were available which *e.g.* have been stashed prior to the update and are now back again we have to make these changes present in the software again. To do this go to folder where the changes have been made (here: **diffractive** case) and invoke `make` and `make install`
```
cd ~/aliSoftware/sw/BUILD/AliPhysics-latest/AliPhysics/PWGUD/DIFFRACTIVE` 
make
make install
```
This updates the local changes which are made in the 
`~/aliSoftware/sw/BUILD/AliPhysics-latest/AliPhysics/PWGUD/DIFFRACTIVE/*` folder & subfolders.
