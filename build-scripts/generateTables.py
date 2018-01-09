#!/usr/bin/python
import json

class Instruction:
    def __init__(self):
        self.name = 'unknown'
        self.firstOpcode = 0
        self.lastOpcode = 0

    def isRange(self):
        return self.firstOpcode != self.lastOpcode

def loadJsonFromFile(fileName):
    with open(fileName, "r") as f:
        return json.loads(f.read())

def loadInstructionSet(fileName):
    data = loadJsonFromFile(fileName)
    instructionSet = []
    for instructionName in data.keys():
        rawInstruction = data[instructionName]
        ins = Instruction()
        ins.name = instructionName
        instructionSet.append(ins)
        for param in rawInstruction.keys():
            if param == 'opcode':
                ins.firstOpcode = ins.lastOpcode = rawInstruction[param]
            elif param == 'range-first':
                ins.firstOpcode = rawInstruction[param]
            elif param == 'range-last':
                ins.lastOpcode = rawInstruction[param]
    return instructionSet

def capitalizeFirstLetter(string):
    return string[0].upper() + string[1:]

def generateDispatchTable(bytecodeSet, fileName):
    with open(fileName, "w") as out:
        for instruction in bytecodeSet:
            instructionName = instruction.name
            if instruction.isRange():
                for i in range(instruction.firstOpcode, instruction.lastOpcode + 1):
                    out.write("case %d + BYTECODE_TABLE_OFFSET: BYTECODE_DISPATCH_NAME_ARGS(%s, %d); break;\n" % (i, capitalizeFirstLetter(instructionName), i - instruction.firstOpcode));
            else:
                out.write("case %d + BYTECODE_TABLE_OFFSET: BYTECODE_DISPATCH_NAME(%s); break;\n" % (instruction.firstOpcode, capitalizeFirstLetter(instructionName)));


# SqueakV3PlusClosures bytecode set
squeakV3PlusClosures = loadInstructionSet("../definitions/SqueakV3PlusClosuresBytecodeSet.json")
squeakV3PlusClosures.sort(key=lambda instruction: instruction.firstOpcode)
generateDispatchTable(squeakV3PlusClosures, "../vm/SqueakV3PlusClosuresBytecodeSetDispatchTable.inc")

# SistaV1 bytecode set
sistaV1 = loadInstructionSet("../definitions/SistaV1BytecodeSet.json")
sistaV1.sort(key=lambda instruction: instruction.firstOpcode)
generateDispatchTable(sistaV1, "../vm/SistaV1BytecodeSetDispatchTable.inc")


