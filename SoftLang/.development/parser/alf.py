from abstractlexer import *
from abstractparser import *

from enum import Enum


class ALFLexer(Lexer):
    class Keyword(Enum):
        # 0 reserved for something, I guess
        DEF = 1
        RET = 2
        VAR = 3
        WHILE = 4
        IF = 5
        INT4 = 6
        INT8 = 7
        DBL = 8
    
    class Punct(Enum):
        # Again, 0 reserved
        COLON = 1
        SEMI = 2
        LPAR = 3
        RPAR = 4
        LSQB = 5
        RSQB = 6
        LBRACE = 7
        RBRACE = 8
        COMMA = 9
        EQ = 10
        ADDEQ = 11
        SUBEQ = 12
        MULEQ = 13
        DIVEQ = 14
        MODEQ = 15
        GEQ = 16
        LEQ = 17
        GT = 18
        LT = 29
        NEQ = 20
        ADD = 21
        SUB = 22
        MUL = 23
        DIV = 24
        MOD = 25
    
    
    syntax = {
        ':': Punct.COLON,
        ';':  Punct.SEMI,
        '(':  Punct.LPAR,
        ')':  Punct.RPAR,
        '[':  Punct.LSQB,
        ']':  Punct.RSQB,
        '{':  Punct.LBRACE,
        '}':  Punct.RBRACE,
        ',':  Punct.COMMA,
        '=':  Punct.EQ,
        '+=': Punct.ADDEQ,
        '-=': Punct.SUBEQ,
        '*=': Punct.MULEQ,
        '/=': Punct.DIVEQ,
        '%=': Punct.MODEQ,
        '>=': Punct.GEQ,
        '<=': Punct.LEQ,
        '>':  Punct.GT,
        '<':  Punct.LT,
        '!=': Punct.NEQ,
        '+':  Punct.ADD,
        '-':  Punct.SUB,
        '*':  Punct.MUL,
        '/':  Punct.DIV,
        '%':  Punct.MOD
    }
    
    keywords = {
        'def': Keyword.DEF,
        'ret': Keyword.RET,
        'var': Keyword.VAR,
        'while': Keyword.WHILE,
        'if': Keyword.IF,
        'int4': Keyword.INT4,
        'int8': Keyword.INT8,
        'dbl': Keyword.DBL
    }


#class ALFParser(abstractparser.Parser):
#    root = None
#    lexerType = ALFLexer


class TmpALFParser(object):
    def __init__(self):
        self.lexer = ALFLexer()
        self.pos = 0
        self.backupPos = 0
    
    def feed(self, data):
        self.lexer.feed(data)
    
    def __len__(self):
        return len(self.lexer.tokens)
    
    def cur(self):
        return self.peek(0)
    
    def peek(self, offset):
        if self.pos + offset in range(len(self)):
            return self.lexer.tokens[self.pos + offset]
        return self.lexer.tokens[-1]
    
    def next(self):
        tmp = self.cur()
        self.pos += 1
        return tmp
    
    def prev(self):
        tmp = self.cur()
        self.pos -= 1
        return tmp
    
    def require(self, stmt):
        if not stmt:
            raise ParseError()
    
    def checkKw(self, kw):
        tmp = cur()
        return isinstance(tmp, KwdTok) and tmp.value == getattr(ALFLexer.Keywords, kw)
    
    def save(self):
        self.backupPos = self.pos
    
    def restore(self):
        self.pos = self.backupPos
    
    def parse(self):
        self.lexer.parse()
        
        result = self.parse_FUNC_DEFS()
        if not isinstance(self.cur(), EndTok):
            raise ParseError()
        
        return result
    
    def parse_FUNC_DEFS(self):
        result = []
        
        while True:
            try:
                result.append(self.parse_FUNC_DEF())
            except ParseError():
                break
        
        return result
    
    def parse_FUNC_DEF(self):
        self.save()
        self.require(self.checkKw("DEF"))
        self.next()
        
        self.restore()
        
    


test = ALFLexer()
test.feed(
    "def int4:main() {\n"
    "    var int4:a;\n"
    "    var int4:b = 5;\n"
    "    b *= 17;\n"
    "    a = (b + 7 - 1) / 2;\n"
    "    ret a + b;"
    "}\n"
)

print(test.parse())
print("[*]", *test.tokens)