javac ./lab5/src/szybkoscJavy/*.java
g++ -std=c++17 ./gen.cpp -o gen.e
g++ -std=c++17 ./triangles.cpp -o triangles.e

echo "dla n == 100"
echo "100" | ./gen.e > input.txt
echo -e "c++ "
./triangles.e < input.txt
echo -e "\n\njava"
java -Xcomp -Xdiag -cp ./lab5/src szybkoscJavy.Main <input.txt

echo -e "\n\n dla n == 250"
echo "250" | ./gen.e > input.txt
echo -e "c++ "
./triangles.e < input.txt
echo -e "\njava"
java -Xcomp -Xdiag -cp ./lab5/src szybkoscJavy.Main <input.txt

echo -e "\n\n dla n == 500"
echo "500" | ./gen.e > input.txt
echo -e "c++ "
./triangles.e < input.txt
echo -e "\njava"
java -Xcomp -Xdiag -cp ./lab5/src szybkoscJavy.Main <input.txt