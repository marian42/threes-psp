TARGET = Threes
OBJS = cards.c main.o
 
CFLAGS = 
CXXFLAGS = $(CFLAGS) -std=c++14 -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -lpspgum -lpspgu
 
# PSP Stuff
BUILD_PRX = 1
PSP_FW_VERSION = 500
PSP_LARGE_MEMORY = 1
 
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Threes
PSP_EBOOT_ICON = ICON0.PNG 
 
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

cards.c : cards.raw
		bin2c cards.raw cards.c cards