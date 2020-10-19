
helloworld: helloworld.s
	gcc -c helloworld.s && ld -o helloworld_asm helloworld.o

debugger: main.cpp debugger.cpp
	g++ -Wall -pedantic --std=c++0x main.cpp debugger.cpp -o debugger

breakworld: breakworld.s
	gcc -c breakworld.s && ld -o breakworld_asm breakworld.o

