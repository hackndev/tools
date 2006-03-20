#! /bin/bash
#Char codes
enter='\n'
ESC=$'\e'
CSI='\['



_keys()
{
	_KEYS=
	__KX=
	ESC_END=[a-zA-NP-Z~^\$@$ESC]
	
	while :
	do
	  IFS= read -r -s -n1 -d '' -t1 __KX
	  _KEYS=$_KEYS$__KX
	  case $__KX in
		"" | $ESC_END ) break ;;
	  esac
	done
}

_key()
{
	IFS= read -r -s -n1 -d '' "${@:-_KEY}"
}

_esc2key()
{
  case $1 in
    ## Cursor keys
    "$CSI"A | ${CSI}OA | "${ESC}[A" ) _ESC2KEY=UP ;;
    "$CSI"B | ${CSI}0B | "${ESC}[B" ) _ESC2KEY=DOWN ;;
    "$CSI"C | ${CSI}OC | "${ESC}[C" ) _ESC2KEY=RIGHT ;;
    "$CSI"D | ${CSI}OD | "${ESC}[D" ) _ESC2KEY=LEFT ;;

    ## Function keys (unshifted)
    "$CSI"11~ | "$CSI["A | ${ESC}OP ) _ESC2KEY=F1 ;;
    "$CSI"12~ | "$CSI["B | ${ESC}OQ ) _ESC2KEY=F2 ;;
    "$CSI"13~ | "$CSI["C | ${ESC}OR ) _ESC2KEY=F3 ;;
    "$CSI"14~ | "$CSI["D | ${ESC}OS ) _ESC2KEY=F4 ;;
    "$CSI"15~ | "$CSI["E ) _ESC2KEY=F5 ;;
    "$CSI"17~ | "$CSI["F ) _ESC2KEY=F6 ;;
    "$CSI"18~ ) _ESC2KEY=F7 ;;
    "$CSI"19~ ) _ESC2KEY=F8 ;;
    "$CSI"20~ ) _ESC2KEY=F9 ;;
    "$CSI"21~ ) _ESC2KEY=F10 ;;
    "$CSI"23~ ) _ESC2KEY=F11 ;;
    "$CSI"24~ ) _ESC2KEY=F12 ;;

    ## Insert, Delete, Home, End, Page Up, Page Down
    "$CSI"2~ ) _ESC2KEY=INS ;;
    "$CSI"3~ ) _ESC2KEY=DEL ;;
    "$CSI"[17]~ | "$CSI"H ) _ESC2KEY=HOME ;;
    "$CSI"[28]~ | "$CSI"F ) _ESC2KEY=END ;;
    "$CSI"5~ ) _ESC2KEY=PGUP ;;
    "$CSI"6~ ) _ESC2KEY=PGDN ;;

    ## Everything else; add other keys before this line
    *) _ESC2KEY=UNKNOWN ;;
  esac
  [ -n "$2" ] && eval "$2=\$_ESC2KEY"
}

_get_key()
{
	_key
	case $_KEY in
		"$ESC") _keys
			_esc2key "$ESC$_KEYS" _KEY
			;;
	esac
}

