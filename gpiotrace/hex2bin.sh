#!/bin/sh

hex2dec() {
if [ "$1" ]; then
    if [ "${1:0:2}" = 0x ]; then
	echo $(($1))
    else
	echo $((0x$1))
    fi
fi
}

dec2bin() {
RES=$1
while [ $RES -ne 0 ]; do
    RES1=$(($RES / 2))
    OUT="$(($RES - $RES1 * 2))$OUT"
    RES=$RES1
done
echo $OUT
unset OUT RES RES1
}

hex2bin() {
dec2bin `hex2dec $1`
}

dec2fix() {
TMP=$(dec2bin $1)
for ((I=${#TMP}; I<32; I++)); do
    TMP="0$TMP"
done
echo $TMP
}



hex2fix() {
dec2fix `hex2dec $1`
}

bin2bits() {
#I=$((${#1} - 1))
echo $1 | for ((I=$((${#1} - 1)); I>=0  ; --I)); do
    read -n 1 bit
    echo "$I -> $bit"
#    I=$((--I))
done
}

fixbin2bits() {
limit=$((${#1} % 32))
if [ $limit != 0 ]; then
    for ((I=31; I==limit ; --I)); do
	echo "$I -> 0"
    done
fi
    
echo $1 | for ((I=$((${#1} - 1)); I>=0  ; --I)); do
    read -n 1 bit
    echo "$I -> $bit"
done
}

hex3bits() {
bin2bits `hex2fix $3``hex2fix $2``hex2fix $1`
}

hex2bits() {
bin2bits `hex2bin $1`
}

dec2bits() {
bin2bits `dec2bin $1`
}