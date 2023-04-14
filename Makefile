.PHONY: all clean

all: myshell
myshell: myshell.o
	g++ -g myshell.o -o myshell
myshell.o: myshell.cpp
	g++ -c -g myshell.cpp
clean :
	rm -f *.o myshell