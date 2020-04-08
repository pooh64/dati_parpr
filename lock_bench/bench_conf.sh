inc_threads=(1 2 4 8 16 32 64 128 256 512 1024)
io_threads=(1 2 4 8 16 32 64 128 256 512 1024)
ring_threads=()
npr=$(nproc)
for((i=1;i<=npr;i++)) do ring_threads+=($i); done;
