CPPFLAGS += -O2 -Wall -lm
CC=clang
list=passthrough signal_generator merge

all: $(list)

passthrough: passthrough.c
	$(CC) $(CPPFLAGS) -o $@ $<

signal_generator: signal_generator.c
	$(CC) $(CPPFLAGS) -o $@ $<

merge: merge.c
	$(CC) $(CPPFLAGS) -o $@ $<

clean:
	rm -f $(list)
