INLINE_EXEC = inline_exec
CC = gcc
# we're permissive on warnings here to enable terser example code (i.e. avoids fixing warnings)
WARNING_EXCEPTIONS = -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable
CFLAGS = -O0 -g -Wall -Wextra $(WARNING_EXCEPTIONS)
CFILES = $(wildcard ??/*.c)
CBIN   = $(patsubst %.c,%.bin,$(CFILES))
AGG = aggregate.md
TITLE = title.md

# Note: html templates replicated from https://github.com/ryangrose/easy-pandoc-templates
TEMPLATEDIR = templates
PANDOC = pandoc
PANDOC_BOOK = --toc --number-sections --listings --template=$(TEMPLATEDIR)/eisvogel.tex
PANDOC_WEBPAGE = --toc --number-sections --listings --template=$(TEMPLATEDIR)/elegant_bootstrap_menu.html

OUTPUT_PREFIX=lectures

all: doc

$(INLINE_EXEC): $(INLINE_EXEC)_tmp
	@./$<

$(INLINE_EXEC)_tmp: $(INLINE_EXEC)_tmp.c
	@$(CC) $(CFLAGS) $^ -o $@

$(INLINE_EXEC)_tmp.c:
	@echo "" >> $@

examples: $(CBIN)

%.bin: %.c
	@$(CC) $(CFLAGS) $^ -o $@

$(INLINE_EXEC)_clean:
	@rm -f $(INLINE_EXEC)_tmp.c $(INLINE_EXEC)_tmp 2> /dev/null

$(AGG): $(TITLE) $(sort $(wildcard ??/*.md))
	cat $^ | awk -f tools/exec_inline_code.awk > $@

build_code: examples $(AGG)

html: build_code
	$(PANDOC) $(AGG) $(PANDOC_WEBPAGE) -o $(OUTPUT_PREFIX).html

pdf: build_code
	$(PANDOC) $(AGG) $(PANDOC_BOOK) -o $(OUTPUT_PREFIX).pdf

doc: html pdf

clean: $(INLINE_EXEC)_clean
	rm -f $(AGG) $(OUTPUT_PREFIX).pdf $(OUTPUT_PREFIX).html $(CBIN)

.PHONY: all clean html pdf doc $(INLINE_EXEC)_clean $(INLINE_EXEC) examples build_code
