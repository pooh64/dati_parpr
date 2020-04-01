dir=$(dirname "$0")
./$dir/../compile.sh $dir/inc.c $1
for i in $(nproc) 4096
do
	echo -n "n_threads=$i: "
	if out=$(/usr/bin/time -f%e ./test $i 2>&1 >/dev/null | tr -d '\n'); then
		echo "$out"
	else
		echo -n "failure"
	fi
done
