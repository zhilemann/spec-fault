CC := $(CC) $(CFLAGS)

spec-pfault-demo: spec-pfault.c demo.c
	$(CC) $^ -o $@
