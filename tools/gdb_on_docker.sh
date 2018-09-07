#!/bin/bash
docker run --rm -i --privileged -v $PWD:$PWD -w $PWD bubackos gdb $*
