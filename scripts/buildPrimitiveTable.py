#!/usr/bin/python
import sys

primitiveNumberNameDictionary = {}
with open('../include/crank-vm/system-primitive-number.h', 'r') as f:
    for line in f:
        line = line.strip()
        if line.startswith('CRANK_VM_SYSTEM_PRIMITIVE_NUMBER_') and ('=' in line):
            name = line[:line.find('=')].strip()
            number = line[line.find('=')+1:line.find(',')].strip()
            if number.isdigit():
                primitiveNumberNameDictionary[name] = int(number)

allPrimitives = []
for line in sys.stdin:
    elements = line.strip().split()
    name = elements[0]
    firstRange = int(primitiveNumberNameDictionary.get(elements[1].strip(), elements[1]))
    lastRange = firstRange
    if len(elements) > 2:
        lastRange = int(elements[2])
    allPrimitives.append((name, firstRange, lastRange))

usedNumbers = map(lambda x: x[1], allPrimitives) + map(lambda x: x[2], allPrimitives)
maxNumber = max(usedNumbers)

primitiveTable = ['NULL'] * (maxNumber + 1)
for primitive in allPrimitives:
    name, firstRange, lastRange = primitive
    for i in range(firstRange, lastRange + 1):
        primitiveTable[i] = name

sys.stdout.write("""/// Automatically generated code by generatePrimitiveTable.sh
#include "numbered-primitives.h"

// The number of primitives in the numbered primitive table
const size_t crankvm_numberedPrimitiveTableSize = %d;

// The numbered primitive table
const crankvm_primitive_function_t crankvm_numberedPrimitiveTable[%d] = {
""" % (len(primitiveTable), len(primitiveTable)))

for primitive in primitiveTable:
    print '    %s,' % (primitive)

sys.stdout.write("""};
""")

