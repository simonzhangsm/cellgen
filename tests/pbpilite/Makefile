DIRS = spu  
PROGRAM_ppu = pbpilite 

IMPORTS = spu/lib_pbpilite_spu.a -lpthread -lspe2 -lm -lc -lmisc -lnuma
      
CFLAGS = -O3 -ggdb -DUNIX -fomit-frame-pointer -DPROFILING
include /opt/cell/sdk/make.footer
