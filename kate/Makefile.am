SUBDIRS = utils interfaces app data pics
EXTRA_DIST = AUTHORS COPYING.LIB ChangeLog DESIGN NEWS README TODO

messages:
	rm -f tips.txt
	(cd data && $(PREPARETIPS) > ../tips.txt)
	$(EXTRACTRC) ./*/*.rc >> ./rc.cpp
	$(XGETTEXT) `find . -name "*.cpp" -o -name "*.txt"` -o $(podir)/kate.pot
	rm -f tips.txt

include ../admin/Doxyfile.am

