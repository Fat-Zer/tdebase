SUBDIRS = compat

xdg_menu_DATA = tde-settings.menu tde-information.menu tde-screensavers.menu

xdg_mergedmenu_DATA = tde-essential.menu
xdg_mergedmenudir = $(xdg_menudir)/applications-merged

xdg_directory_DATA = \
tde-main.directory \
tde-development.directory \
tde-editors.directory \
tde-edutainment.directory \
tde-games-arcade.directory \
tde-games-board.directory \
tde-games-card.directory \
tde-games-kids.directory \
tde-games-roguelikes.directory \
tde-games-strategy.directory \
tde-games.directory \
tde-graphics.directory \
tde-information.directory \
tde-internet-terminal.directory \
tde-internet.directory \
tde-more.directory \
tde-multimedia.directory \
tde-office.directory \
tde-science.directory \
tde-settings-accessibility.directory \
tde-settings-components.directory \
tde-settings-desktop.directory \
tde-settings-looknfeel.directory \
tde-settings-network.directory \
tde-settings-peripherals.directory \
tde-settings-hardware.directory \
tde-settings-power.directory \
tde-settings-security.directory \
tde-settings-sound.directory \
tde-settings-system.directory \
tde-settings-webbrowsing.directory \
tde-settings.directory \
tde-settingsmenu.directory \
tde-system-screensavers.directory \
tde-system-terminal.directory \
tde-system.directory \
tde-toys.directory \
tde-unknown.directory \
tde-utilities-xutils.directory \
tde-utilities.directory \
tde-utilities-accessibility.directory \
tde-utilities-desktop.directory \
tde-utilities-peripherals.directory \
tde-utilities-file.directory \
tde-utilities-pim.directory \
tde-development-translation.directory \
tde-development-webdevelopment.directory

install-data-local:
	for file in `cat $(srcdir)/old_desktop_files` ; do \
	    if test -f $(DESTDIR)$(xdg_appsdir)/$${file} ; then \
	        rm $(DESTDIR)$(xdg_appsdir)/$${file} ; \
		echo Deleting $(DESTDIR)$(xdg_appsdir)/$${file} ; \
	    fi; \
	done;

	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/.hidden
	$(INSTALL_DATA) $(srcdir)/hidden.directory $(DESTDIR)$(kde_appsdir)/.hidden/.directory

uninstall-local:
	-rm -f $(DESTDIR)$(kde_appsdir)/.hidden/.directory
