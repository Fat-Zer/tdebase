## Makefile.am for tdesud

INCLUDES = $(all_includes)

KDE_CXXFLAGS = $(KDE_USE_FPIE)

bin_PROGRAMS = tdesud
tdesud_SOURCES = tdesud.cpp repo.cpp lexer.cpp handler.cpp secure.cpp 
tdesud_LDFLAGS = $(KDE_USE_PIE) $(all_libraries) $(KDE_RPATH)
tdesud_LDADD = $(LIB_TDECORE) -ltdesu $(LIBSOCKET)
noinst_HEADERS = repo.h handler.h lexer.h secure.h

## tdesud needs to be suid or sgid something
install-data-local:
	@echo "********************************************************"
	@echo ""
	@echo "For security reasons, tdesud is installed setgid nogroup."
	@echo "Kdesud is the daemon that implements the password caching."
	@echo ""
	@echo "You should NOT use the password caching feature if tdesud is"
	@echo "not installed setgid nogroup."
	@echo ""
	@echo "********************************************************"

install-exec-hook: 
	@(chown root:@nogroup@ $(DESTDIR)$(bindir)/tdesud && chmod 2755  $(DESTDIR)$(bindir)/tdesud) \
	|| echo "Error: Could not install tdesud as setgid nogroup!!\n" \
	"The password caching feature is disabled."

messages:
	$(XGETTEXT) $(tdesud_SOURCES) -o $(podir)/tdesud.pot
