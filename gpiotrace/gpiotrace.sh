#!/bin/sh

REG1=0
REG2=0
REG3=0
REG4=0

. hex2bin.sh
# no ignore
# ignore_gpios=cat
ignore_gpios="grep -vE ^(00|15|16|24|25|28|29|30|31|32|38|43|44|46|47|58|59|60|61|62|63|64|65|66|67|68|69|70|71|72|73|74|75|76|77|78|89|95|113|114|116)"

# interesting GPIOs: 9+47

if [ $# = 0 ]; then
	echo << EOB
$0
Usage:
	$0 [register_group] [input_file]
EOB
	exit
fi
FILE=${2:-GPIO.txt}

sed "s/.*\($1[^|]*\)\(||\)*.*/\1/g" $FILE | while read dummy timestamp new1 new2 new3 dummy2; do
#	echo "$dummy $timestamp $new1 $new2 $new3 $new4 $dummy2"
	dif1=$((REG1 ^ new1))
	dif2=$((REG2 ^ new2))
	dif3=$((REG3 ^ new3))
	dif4=$((REG4 ^ new4))
	echo "---------------- $timestamp ----------------"
	bin2bits `dec2fix $((REG4 & dif4))``dec2fix $((REG3 & dif3))``dec2fix $((REG2 & dif2))``dec2fix $((REG1 & dif1))` | grep -v -- '-> 0' | sed 's/-> 1/-> 0/' | $ignore_gpios
	bin2bits `dec2fix $((new4 & dif4))``dec2fix $((new3 & dif3))``dec2fix $((new2 & dif2))``dec2fix $((new1 & dif1))` | grep -v -- '-> 0' | $ignore_gpios
	REG1=$new1
	REG2=$new2
	REG3=$new3
	REG4=$new4
done
