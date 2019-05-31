#!/bin/bash
cd $( dirname "${BASH_SOURCE[0]}")
cat ../vm/* | grep "^CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER" | sed 's/CRANK_VM_CONNECT_PRIMITIVE_TO_NUMBER//g; s/[(,]//g; s/).*//g' | ./buildPrimitiveTable.py > ../vm/numbered-primitives.c
