import argparse
from collections import deque


CCodeTpl = """
#ifndef OPCODE_H_GUARD
#define OPCODE_H_GUARD

#include <stdint.h>

//typedef uint8_t opcode_t;
typedef enum opcode_e {{
    {opcodes}
}} __attribute__((__packed__)) opcode_t;

static_assert(sizeof(opcode_t) == 1);


const char *OPNAMES[256] = {{
    {opnames}
}};

unsigned char OPARGS[256] = {{
    {opargs}
}};

#endif // OPCODE_H_GUARD
""".lstrip()


class OpcodeDefParser(object):
    def __init__(self, ifileName=None):
        self.buf = deque()
        self.opcodes = {}
        
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
        line = self.nextLine().strip()
        
        if not line or line.startswith("#"):
            return
        
        print("[DBG]", line, flush=True)
        
        opnum, line = line.split(":", 1)
        
        try:
            opnum = int(opnum, 16)
        except ValueError:
            opnum = int(opnum, 10)
        
        opname, attrs = line.split("(", 1);
        opname = opname.strip()
        attrs = attrs.strip()
        
        assert attrs.endswith(")")
        attrs = attrs[:-1]
        
        attrs = attrs.split(',')  # For the future
        
        assert len(attrs) == 1
        
        assert opnum not in self.opcodes
        self.opcodes[opnum] = (opname, attrs)
    
    def genCCode(self):
        opnames = ',\n    '.join(("{name}".format(name=f'"{self.opcodes[ind][0]}"' if ind in self.opcodes else "NULL") for ind in range(256)));
        
        opargs  = ', '.join(("{argcnt}".format(argcnt=self.opcodes[ind][1][0] if ind in self.opcodes else "0") for ind in range(256)));
        
        #padLength = max(map(lambda x: len(x[0]), self.opcodes
        #opcodes = '\n'.join(("const opcode_t OP_{name} = 0x{ind:02x};".format(name=self.opcodes[ind][0].upper(), ind=ind) for ind in self.opcodes));
        opcodes = ',\n    '.join(("OP_{name} = 0x{ind:02x}".format(name=self.opcodes[ind][0].upper(), ind=ind) for ind in self.opcodes));
        
        
        return CCodeTpl.format(opnames=opnames, opargs=opargs, opcodes=opcodes);


if __name__ == "__main__":
    argParser = argparse.ArgumentParser()
    argParser.add_argument('-i', '--ifile', help='Input .def file name', default="opcodes.def")
    argParser.add_argument('-o', '--ofile', help='Output .h file name', default="opcodes.h")
    args = argParser.parse_args()
    
    parser = OpcodeDefParser(ifileName=args.ifile)
    
    parser.parseAll()
    
    parser.write(args.ofile)