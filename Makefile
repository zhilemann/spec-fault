CC := $(CC) $(CFLAGS)

spec-pfault-demo.exe: src/spec-pfault.c demo.c
	$(CC) $^ -o $@

spec-pfault-test.exe: src/spec-pfault.c test.c
	$(CC) $^ -o $@
