gcc -shared -o libslock.so slock.c
gcc -O2 inc_test.c -L. -pthread -lslock -o inc_test
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
time ./inc_test
