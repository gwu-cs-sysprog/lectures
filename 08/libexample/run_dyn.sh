#!/bin/sh

# set the LD_LIBRARY_PATH environmental variable which `ld` uses to find the libraries!
LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH $1
