findNotScoped() {
	grep -rn "[^A-Za-z0-9:/\"]$1[^A-Za-z0-9\"]" . | grep -vi '^[^:]*/'"$1"'[.][pyhcp.]*:' | grep -v '^[^:]*:[0-9]*:namespace MGE {' | grep "$1"
}

findAllNotScoped() {
	exec 3<"$1"
	while read -u 3 c x; do
		if [ "$c" != "#" ]; then
			findNotScoped "$c"
			echo "---"
			read x
		fi
	done
	exec 3<&-
}
