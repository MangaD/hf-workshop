#!/bin/bash

# Place this file in a folder with the HF SWF to test the data files export
# and replace. Every data file will be replaced with the exported version of itself.
# Then the SWF will be exported. If the exported SWF differs from the original,
# then the export and/or replace function is bugged.

printf " ------------------------------------------------------------\n"
printf " \tHFW test if export and replace data is working properly.\n"
printf " \tThe SWF input should not be compressed.\n"
printf " ------------------------------------------------------------\n"

RED='\033[0;31m'   	# RED
NC='\033[0m'       	# No Color
GREEN='\033[0;32m' 	# GREEN
YELLOW='\033[1;33m' 	# YELLOW
BLINK1='\e[5m'
BLINK2='\e[25m'	   	#BLINK
INV='\e[7m'         #INVERTED
LGREEN='\e[102m'
BOLD='\e[1m'

# Get arguments with flags in bash
# https://stackoverflow.com/a/57511532/3049315
while getopts "f:" arg; do
	case $arg in
		f) in=$OPTARG;;
	esac
done

out=HF_out.swf

testFiles() {

	echo "Processing $in...";
	echo -e "$in\n5\n2\nall\n0\n0\n" | ./HFWorkshop >/dev/null 2>&1

	# Working with bash arrays: 
	# - https://linuxconfig.org/how-to-use-arrays-in-bash-script
	# - https://www.shell-tips.com/bash/arrays/
	declare -A data_files

	for f in *.zip;
	do
		# Extract ID from file name
		# https://stackoverflow.com/a/36798723/3049315
		id=$(echo "$f" | sed -n "s/^\([0-9]\+\).*$/\1/p")

		data_files+=([$id]=$f)
	done

	for key in "${!data_files[@]}";
	do
		input_str="$in\n5\n"

		input_str+="3\n$key\n"
		value=${data_files[$key]}
		input_str+="$value\n" # value

		input_str+="0\n\n6\n0\n\n0\n"

		echo -e "$input_str" | ./HFWorkshop >/dev/null 2>&1

		echo "Processing $value...";

		if cmp -s -- "$in" "$out"; then
			echo -e "${BOLD}${GREEN}${INV}OK!${NC}"
		else
			echo -e "${BOLD}${RED}${BLINK1}${INV}FAIL${NC}"
		fi

	done

	rm *.zip
}

# TODO: -r flag for all SWF files

# Check if variable is set
# https://stackoverflow.com/a/13864829/3049315
if [ -z "$in" ]; then
	echo "You must provide the name of the SWF file with the -f flag."
	exit
fi

echo -e "${BOLD}${YELLOW}WARNING: THIS SCRIPT WILL DELETE ALL ZIP FILES IN THE CURRENT FOLDER! ARE YOU SURE YOU WANT TO CONTINUE?${NC} [y/N]"

# https://stackoverflow.com/a/29436276/3049315
read answer
if [[ $answer == y || $answer == Y ]]; then
	testFiles
fi