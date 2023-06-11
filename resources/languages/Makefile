# Makefile created by MangaD

# TUTORIAL:
# https://www.labri.fr/perso/fleury/posts/programming/a-quick-gettext-tutorial.html

# Notes:
#  - If 'cmd /C' does not work, replace with 'cmd //C'

CMD=cmd /C

DIRS = pt de en zh zh_TW
LANG =

# https://stackoverflow.com/questions/4036191/sources-from-subdirectories-in-makefile
PO_FILES = $(wildcard **/*.po)
MO_FILES = $(PO_FILES:po=mo)

.PHONY: all clean init update potGen copy

all: $(MO_FILES)

potGen:
	xgettext --keyword=getText --language=C++ --add-comments \
		--package-name="hfworkshop" --package-version=1.0 \
		--copyright-holder="MangaD" --default-domain="hfworkshop" \
		--sort-output -o hfworkshop.pot ../../source/main.cpp

init: potGen
ifeq ($(LANG),)
	$(error Please set the language. Usage: make LANG="zh" init)
endif
ifeq ($(OS),Windows_NT)
	powershell -Command "(gc hfworkshop.pot) -replace 'CHARSET', 'UTF-8' | Out-File -encoding ASCII hfworkshop.pot"
	msginit --input=hfworkshop.pot --no-translator --locale=$(LANG).UTF-8 --output=$(LANG)\hfworkshop.po
else
# https://stackoverflow.com/questions/1083518/msgfmt-invalid-multibyte-sequence-error-on-a-polish-text
	sed --in-place hfworkshop.pot --expression='s/CHARSET/UTF-8/'
	msginit --input=hfworkshop.pot --no-translator --locale=$(LANG).UTF-8 --output=$(LANG)/hfworkshop.po
endif

update: potGen
ifeq ($(OS),Windows_NT)
	FOR %%D IN ($(DIRS)) DO msgmerge --update %%D\hfworkshop.po hfworkshop.pot
else
	for d in $(DIRS); do msgmerge --update $$d/hfworkshop.po hfworkshop.pot ; done
endif

%.mo: %.po
	msgfmt --output-file=$@ $<

copy:
ifeq ($(OS),Windows_NT)
	$(CMD) "FOR %%D IN ($(DIRS)) DO ( \
		mkdir ..\..\locale\%%D\LC_MESSAGES && \
		copy %%D\*.mo ..\..\locale\%%D\LC_MESSAGES \
	)"  2>nul
else
	mkdir -p ../../locale
	for d in $(DIRS) ; \
	do \
		mkdir -p ../../locale/$$d/LC_MESSAGES ; \
		cp $$d/*.mo ../../locale/$$d/LC_MESSAGES ; \
	done
endif

clean:
ifeq ($(OS),Windows_NT)
	$(CMD) "FOR %%D IN ($(DIRS)) DO del %%D\*.mo %%D\*~" 2>nul
else
	for d in $(DIRS); do rm -f $$d/*.mo $$d/*~ ; done
endif
