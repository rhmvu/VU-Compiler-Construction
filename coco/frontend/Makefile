CFLAGS :=
LDFLAGS :=

.PHONY: all check clean
.SUFFIXES:

all:

%: %.o
	clang -o $@ $< $(LDFLAGS)

%.o: %.ll
	llc -filetype=obj -o $@ $<

%.ll: %.fc
	frontend $(CFLAGS) -o $@ $<

check:
	@python3 runtests.py

clean:
	rm -rf parsetab.py parser.out __pycache__ test/tmp*
