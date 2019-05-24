# (Simple) Makefile for toolbox
PREFIX=/usr/local
CURR_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

VPATH = src:mod_src:obj:modules

# (arch) defs
include arch/make.gcc.linux

#OBJECTS: .o and .mod for regular object; .obj and .dmod for debug ones
#OBJ = modules.o help.o
#OBJ_DBG = modules.obj help.obj
#MODULES
#MOD_OBJ = mod_test.mod mod_math.mod
#MOD_OBJ_DBG = mod_test.dmod mod_math.dmod


all: uspex_chem
debug: uspex_chem_dbg
glib: uspex_gchem
dglib: uspex_gchem_dbg


uspex_chem:
	$(CC) $(INCFLAGS) $(CFLAGS) $(OPTFLAGS) ./src/uspex_chem.c -o ./uspex_chem $(LDFLAGS) $(LIBS)
uspex_chem_dbg:
	$(CC) $(INCFLAGS) $(CFLAGS) $(DBGFLAGS) ./src/uspex_chem.c -o ./uspex_chem_dbg $(LDFLAGS) $(LIBS)
uspex_gchem:
	$(CC) $(INCFLAGS) $(CFLAGS) $(OPTFLAGS) $(GCFLAGS) ./src/uspex_chem.c -o ./uspex_gchem $(DBGLDFLAGS) $(LIBS) $(GLFLAGS)
uspex_gchem_dbg:
	$(CC) $(INCFLAGS) $(CFLAGS) $(DBGFLAGS) $(GCFLAGS) ./src/uspex_chem.c -o ./uspex_gchem_dbg $(DBGLDFLAGS) $(LIBS) $(GLFLAGS)

.SUFFIXES:
.SUFFIXES:      .c .cc .C .cpp .obj .o .mod .dmod

.c.o :
	$(CC) -o obj/$@ -c $(CFLAGS) $(OPTFLAGS) $< $(INCFLAGS)
.c.obj :
	$(CC) -o obj/$@ -c $(CFLAGS) $(DBGFLAGS) $< $(INCFLAGS)
.c.mod :
	$(CC) -fPIC -shared -o modules/$@ $(CFLAGS) $(OPTFLAGS) $< $(INCFLAGS)
.c.dmod :
	$(CC) -fPIC -shared -o modules/$@ $(CFLAGS) $(DBGFLAGS) $< $(INCFLAGS)

.FORCE:

count:
	wc src/*.c scripts/* include/*.h include/*.def

clean:
	rm -f ./uspex_chem
	rm -f ./uspex_chem_dbg
	rm -f ./uspex_gchem
	rm -f ./uspex_gchem_dbg


.PHONY: .FORCE
.PHONY: all
.PHONY: debug
.PHONY: glib
.PHONY: dglib
.PHONY: count
.PHONY: clean
