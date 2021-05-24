

Flags = -std=c++11  -Wall -Wextra
output: Makefile tracerouter.o send.o receive.o
	g++ $(Flags) -o output tracerouter.o send.o receive.o 

tracerouter.o: Makefile tracerouter.cpp send.o receive.o
	g++ -c  $(Flags) tracerouter.cpp -o tracerouter.o 
receive.o: receive.cpp receive.h
	g++ -c $(Flags) receive.cpp	-o receive.o
send.o: Makefile send.cpp send.h
	g++ -c $(Flags) send.cpp -o send.o

clean:
	rm *.o output