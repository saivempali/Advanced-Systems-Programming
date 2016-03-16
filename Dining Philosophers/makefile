all: host philosopher

host: host.cpp
	g++ -std=c++0x -o host host.cpp -pthread

philosopher: philosopher.cpp
	g++ -std=c++0x -o philosopher philosopher.cpp -pthread
	@echo Compilation Successful!

clean: 
	rm host philosopher
	@echo All programs removed. Compile again!
