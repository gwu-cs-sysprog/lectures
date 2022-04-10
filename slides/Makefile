PRESENTATIONS = $(wildcard *slides.md)
REVEALPREFIX  = reveal.js-master
REVEALDIR     = reveal

all: $(REVEALDIR) $(patsubst %.md,%.html,$(PRESENTATIONS))

%.html: %.md
	pandoc --from markdown+grid_tables --template=template.html -t revealjs -o $@ $<

$(REVEALDIR):
	unzip $(REVEALPREFIX).zip
	mv $(REVEALPREFIX) $(REVEALDIR)

clean:
	rm -rf $(REVEALDIR) $(patsubst %.md,%.html,$(PRESENTATIONS))

# .PHONY: $(REVEALDIR)
