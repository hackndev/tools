# Directional pad command shell
# originally written by fahhem
# inspired by ezhikov's cmd.c

#Make all the methods from cmd.c into this file
#Then you'll have it in shell form?
#Hopefully?
#
. getkey.sh

syms=( 'a' 'b' 'c' 'd' 'e' 'f' 'g' 'h' 'i' 'j' 'k' 'l' 'm' 'n' 'o' 'p' 'q' 'r' 's' 't' 'u' 'v' 'w' 'x' 'y' 'z' '0' '1' '2' '3' '4' '5' '6' '7' '8' '9' 'A' 'B' 'C' 'D' 'E' 'F' 'G' 'H' 'I' 'J' 'K' 'L' 'M' 'N' 'O' 'P' 'Q' 'R' 'S' 'T' 'U' 'V' 'W' 'X' 'Y' 'Z' "'" "|" '>' '<' '&' '*' '\\' '/' ' ' '!' '.' ',' '{' '}' '[' ']' ';' ':' '(' ')' '#' '@' '+' '-' '?' '\"' '_' )
line=0
pos=0
table_l=${#syms[@]}
last_enter=0
LLEN=6
dest=""
ENTER='h'
BACKSPACE='f'

get_sel(){
	#ret=char[40+$LLEN]
	esc_l="\033[3;37;40m"
	esc_sl="\033[0;30;47m"
	esc_r="\033[0;0;0m"
	for ((i=0 ; i<$LLEN; i++))
	do
		sm[$i]=${syms[`expr $line \* $LLEN % $table_l + $i`]};
	done
	left=
	for ((a=0; a<pos; a++))
	do
		left=$left${sm[$a]}
	done
	tmp=${sm[$pos]}
	right=
	for ((a=`expr $pos + 1`; a<LLEN; a++))
	do
		right=$right${sm[$a]}
	done
	sm[$pos]=0
	_GET_SEL="$esc_l|$left$esc_sl$tmp$esc_l$right$esc_r"
}

drop(){
	get_sel
	echo -n -e $_GET_SEL
}

clean_drop(){
	tput dch `expr $LLEN + 1`
	tput cub `expr $LLEN + 1`
	drop
}

clean_show(){
	tput dch `expr $LLEN + 1`
	tput cub `expr $LLEN + 1`
	echo -n "${syms[`expr $line \* $LLEN + $pos`]}"
}

clean_drop

while :
do
	_get_key
	case $_KEY in
		"LEFT")
			if [ $pos -gt 0 ]
			then
				pos=`expr $pos - 1`
			else
				pos=`expr $LLEN - 1`
			fi
			last_enter=0
			clean_drop
			;;
		"RIGHT")
			pos=`expr \( $pos + 1 \) % $LLEN`
			last_enter=0
			clean_drop
			;;
		"UP")
			if [ "$line" -gt "0" ]
			then
				line=`expr $line - 1`
			else
				line=`expr $table_l / $LLEN - 1`
			fi
			last_enter=0
			clean_drop
			;;
		"DOWN")
			line=`expr \( $line + 1 \) % \( $table_l / $LLEN \)`
			last_enter=0
			clean_drop
			;;
		"$BACKSPACE")
			tput dch 1
			tput cub 1
			clean_drop
			;;
		"$ENTER")
			if [ "$last_enter" -eq "1" ]
			then
				$dest
				dest=""
				last_enter=0
				pos=0
			else
				last_enter=1
				clean_show
				dest=$dest${syms[`expr $line \* $LLEN + $pos`]}
				drop
			fi	
			;;
		"p")
			tput cub `expr $LLEN + 1`
			echo -n "       "
			echo -e -n "\t"
			clean_drop
			;;
	esac
done

