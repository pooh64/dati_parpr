echo -n $1 >>data.csv
for i in 1 2 4 8 16 32 64 # 128 256 512 1024 2048 4096 8192
do
	echo $i
	echo -n ", " >>data.csv
	/usr/bin/time -f%e ./test $i 2>&1 >/dev/null | tr -d '\n' >>data.csv
done
printf "\n" >>data.csv
