hrm-ccpu: main.o commands.o interpret.o
	g++ main.o commands.o interpret.o -o hrm-ccpu

main.o: main.cpp commands.hpp interpret.hpp
	g++ -std=c++11 -c main.cpp -o main.o

commands.o: commands.cpp commands.hpp commandsgen.ipp
	g++ -std=c++11 -c commands.cpp -o commands.o

interpret.o: interpret.cpp interpret.hpp commands.hpp
	g++ -std=c++11 -c interpret.cpp -o interpret.o

gperf:
	gperf instructions.gperf > commandsgen.ipp

clean:
	-rm main.o commands.o interpret.o hrm-ccpu
