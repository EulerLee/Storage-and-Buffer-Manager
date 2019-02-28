object = buffer.o storage.o
project : $(object) main.cc init.cc
	g++ -o project main.cc $(object)
	g++ -o init init.cc $(object)
buffer.o : buffer.cc buffer.h environment.h storage.h
	g++ -c buffer.cc
storage.o : storage.cc storage.h environment.h buffer.h
	g++ -c storage.cc
clean :
	rm project $(object) init project
