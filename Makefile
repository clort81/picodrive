PLATFORM=FREMANTLE
GAME_VERSION=1.36
DESTDIR="/usr/" 

ifeq "$(PLATFORM)" "DIABLO"
MAKEMENU=@echo ignoring
else
MAKEMENU=make -f Makefile -C platform/maemo/menu -k
endif
all:
	make -f Makefile -C platform/maemo -k all -I/usr/include/gtk-2.0 PLATFORM=$(PLATFORM) GAME_VERSION=$(GAME_VERSION)
	$(MAKEMENU) all GAME_VERSION=$(GAME_VERSION)
install:
	make -f Makefile -C platform/maemo -k install -I/usr/include/gtk-2.0  DESTDIR=$(DESTDIR) GAME_VERSION=$(GAME_VERSION)
	$(MAKEMENU) install DESTDIR=$(DESTDIR) GAME_VERSION=$(GAME_VERSION)
clean:
	make -f Makefile -C platform/maemo -k clean
	$(MAKEMENU) clean 
