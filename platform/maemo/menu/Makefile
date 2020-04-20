PKGS:=gtk+-2.0 hildon-1 hildon-fm-2 libhildonmime gconf-2.0 x11
CPPFLAGS:=$(shell pkg-config --cflags $(PKGS))
LDLIBS:=$(shell pkg-config --libs $(PKGS))

-include config.mk

# Some hardcoded paths
GAME_PLUGIN_PATH=/usr/lib/picodrive_plugin.so
GAME_CONF_PATH=~/.config/picodrive/picodrive.conf
GAME_BIN_PATH=/usr/bin/picodrive
GAME_BANNER_PATH=/usr/share/pixmaps/osso-games-startup-picodrive.png
MAEMO_VERSION=5
OSSO_GAMES_STARTUP=/usr/bin/osso_games_startup

# The about dialog will use this
CPPFLAGS+=-DGAME_VERSION=$(GAME_VERSION)
LDFLAGS=-shared

GETTEXT_PACKAGE:=picodrive_plugin
ifeq ($(CONF_NLS), 1)
	CPPFLAGS += -DCONF_NLS=1 -DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\"
endif

M4:=m4
M4DEFS:= -DGAME_VERSION=$(GAME_VERSION) -DGAME_PLUGIN_PATH=$(GAME_PLUGIN_PATH)
M4DEFS+= -DGAME_CONF_PATH=$(GAME_CONF_PATH) -DGAME_BIN_PATH=$(GAME_BIN_PATH)
M4DEFS+= -DGAME_BANNER_PATH=$(GAME_BANNER_PATH)
ifdef MAEMO_VERSION
M4DEFS+= -DMAEMO -DMAEMO_MAJOR=$(MAEMO_VERSION)
endif

XGETTEXT:=xgettext
XGETTEXT_OPTS:=--default-domain=$(GETTEXT_PACKAGE) --from-code=UTF-8 \
	--language=C --msgid-bugs-address=maemo@javispedro.com \
	--package-name=PicoDrive --package-version=$(GAME_VERSION) \
	--keyword=_ --keyword=N_
EXTRA_GETTEXT_SRCS:=buttons.inc

SRCS:=plugin.c state.c save.c controls.c keys.c settings.c about.c \
	 cellrendererkey.c
OBJS:=$(SRCS:.c=.o)

POT:=i18n/$(GETTEXT_PACKAGE).pot
PO_FILES:=$(wildcard i18n/*.po)
MO_FILES:=$(PO_FILES:.po=.mo)
LANGS:=$(basename $(notdir $(PO_FILES)))

DATA_FILES:=picodrive.conf picodrive.desktop
DATA_FILES+=picodrive.service picodrive.startup.service
DATA_FILES+=picodrive.schemas

all: picodrive_plugin.so $(DATA_FILES) $(MO_FILES) picodrive.launcher

picodrive.launcher:
	$(CC) $(CPPFLAGS) -o $@ -DOSSO_GAMES_STARTUP="$(OSSO_GAMES_STARTUP)" -DGAME_CONF="$(GAME_CONF)" launcher.c

picodrive_plugin.so: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(POT): $(SRCS) $(EXTRA_GETTEXT_SRCS)
	$(XGETTEXT) $(XGETTEXT_OPTS) -o$@ $^

$(MO_FILES): %.mo: %.po
	msgfmt -c -o $@ $<

$(PO_FILES): %: $(POT)
	msgmerge -U $@ $(POT)
	@touch $@
	
clean: 
	rm -f picodrive_plugin.so *.o
	rm -f $(MO_FILES)
	rm -f $(DATA_FILES)

%: %.m4
	$(M4) $(M4DEFS) $^ > $@

install: all
	install -D picodrive.launcher $(DESTDIR)/usr/bin/picodrive.launcher
	install -D picodrive_plugin.so $(DESTDIR)$(GAME_PLUGIN_PATH)
	install -D -m 0644 picodrive.conf $(DESTDIR)$(GAME_CONF_PATH)
	install -D -m 0644 picodrive.desktop $(DESTDIR)/usr/share/applications/hildon/picodrive.desktop
	install -D -m 0644 picodrive.service $(DESTDIR)/usr/share/dbus-1/services/com.jcq.picodrive.service
	install -D -m 0644 picodrive.startup.service $(DESTDIR)/usr/share/dbus-1/services/com.jcq.picodrive.startup.service
	install -D -m 0644 icons/picodrive.png $(DESTDIR)/usr/share/icons/hicolor/scalable/hildon/picodrive.png
	install -D -m 0644 icons/banner.png $(DESTDIR)$(GAME_BANNER_PATH)
	install -D -m 0644 picodrive.schemas $(DESTDIR)/usr/share/gconf/schemas/picodrive.schemas
#ifneq ($(GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL),1)
#	gconftool-2 --makefile-install-rule $(DESTDIR)/usr/share/gconf/schemas/picodrive.schemas 
#endif
#	for lang in $(LANGS); do \
#		install -d $(DESTDIR)/usr/share/locale/$$lang/LC_MESSAGES ; \
#		install -m 0644 i18n/$$lang.mo \
#		 $(DESTDIR)/usr/share/locale/$$lang/LC_MESSAGES/$(GETTEXT_PACKAGE).mo ; \
#	done

.PHONY: all clean install

