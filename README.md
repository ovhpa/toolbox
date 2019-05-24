# toolbox

Toolbox is some place where I will put some scripts and/or some small programs for use with theoretical chemistry softwares.
I keep them here, before using them somewhere else.

License is GPLv3 by default see [here](LICENSE)

## Compile

1/ Select or create a `make.xxx` in the arch folder.
2/ replace the line that says `include arch/make.gcc.linux` with your `make.xxx` file:
```
include arch/make.xxx
```
3/ type:
```
make all
```
In the main directory (where the `Makefile` file is).

## Install

There is no installation target yet, just copy the files where you can execute it.
Here are the following programs and script:


disp\_vibr.bash: create a xsf file with the animation of each vibration modes from a VASP calculation.
get\_lvl.bash: display some interesting level information for each k-point of a VASP calculation.

uspex\_chem: read USPEX Individuals file and plot a convex hull, using external reference in a small file, `chem.in` if available.


## Develop

A debug` target is available for developing purpose.
Using `make debug` will automatically turn the development mode.
A GLIB target is also available (with its debug target) for portability.
Use `make glib` for the former and `make dglib` for the latter.

## CHANGELOG

v0.0.1 initial version. almost nothing to see there.


## TODO

- [x] Initial pre-release
- [ ] Get a workable v0.1.0
- [ ] Prepare a v1.0.0
- [ ] Stop making lists.

    
-- OVHPA (2019)
