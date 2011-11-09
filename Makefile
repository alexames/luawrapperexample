test : *.cpp
	g++ -I/usr/include/lua5.1 -llua5.1 *.cpp -o example 
