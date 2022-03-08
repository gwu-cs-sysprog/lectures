#!/bin/sh

gdb --args env LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH $1
