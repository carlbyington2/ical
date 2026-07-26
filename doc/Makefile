# This Makefile uses programs available locally.
# So don't complain if it does not work for you.

MAN  = ical
IDOC = interface
RDOCS = $(MAN).man $(MAN).doc $(IDOC).doc

all: $(RDOCS) scrub

release: $(RDOCS) scrub

$(MAN).man: $(MAN).html
	html2man $(MAN).html > $@

$(MAN).doc: $(MAN).html
	html2tk $(MAN).html > $@

$(IDOC).doc: $(IDOC).html
	html2tk $(IDOC).html > $@

clean: scrub

scrub:
