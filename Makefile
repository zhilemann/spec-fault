CC := $(CC) $(CFLAGS)

demo.exe: src/spec-fault.c demo.c
	$(CC) $^ -o $@

test.exe: src/spec-fault.c test.c
	$(CC) $^ -o $@
