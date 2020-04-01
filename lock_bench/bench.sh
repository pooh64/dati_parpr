shopt -s nullglob
for b_script in ./bench/*.sh
do
	b_base=$(basename $b_script)
	b_name="${b_base%.*}"
	b_lock="${1}/${b_name}_lock.c"
	echo "bench/$b_name"
	./$b_script $b_lock
done
