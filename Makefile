.PHONY: all clean

all: myshell
myshell: myshell.o
	g++ -g shell2_dvir.o -o myshell
myshell.o: shell2_dvir.cpp 
	g++ -c -g shell2_dvir.cpp
clean :
	rm -f *.o myshell