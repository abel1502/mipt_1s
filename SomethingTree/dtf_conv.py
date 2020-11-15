import argparse
from enum import Enum
import struct


verbosity = 0


def log(*args, level=1, **kwargs):
    if level <= verbosity:
        print("[DBG]", *args, **kwargs)


class Mode(Enum):
    DTF2DB = 0
    DB2DTF = 1


def run(ifile, ofile, mode):
    assert isinstance(mode, Mode)
    
    if mode == Mode.DTF2DB:
        f = open(ifile, "rb")
        tree = Tree.readDtf(f)
    else:
        f = open(ifile, "r")
        tree = Tree.readDb(f)
    
    f.close()
    
    if mode == Mode.DTF2DB:
        f = open(ofile, "w")
        tree.writeDb(f)
    else:
        f = open(ofile, "wb")
        tree.writeDtf(f)
    
    f.close()


class Tree:
    dtfHeader = struct.Struct("<4sI")
    dtfMagic = b"DTF\n"
    dtfVersion = 0
    
    def __init__(self, root):
        self.root = root
    
    @staticmethod
    def readDtf(ifile):
        header = Tree.dtfHeader.unpack(ifile.read(Tree.dtfHeader.size))
        assert header[0] == Tree.dtfMagic and header[1] == Tree.dtfVersion
        
        return Tree(Node.readDtf(ifile))
    
    @staticmethod
    def readDb(ifile):
        return Tree(Node.readDb(ifile))
    
    def writeDtf(self, ofile):
        header = Tree.dtfHeader.pack(Tree.dtfMagic, Tree.dtfVersion)
        ofile.write(header)
        
        self.root.writeDtf(ofile)
    
    def writeDb(self, ofile):
        self.root.writeDb(ofile)


class Node:
    def __init__(self, value, children):
        assert len(value) < 128
        
        self.value = value
        self.children = tuple(children)
    
    @staticmethod
    def readDtf(ifile):
        header = ifile.read(1)[0]
        header = header & 0b1, (header & 0b11111110) >> 1
        
        value = ifile.read(header[1]).decode()
        children = []
        
        if header[0] == 1:
            children.append(Node.readDtf(ifile))
            children.append(Node.readDtf(ifile))
        
        return Node(value, children)
    
    @staticmethod
    def readUntil(until, ofile):
        buf = []
        
        while not any([x in buf for x in until]):
            buf.append(ofile.read(1))
        
        return ''.join(buf)
    
    @staticmethod
    def readDb(ifile):
        Node.readUntil('"', ifile)
        value = Node.readUntil('"', ifile)[:-1]
        
        isChoice = Node.readUntil('["]', ifile)[-1] == '['
        children = []
        if isChoice:
            children.append(Node.readDb(ifile))
            
            children.append(Node.readDb(ifile))
        else:
            ifile.seek(ifile.tell() - 1)
        
        return Node(value, children)            
    
    def writeDtf(self, ofile):
        log(">", self)
        
        header = ((len(self.children) > 0) & 0b1) | ((len(self.value) << 1) & 0b11111110)
        ofile.write(bytes([header]))
        
        ofile.write(self.value.encode())
        
        for child in self.children:
            child.writeDtf(ofile)
    
    def writeDb(self, ofile):
        ofile.write('"{}"'.format(self.value))
        
        if len(self.children) == 2:
            ofile.write(' [')
            self.children[0].writeDb(ofile)
            ofile.write(' ')
            self.children[1].writeDb(ofile)
            ofile.write(']')
    
    def __str__(self):
        return f"<\"{self.value}\", {self.children}>"
    
    __repr__ = __str__


if __name__ == "__main__":
    argParser = argparse.ArgumentParser()
    argParser.add_argument('-i', '--ifile', help='Input file name', required=True)
    argParser.add_argument('-o', '--ofile', help='Output file name', required=True)
    argParser.add_argument('-v', '--verbose', help='Increase verbosity', action='count', default=0)
    args = argParser.parse_args()
    
    verbosity = args.verbose
    
    mode = Mode.DTF2DB if args.ifile.endswith(".dtf") else Mode.DB2DTF
    
    assert mode == Mode.DTF2DB and args.ifile.endswith(".dtf") and args.ofile.endswith(".db") or \
           mode == Mode.DB2DTF and args.ifile.endswith(".db") and args.ofile.endswith(".dtf")
    
    run(args.ifile, args.ofile, mode)