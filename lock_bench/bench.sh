source bench_conf.sh
shopt -s nullglob
for b_file in ./bench/*.c
do
	b_base=$(basename $b_file)
	b_name="${b_base%.*}"
	threads_vname="${b_name}_threads[*]"
	threads_arr=${!threads_vname}
	echo "bench/${b_base}"
	echo -n "n_threads; "
	for i in ${threads_arr[*]}; do echo -n "$i; "; done
	echo ""
	for st_dir in ./students/*/
	do
		source "${st_dir}bench_conf.sh"
		lock_vname=${b_name}_lock
		lock_file="${st_dir}${!lock_vname}"
		if [ "$lock_file" = "$st_dir" ]; then
			continue
		fi
		./compile.sh $b_file $lock_file
		echo -n "$(basename $st_dir); "
		for i in ${threads_arr[*]}; do ./measure.sh $i; done
		echo ""
	done
done
