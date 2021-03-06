TARGET = Threes
OBJS = img/cards.c img/spritesheet.c img/icon0.c text.o ThreesGame.o input.o Application.o utils.o ThreesGrid.o main.o
 
CFLAGS = -O2
CXXFLAGS = $(CFLAGS) -std=c++14 -fno-rtti -fno-exceptions
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -lpspgum -lpspgu -lm
 
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Threes
PSP_EBOOT_ICON = img/ICON0.PNG 
 
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

img/cards.c : img/cards.raw
		bin2c img/cards.raw img/cards.c cards
img/spritesheet.c : img/spritesheet.raw
		bin2c img/spritesheet.raw img/spritesheet.c spritesheet
img/icon0.c : img/ICON0.PNG
		bin2c img/ICON0.PNG img/icon0.c icon0