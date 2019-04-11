#!/bin/bash

cd $( dirname "${BASH_SOURCE[0]}")
./makeBytecodeTable.py ../definitions/SqueakV3PlusClosuresBytecodeSet.json ../vm/SqueakV3PlusClosuresBytecodeSetTable.inc || exit 1
./makeBytecodeTable.py ../definitions/SistaV1BytecodeSet.json ../vm/SistaV1BytecodeSetTable.inc || exit 1
