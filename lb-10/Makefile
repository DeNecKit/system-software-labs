.PHONY: run clean

SRC = ${wildcard *.c}
OBJ = ${patsubst %.c, %.o, ${SRC}}
OUT = main.exe

${OUT}: ${OBJ}
	gcc $^ -o $@

%.o: %.c
	gcc -c $^ -o $@ -Wall -Wextra -ggdb

run: ${OUT}
	./$^

clean:
	powershell rm ${OUT}
	powershell rm ${OBJ}
