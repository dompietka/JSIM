for ((i=0; i>=0 ;i++))
do

	./solv < ./malutkie_testy_przelewanka/in/prze$i.in > x
 
	if diff -b -w x ./malutkie_testy_przelewanka/out/prze$i.out > /dev/null
	then 
		echo -e "TEST$i      OK!   "
	else
		echo  "TEST$i       NIEOK!"
		more x
		echo -en "\n" 
		more ./malutkie_testy_przelewanka/in/prze$i.in
		echo -en "\n"
		more ./malutkie_testy_przelewanka/out/prze$i.out
		break
	fi
done