CC := clang
CFLAGS := -g -Wall -Werror -Wno-unused-function -Wno-unused-variable

all: p2pchat pchat

clean:
	rm -rf p2pchat p2pchat.dSYM pchat pchat.dSYM

pchat: pchat.c ui.c ui.h list.c list.h
	$(CC) $(CFLAGS) -o pchat pchat.c ui.c list.c -lform -lncurses -lpthread


p2pchat: p2pchat.c ui.c ui.h
	$(CC) $(CFLAGS) -o p2pchat p2pchat.c ui.c -lform -lncurses -lpthread
