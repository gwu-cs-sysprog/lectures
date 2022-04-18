INLINE_EXEC = inline_exec
CC = gcc
# we're permissive on warnings here to enable terser example code (i.e. avoids fixing warnings)
WARNING_EXCEPTIONS = -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable
CFLAGS = -ggdb3 -rdynamic -Wall -Wextra $(WARNING_EXCEPTIONS)
CLIBS  = -ldl
CFILES = $(wildcard ??/*.c)
CBIN   = $(patsubst %.c,%.bin,$(CFILES))
NESTED = 08/libexample 09/malloclog
AGG = aggregate.md
TITLE = title.md

# Note: html templates replicated from https://github.com/ryangrose/easy-pandoc-templates
TEMPLATEDIR = templates
PANDOC = pandoc
PANDOC_BOOK = --toc --number-sections --listings --template=$(TEMPLATEDIR)/eisvogel.tex
PANDOC_WEBPAGE = --toc --number-sections --listings --template=$(TEMPLATEDIR)/elegant_bootstrap_menu.html

INSTALLDIR = ../gwu-cs-sysprog.github.io/

OUTPUT_PREFIX=lectures

all: doc
	@:

nested:
	@$(foreach N,$(NESTED),make --no-print-directory -C $(N);)

$(INLINE_EXEC): $(INLINE_EXEC)_tmp nested
	@./$<

$(INLINE_EXEC)_tmp: $(INLINE_EXEC)_tmp.c
	@$(CC) $(CFLAGS) $^ -o $@ $(CLIBS)

$(INLINE_EXEC)_tmp.c:
	@echo "" >> $@

examples: $(CBIN)

%.bin: %.c
	@$(CC) $(CFLAGS) $^ -o $@ $(CLIBS)

$(INLINE_EXEC)_clean:
	@rm -f $(INLINE_EXEC)_tmp.c $(INLINE_EXEC)_tmp 2> /dev/null

$(AGG): $(TITLE) $(sort $(wildcard ??/??_*.md))
	cat $^ | awk -f tools/exec_inline_code.awk > $@

build_code: examples $(AGG)

html: build_code
	$(PANDOC) $(AGG) $(PANDOC_WEBPAGE) -o $(OUTPUT_PREFIX).html

pdf: build_code
	$(PANDOC) $(AGG) $(PANDOC_BOOK) -o $(OUTPUT_PREFIX).pdf

install: html
	make -C slides
	cp lectures.html $(INSTALLDIR)/index.html
	cp figures/* $(INSTALLDIR)/figures/
	cp -r slides/*_slides.html slides/reveal slides/figures $(INSTALLDIR)/slides/

doc: html pdf

clean: $(INLINE_EXEC)_clean
	rm -f $(AGG) $(OUTPUT_PREFIX).pdf $(OUTPUT_PREFIX).html $(CBIN)

.PHONY: all clean html pdf doc $(INLINE_EXEC)_clean $(INLINE_EXEC) examples build_code nested
