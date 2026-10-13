CC      = gcc
CFLAGS  = -std=c17 -Wall -Wextra -Wpedantic -O2
SRCS    = $(wildcard src/*.c)
OBJS    = $(SRCS:.c=.o)
HEADERS = $(wildcard src/*.h)

calc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# ogni .o dipende dal suo .c e dagli header (semplificato: tutti gli .h)
src/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# versione con AddressSanitizer per cercare leak e accessi sbagliati
debug: CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined
debug: clean calc

# esegue gli esempi e confronta stdout, stderr e codice d'uscita con quelli attesi in tests/
test: calc
	@./calc esempi/esempio.calc > tests/.out 2> tests/.err; echo $$? > tests/.rc
	@diff -u tests/esempio.out tests/.out && diff -u /dev/null tests/.err && test "$$(cat tests/.rc)" = 0 \
		&& echo "ok    esempio.calc" || { echo "ERRORE esempio.calc"; exit 1; }
	@./calc esempi/errori.calc > tests/.out 2> tests/.err; echo $$? > tests/.rc
	@diff -u tests/errori.out tests/.out && diff -u tests/errori.err tests/.err && test "$$(cat tests/.rc)" = 1 \
		&& echo "ok    errori.calc" || { echo "ERRORE errori.calc"; exit 1; }
	@rm -f tests/.out tests/.err tests/.rc

clean:
	rm -f calc $(OBJS) tests/.out tests/.err tests/.rc

.PHONY: debug test clean
