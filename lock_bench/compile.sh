mkdir -p build
if [[ $2 == *.c ]]
then
	gcc -O2 -Wall -I./ --std=gnu11 -g -pthread $1 $2 -o test
else
	gcc -c -O2 -Wall -I./ --std=gnu11 -g $1 -o build/bench.o
	g++ -c -O2 -Wall -I./ --std=c++14 -g $2 -o build/lock.o
	g++ -pthread build/bench.o build/lock.o -o test
fi
