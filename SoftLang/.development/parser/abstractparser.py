import abstractlexer

# TODO: finish

class ParseError(Exception):
    pass


class ParserNode(object):
    def match(self, parser):
        pass
    
    def __add__(self, other):
        pass
    
    def __radd__(self, other):
        pass
    
    def __mul__(self, other):
        pass
    
    def __rmul__(self, other):
        pass


class ElementNode(ParserNode):
    pass

class AndNode(ParserNode):
    pass

class OrNode(ParserNode):
    pass

class RepeatNode(ParserNode):
    pass

class KeywordNode(ParserNode):
    def __init__(self, kwd):
        self.kwd = kwd
    
    def match(self, parser):
        if isinstance(parser.cur(), parser.lexerType.)

class NameNode(ParserNode):
    pass

class PunctNode(ParserNode):
    pass

class NumNode(ParserNode):
    pass


class Parser(object):
    root = None
    lexerType = abstractlexer.Lexer
    
    def __init__(self):
        self.lexer = self.lexerType()
        self.pos = 0
    
    def feed(self, data):
        self.lexer.feed(data)
    
    def cur(self):
        return peek(0)
    
    def peek(self, offset):
        if self.pos + offset in range(len(self.lexer.tokens)):
            return self.lexer.tokens[self.pos + offset]
        return self.lexer.tokens[-1]
    
    def next(self):
        tmp = self.cur()
        self.pos += 1
        return tmp
    
    def prev(self):
        tmp = self.cur()
        assert self.pos > 0
        self.pos -= 1
        return tmp
    
    def parse(self):
        self.lexer.parse()
        if self.lexer.getError():
            print("Syntax error.")
            return None
        result = root.match(self)
        if not isinstance(self.cur(), EndTok):
            raise ParseError

