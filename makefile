CC = gcc
CFLAGS = -Wall -ansi -pedantic 
OBJS = main.o preprocessor.o first_pass.o second_pass.o utils.o tables.o command_info.o
TARGET = assembler

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c preprocessor.h first_pass.h second_pass.h
preprocessor.o: preprocessor.c preprocessor.h
first_pass.o: first_pass.c first_pass.h
second_pass.o: second_pass.c second_pass.h
utils.o: utils.c utils.h
tables.o: tables.c tables.h
command_info.o: command_info.c command_info.h

clean: 
	rm -f *.o $(TARGET)
