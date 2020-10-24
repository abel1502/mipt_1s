import argparse
from collections import deque
from functools import reduce
from enum import Enum


# Opcode structure:
# *** Warning: Apparently, C has this reversed... ***
# [--------] ([-][--][-][---][#] arg)
#              1  2   3  4    5
# 1) type: float/intergal
# 2) type: size (2**0, 2**1, 2**2, 2**3)
# 3) type: low/high
# 4) loc: mem, reg, imm (1 bit for each, 0b100 is invalid)
# 5) reserved
# arg) argument(s) (based on the previous values)


verbosity = 0


def log(*args, level=1, **kwargs):
    if level <= verbosity:
        print("[DBG]", *args, **kwargs)


class AddrMode(object):
    i = 0b0
    f = 0b1
    
    b  = 0b00
    w  = 0b01
    dw = 0b10
    qw = 0b11
    
    l = 0b0
    h = 0b1
    
    imm = 0b001
    reg = 0b010
    mem = 0b100
    
    types = {'df' : f << 3 | qw << 1 | l, 
             'fl' : f << 3 | dw << 1 | l, 
             'fh' : f << 3 | dw << 1 | h, 
             'qw' : i << 3 | qw << 1 | l, 
             'dwl': i << 3 | dw << 1 | l, 
             'dwh': i << 3 | dw << 1 | h,
             'wl' : i << 3 | w  << 1 | l,
             'wh' : i << 3 | w  << 1 | h, 
             'bl' : i << 3 | b  << 1 | l, 
             'bh' : i << 3 | b  << 1 | h}
    
    locs = {"": 0, "stack": 0, "imm" : imm, "reg": reg, "mem": mem}
    
    @staticmethod
    def parseLoc(name):
        isMem = False
        if name.startswith('[') and name.endswith(']'):
            isMem = True
            name = name[1:-1]
        
        names = name.split('+')
        assert 1 <= len(names) <= 2
        
        return reduce(lambda old, cur: old | cur, map(lambda x: AddrMode.locs[x.strip()], names), 0)
    
    @staticmethod
    def parseType(name):
        return AddrMode.types[name.strip()]


class Opcode(object):
    def __init__(self, num, name, types, locs):
        self.num = num
        self.name = name
        self.types = tuple(types)
        self.locs = tuple(locs)
    
    @staticmethod
    def parseArg(arg):
        if arg.strip() == '':
            return ([], [])
        
        types, locs = arg.split(':')
        
        types = list(map(AddrMode.parseType, types.strip().split(',')))
        locs  = list(map(AddrMode.parseLoc, locs.strip().split(',')))
        
        return (types, locs)
    
    @classmethod
    def parse(cls, line):
        num, line = line.split(":", 1)
        
        try:
            num = int(num, 16)
        except ValueError:
            num = int(num, 10)
        
        #assert num not in self.opcodes
        
        name, arg = line.split("(", 1);
        name = name.strip()
        arg = arg.strip()
        
        assert arg.endswith(")")
        arg = arg[:-1]
        
        #args = args.split(';')  # For the future
        #assert len(args) == 1
        #for i in range(len(args)):
        #    args[i] = self.parseArg(args[i])
        
        return cls(num, name, *cls.parseArg(arg))
    
    def genOpDef(self):
        # OP_DEF(
        #    0x01, /* num */
        #    PUSH, /* name cap */
        #    push, /* name low */
        #    1, /* argument count (currently 0 or 1, but for scalability we'll make it a number) */
        #    0x1234, /* argType bitmask */
        #    0x56, /* argLoc bitmask */
        #    {<code>})
        return "DEF_OP(0x{num:02x}, {nameCap:4}, {nameLow:4}, {argCnt}, 0b{argTypeMask:016b}, 0b{argLocMask:08b}, {{ {code} }})".format(
            num=self.num,
            nameCap=self.name.upper(),
            nameLow=self.name,
            argCnt=int(bool(self.types)),
            argTypeMask=reduce(lambda old, cur: old | (1 << (cur)), self.types, 0),
            argLocMask=reduce(lambda old, cur: old | (1 << (cur)), self.locs, 0),
            code=''
        )


class OpcodeDefParser(object):
    outTemplate = "{}"
    
    def __init__(self, ifileName=None):
        self.buf = deque()
        self.opcodes = []
        
        if ifileName is not None:
            self.read(ifileName)
    
    def read(self, ifileName):
        with open(ifileName, "r") as ifile:
            self.buf.extend(ifile.read().split('\n'))
    
    def write(self, ofileName):
        with open(ofileName, "w") as ofile:
            ofile.write(self.genCCode())
    
    def isEmpty(self):
        return len(self.buf) == 0
    
    def nextLine(self):
        assert not self.isEmpty()
        
        return self.buf.popleft()
    
    def parseAll(self):
        while not self.isEmpty():
            self.parseOpcodeDef()
    
    def parseOpcodeDef(self):
        line = self.nextLine().split("#", 1)[0].strip()
        
        if not line:
            return
        
        log(line, flush=True)
        
        self.opcodes.append(Opcode.parse(line))
    
    def genCCode(self):
        return self.outTemplate.format("\n".join(map(lambda x: x.genOpDef(), self.opcodes)))


if __name__ == "__main__":
    #parser = OpcodeDefParser()
    #print(parser.genCList("static const char *OPNAMES[256] = ", {"OP_PUSH": "\"push\"", "OP_POP": "\"pop\""}, compact=True))
    #exit()
    
    argParser = argparse.ArgumentParser()
    argParser.add_argument('-i', '--ifile', help='Input .def file name', default="opcodes.def")
    argParser.add_argument('-o', '--ofile', help='Output .h file name', default="opcode_defs.h.autogen")
    argParser.add_argument('-v', '--verbose', help='Increase verbosity', action='count', default=0)
    args = argParser.parse_args()
    
    verbosity = args.verbose
    
    parser = OpcodeDefParser(ifileName=args.ifile)
    
    parser.parseAll()
    
    parser.write(args.ofile)