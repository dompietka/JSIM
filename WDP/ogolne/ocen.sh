for ((i=0; i>=0 ;i++))
do

    ##brut
    echo i > seed
    ./generatorka < seed > wejscie

    ./solv < wejscie > wzo
    ./brut < wejscie > bru

    if diff -b -w wzo bru > dev/null

    ##plik
    ./solv < ./folder/in/nazwa$i.in > x

	if diff -b -w x ./folder/out/nazwa$i.out > /dev/null
	then 
		echo -e "TEST$i      OK!   "
	else
		echo  "TEST$i       NIEOK!"
		more x
		echo -en "\n"
		more ./folder/out/nazwa$i.out
        echo -en "\n" 
		more ./folder/in/nazwa$i.in
		
		break
	fi
done

##dodac kolorki i kompilowanie 