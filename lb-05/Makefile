.PHONY: run clean

OUT = main

${OUT}: ${wildcard *.c}
	gcc $^ -o $@ -Wall -Wextra -ggdb -lpthread

run: ${OUT}
	./$^

clean:
	rm ${OUT}
