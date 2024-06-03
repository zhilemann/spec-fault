CC := $(CC) $(CFLAGS)

spec-pfault-demo.exe: spec-pfault.c demo.c
	$(CC) $^ -o $@
