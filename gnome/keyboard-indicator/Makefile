# SHELL_SRC and SHELL_BUILD name matching GNOME Shell 51 source/build trees.
LOCALBASE?= /usr/local
MUTTERDIR= ${LOCALBASE}/lib/mutter-51
SHELL_BUILD?= ${SHELL_SRC}/build
EXTDIR?= ${HOME}/.local/share/gnome-shell/extensions/kbdlight@omnibook
CFLAGS+= -fPIC -Wall -I${SHELL_SRC}/src -I${SHELL_SRC}/src/st \
         -I${SHELL_BUILD}/src -I${SHELL_BUILD}/src/st
GTK_CFLAGS!= pkg-config --cflags libmutter-51 gtk4 atk
GTK_LIBS!= pkg-config --libs mutter-clutter-51 gio-2.0

all: libomnibook-keyboard.so OmniBookKeyboard-1.0.typelib

libomnibook-keyboard.so: indicator.c
	${CC} ${CFLAGS} ${GTK_CFLAGS} -shared -o $@ $> ${GTK_LIBS} \
	    -L${LOCALBASE}/lib/gnome-shell -Wl,-rpath,${LOCALBASE}/lib/gnome-shell -Wl,-rpath,${MUTTERDIR} -lst-51

OmniBookKeyboard-1.0.typelib: OmniBookKeyboard-1.0.gir
	g-ir-compiler --includedir=${MUTTERDIR} -o $@ $>

install: all
	install -d ${DESTDIR}${EXTDIR}
	install -m 0644 extension.js metadata.json libomnibook-keyboard.so \
	    OmniBookKeyboard-1.0.typelib ${DESTDIR}${EXTDIR}/
