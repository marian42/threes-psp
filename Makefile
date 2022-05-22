TARGET = Threes
OBJS = img/cards.c main.o
 
CFLAGS = -O2
CXXFLAGS = $(CFLAGS) -std=c++14 -fno-rtti -fno-exceptions
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -lpspgum -lpspgu
 
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Threes
PSP_EBOOT_ICON = ICON0.PNG 
 
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

img/cards.c : img/cards.raw
		bin2c img/cards.raw img/cards.c cards