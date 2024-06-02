spec-pfault: core.c main.c
	$(CC) -O2 $^ -o $@

spec-fetch: core.c spec-fetch.c
	$(CC) -O2 $^ -o $@
