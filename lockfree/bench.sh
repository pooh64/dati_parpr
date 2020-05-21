for ((i = 1; i < 40; ++i))
do
	if out=$(/usr/bin/time -f%e ./a.out $i 2>&1 >/dev/null | tr -d '\n'); then
		echo -n "$out; "
	else
		echo -n "failure;"
	fi
done
