
all: compile run

compile:
	javac ./lab5/src/szybkoscJavy/*.java
	g++ -std=c++17 -Wall -o3 ./gen.cpp -o gen.e
	g++ -std=c++17 -Wall -o3 ./triangles.cpp -o triangles.e

 
run:
	./gen.e > input.txt
	./triangles.e < input.txt
	java -cp ./lab5/src szybkoscJavy.Main <input.txt


