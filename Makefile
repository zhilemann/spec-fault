CC := $(CC) $(CFLAGS)

demo.exe: lib/spec-fault.c demo.c
	$(CC) $^ -o $@

test.exe: lib/spec-fault.c test/*.c
	$(CC) $^ -o $@
