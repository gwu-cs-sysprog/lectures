#!/bin/sh

gdb --args env LD_PRELOAD=./libmalloclog.so ./main.bin
