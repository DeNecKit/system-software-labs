.PHONY: run clean

OUT = main

${OUT}: ${wildcard *.c}
	gcc $^ -o $@ -Wall -Wextra -ggdb -pthread

run: ${OUT}
	./$^

clean:
	rm ${OUT}
	rm /dev/shm/sem.sem* -f
