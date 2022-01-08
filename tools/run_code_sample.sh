#!/bin/sh

# Assumes that the Makefile to compile and run the code is in the current directory using the default recipe

rm -f $1
make --no-print-directory inline_exec 2>> $1 >> $1
# remove some strange characters output by gcc
sed -i -e 's/‘//g' -e 's/’//g' $1
