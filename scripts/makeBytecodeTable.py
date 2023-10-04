#!/usr/bin/env python3
import json
import sys

def buildBytecodeTableFromSpec(bytecodeSetSpec):
    bytecodes = [None]*256
    for bytecodeName in bytecodeSetSpec.keys():
        bytecodeSpec = bytecodeSetSpec[bytecodeName]
        bytecodeCapitalizedName = bytecodeName[0].upper() + bytecodeName[1:]
        if 'range-first' in bytecodeSpec:
            rangeFirst = bytecodeSpec['range-first']
            rangeLast = bytecodeSpec['range-last']
            for i in range(rangeFirst, rangeLast + 1):
                bytecodes[i] = (bytecodeCapitalizedName, i - rangeFirst)
        elif 'opcode' in bytecodeSpec:
            opcode = bytecodeSpec['opcode']
            bytecodes[opcode] = (bytecodeCapitalizedName, None)
        else:
            raise Error("Missing opcode or range spec for " + bytecodeName)

    return bytecodes

def main():
    if len(sys.argv) < 3:
        print("makeBytecodeTable.py <BytecodeSet.json> <BytecodeTable.inc>")
        return

    # Read the byte code table json.
    with open(sys.argv[1], 'r') as f:
        bytecodeSpecJson = json.loads(f.read())

    # Expand the bytecode table
    bytecodeTable = buildBytecodeTableFromSpec(bytecodeSpecJson)

    # Write the byte code table preprocessor spec.
    with open(sys.argv[2], 'w') as out:
        for i in range(0, len(bytecodeTable)):
            bytecodeSpec = bytecodeTable[i]
            if bytecodeSpec is None:
                    out.write("UNDEFINED_BYTECODE(%d)\n" % i)
            else:
                bytecodeName, bytecodeImplicitParam = bytecodeSpec
                if bytecodeImplicitParam is None:
                    out.write("BYTECODE(%d, %s)\n" % (i, bytecodeName))
                else:
                    out.write("BYTECODE_WITH_IMPLICIT_PARAM(%d, %s, %d)\n" % (i, bytecodeName, bytecodeImplicitParam))

if __name__ == "__main__":
    main()
