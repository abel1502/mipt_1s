class BinTree(object):
    def __init__(self):
        self._data = [None]
    
    def __len__(self):
        return len(self._data) - 1
    
    def __contains__(self, index):
        return isinstance(index, int) and (1 <= index <= len(self) or -len(self) <= index <= -1)
    
    def __getitem__(self, index):
        assert index in self
        return self._data[index]
    
    def __setitem__(self, index, value):
        assert index in self
        self._data[index] = value
    
    def parent(self, index):
        assert index in self
        res = index // 2
        return res if res in self else None
    
    def lChild(self, index):
        assert index in self
        res = index * 2 + 1
        return res if res in self else None
    
    def rChild(self, index):
        assert index in self
        res = index * 2 + 1
        return res if res in self else None
    
    def __str__(self):
        return "[{}]".format(', '.join(map(str, self._data[1:])))


class Heap(BinTree):
    def __init__(self, isReverse=False):
        super().__init__()
        self.isReverse = isReverse
    
    def push(self, value):
        self._data.append(value)
        self.relaxUp(len(self))
    
    def peek(self):
        return self[1]
    
    def pop(self):
        self[1], self[-1] = self[-1], self[1]
        res = self._data.pop()
        self.relaxDown(1)
        return res
    
    def relaxUp(self, ind):
        while ind > 1 and (self[ind] > self[self.parent(ind)]) ^ self.isReverse:
            self[ind], self[self.parent(ind)] = self[self.parent(ind)], self[ind]
            ind = self.parent(ind)
    
    def relaxDown(self, ind):
        while True:
            cur3 = [ind]
            if self.lChild(ind) in self:
                cur3.append(self.lChild(ind))
            if self.rChild(ind) in self:
                cur3.append(self.rChild(ind))
            newInd = (min if self.isReverse else max)(cur3, key=lambda x: self[x])
            
            if ind == newInd:
                break
            
            self[ind], self[newInd] = self[newInd], self[ind]
            ind = newInd
    
    def __str__(self):
        return "{sym}Heap({data})".format(data=BinTree.__str__(self), sym="Min" if self.isReverse else "Max")


#class Struct1(object):
    #def __init__(self):
        #minh = Heap(isReverse=False)
        #maxh = Heap(isReverse=True)
    
    #def insert(self, x):
        #pass
    
    #def getMin(self):
        #return minh.peek()
    
    #def getMax(self):
        #return 
    
    #def extractMin(self):
        #pass


#h = Heap()

#h.push(1)
#h.push(123)
#h.push(17)
#h.push(134)

#print(h.pop())

#print(h)
    

class Struct1(BinTree):
    def insert(self, x):
        pass
    
    def getMin(self):
        ind = 1
        while self.lChild(ind) is not None:
            ind = self.lChild(ind)
        return self[ind]
    
    def getMax(self):
        ind = 1
        while self.rChild(ind) is not None:
            ind = self.rChild(ind)
        return self[ind]

    def extractMin(self):
        pass


s1 = Struct1()
