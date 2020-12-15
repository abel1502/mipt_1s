class Token(object):
    def __repr__(self):
        return str(self)


class PunctTok(Token):
    def __init__(self, value):
        self.value = value
    
    def __str__(self):
        return "P<{}>".format(self.value)

class KwdTok(Token):
    def __init__(self, value):
        self.value = value
    
    def __str__(self):
        return "K<{}>".format(self.value)

class NameTok(Token):
    def __init__(self, value):
        self.value = value
    
    def __str__(self):
        return "N<{}>".format(self.value)    

class NumTok(Token):
    def __init__(self, integer, fraction, exp, isFloat):
        self.integer = integer
        self.fraction = fraction
        self.exp = exp
        self.isFloat = isFloat
    
    def __str__(self):
        if self.isFloat:
            return "#<{}{}{}{}>".format(self.integer,
                                        "." + str(self.fraction) if self.fraction != 0 else "",
                                        "e" + str(self.exp) if self.exp != 0 else "",
                                        "." if self.exp == self.fraction == 0 else "")
        else:
            return "#<{}>".format(self.integer)
    
    def asInt4(self):
        return self.integer
    
    def asInt8(self):
        return self.integer
    
    def asDbl(self):
        return double('{}.{}e{}'.format(self.integer, self.fraction, self.exp))

class ErrTok(Token):
    def __init__(self):
        pass
    
    def __str__(self):
        return "E"

class EndTok(Token):
    def __init__(self):
        pass
    
    def __str__(self):
        return "."


class Lexer(object):
    syntax = dict()
    keywords = dict()
    
    def __init__(self):
        self.tokens = []
        self.buf = ""
        self.pos = 0
    
    def getError(self):
        if not self.tokens:
            return False
        
        assert isinstance(self.tokens[-1], (ErrTok, EndTok))
        
        return isinstance(self.tokens[-1], ErrTok)
    
    def feed(self, data):
        self.buf += data
    
    def parse(self):
        while True:
            tmp = self.nextTok()
            self.tokens.append(tmp)
            if isinstance(tmp, ErrTok) or isinstance(tmp, EndTok):
                break
    
    def isEnd(self):
        return self.pos >= len(self.buf)
    
    def cur(self):
        return self.peek(0)
    
    def peek(self, offset):
        if self.pos + offset in range(len(self.buf)):
            return self.buf[self.pos + offset]
        return '\0'
    
    def next(self):
        tmp = self.cur()
        self.pos += 1
        return tmp
    
    def prev(self):
        tmp = self.cur()
        self.pos -= 1
        return tmp
    
    def nextTok(self):
        self.skipSpace()
        
        tmp = self.cur()
        if tmp == '\0':
            self.next()
            return EndTok()
        if tmp.isalpha():
            return self.nextIdentifier()
        if tmp.isdigit():
            return self.nextNumber()
        return self.nextPunct()
    
    def skipSpace(self):
        while self.cur().isspace():
            self.next()
    
    def nextIdentifier(self):
        assert self.cur().isalpha()
        tmp = []
        while self.cur().isalpha() or self.cur().isdigit() or self.cur() == "_":
            tmp.append(self.next())
        tmp = "".join(tmp)
        if tmp in self.keywords:
            return KwdTok(self.keywords[tmp])
        return NameTok(tmp)
    
    @staticmethod
    def _parseDigit(base, c):
        assert base in {2, 8, 10, 16} and isinstance(c, str) and len(c) == 1
        
        if c.isdigit():
            result = ord(c) - ord('0')
            if result >= base:
                return -1
            return result
        if base != 16:
            return -1
        if 'A' <= c <= 'F':
            return c - 'A' + 10
        if 'a' <= c <= 'f':
            return c - 'a' + 10
        
        return -1
    
    def nextNumber(self): # TODO: actually, this should work differently: fraction should be a double itself, because otherwise leading zeroes are ignored
        assert self.cur().isdigit()
        
        base = 10
        if self.cur() == '0' and self.peek(1) in 'box':
            self.next()
            baseChar = self.next()
            if baseChar == 'b':
                base = 2
            elif baseChar == 'o':
                base = 8
            elif baseChar == 'x':
                base = 16
            else:
                assert False
        
        integer = 0
        integerLen = 0
        
        curDigit = self._parseDigit(base, self.cur())
        while curDigit >= 0: # TODO: limit
            integerLen += 1
            integer = integer * base + curDigit
            self.next()
            curDigit = self._parseDigit(base, self.cur())
        
        isFloat = False
        fraction = 0
        exp = 0
        
        if self.cur() == '.' and (integerLen > 0 or self._parseDigit(base, self.peek(1).isdigit()) >= 0):
            isFloat = True
            
            self.next()
            
            curDigit = self._parseDigit(base, self.cur())
            while curDigit >= 0:  # TODO: limit?
                fraction = base * fraction + curDigit
                self.next()
                curDigit = self._parseDigit(base, self.cur())
        
        if self.cur() == 'e' or self.cur() == 'E':  # TODO: WARNING: in hexadecimal you won't be able to define floats this way, since e is a valid digit
            isFloat = True                          # Actually, exp should probably only be allowed in decimal literals, because it's hard to take into account with arbitrary bases
                                                    # Also, we're currently not parsing negative exponent...
            self.next()
            
            curDigit = self._parseDigit(base, self.cur())
            if curDigit < 0:
                return ErrTok()
            
            while curDigit >= 0:  # TODO: limit?
                exp = base * exp + curDigit
                self.next()
                curDigit = self._parseDigit(base, self.cur())
        
        if self.cur().isalpha():  # a number can't have an alpha prefix for now
            return ErrTok()
        
        return NumTok(integer, fraction, exp, isFloat)
    
    def nextPunct(self):
        if not self.syntax:
            return ErrTok()
        
        maxLen = max(map(len, self.syntax))
        
        curPunct = ""
        for i in range(maxLen):
            curPunct += self.peek(i)
        
        bestMatch = None
        bestMatchLen = 0
        
        for punct in self.syntax:
            if curPunct.startswith(punct) and len(punct) > bestMatchLen:
                bestMatch = self.syntax[punct]
                bestMatchLen = len(punct)
        
        for i in range(bestMatchLen):
            self.next()
        
        if bestMatchLen == 0:
            return ErrTok()
        
        return PunctTok(bestMatch)



