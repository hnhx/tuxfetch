PREFIX ?= /usr
LD_LIBS = -lprocps

install:
	@gcc tuxfetch.c -o $(DESTDIR)$(PREFIX)/bin/tuxfetch $(LD_LIBS)

uninstall:
	@rm $(DESTDIR)$(PREFIX)/bin/tuxfetch