gcc -O2 inc_test.c -L. -pthread -llock -o inc_test
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}${LD_LIBRARY_PATH:+:}.
time ./inc_test $1
