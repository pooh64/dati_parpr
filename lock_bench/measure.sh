if out=$(/usr/bin/time -f%e ./test $1 2>&1 >/dev/null | tr -d '\n'); then
	echo -n "$out; "
else
	echo -n "failure;"
fi
