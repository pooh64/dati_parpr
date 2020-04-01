dir=$(dirname "$0")
./$dir/../compile.sh $dir/ring.c $1
if out=$(/usr/bin/time -f%e ./test 2>&1 >/dev/null | tr -d '\n'); then
	echo "$out"
else
	echo -n "failure"
fi
