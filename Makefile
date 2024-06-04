CC := $(CC) $(CFLAGS)

demo.exe: src/spec-pfault.c demo.c
	$(CC) $^ -o $@

test.exe: src/spec-pfault.c test.c
	$(CC) $^ -o $@
