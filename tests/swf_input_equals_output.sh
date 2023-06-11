#!/bin/bash

printf " ------------------------------------------------------------\n"
printf " \tHFW test if SWF output is equal to SWF input.\n"
printf " \tThe SWF input should not be compressed.\n"
printf " ------------------------------------------------------------\n"

RED='\033[0;31m'   	# RED
NC='\033[0m'       	# No Color
GREEN='\033[0;32m' 	# GREEN
YELLOW='\33[1;33m' 	# YELLOW
BLINK1='\e[5m'
BLINK2='\e[25m'	   	#BLINK
INV='\e[7m'         #INVERTED
LGREEN='\e[102m'
BOLD='\e[1m'

out=HF_out.swf

for f in *.swf;
do
	echo "Processing $f...";
	echo -e "$f\n6\n0\n\n0\n" | ./HFWorkshop >/dev/null 2>&1

	if cmp -s -- "$f" "$out"; then
		echo -e "${BOLD}${GREEN}${INV}OK!${NC}"
	else
		echo -e "${BOLD}${RED}${BLINK1}${INV}ERROR${NC}"
	fi
done







