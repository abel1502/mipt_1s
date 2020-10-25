import subprocess
import io
import random

INF = 10 ** 20

def test(l, k, cells, ans):
    inData = "{l} {n} {k}\n{cells}\n".format(n=len(cells), l=l, k=k, cells=' '.join(map(str, cells)))
    result = subprocess.run("F.exe", input=inData.encode(), stdout=subprocess.PIPE)
    d, c, s = map(int, result.stdout.decode().split())
    res = result.returncode == 0 and d == ans[0] and c == ans[1] # and s == ans[1]
    
    if not res:
        print("!", d, c, s)
    
    return res


def solve(l, k, cells):
    cells = set(cells)
    buf = [int((i % l + 1)  in cells) for i in range(3 * l)]
    
    blkSize = l // k
    
    minDelta = INF
    minStart = 0
    minCnt = 0
    
    for offset in range(blkSize):
        minBlk = INF
        maxBlk = -INF
        
        for blk in range(k):
            curBlk = sum(buf[blk * blkSize + offset:(blk + 1) * blkSize + offset])
            minBlk = min(minBlk, curBlk)
            maxBlk = max(maxBlk, curBlk)
    
        curDelta = maxBlk - minBlk
        
        if curDelta < minDelta:
            minDelta = curDelta
            minStart = offset
            minCnt = 0
        
        if curDelta == minDelta:
            minCnt += 1
    
    return (minDelta, minCnt * k, minStart + 1)
            


def genTest(bndN=(1, 20), bndK=(1,10), bndL=(1,10)):
    n = random.randrange(*bndN)
    k = random.randrange(*bndK)
    l = k * random.randrange(*bndL)
    
    cells = list(range(1, l + 1))
    random.shuffle(cells)
    cells = cells[:n]
    
    ans = solve(l, k, cells)
     
    return (l, k, cells), ans


def stress(count):
    for i in range(count):
        data, ans = genTest()
        
        if not test(*data, ans):
            print("[Error]", data, ans)
            #break


stress(100)
exit()

data = (16, 8, [8, 10, 15, 16, 4, 11])

"""
16 6 8
8 10 15 16 4 11

"""

ans = solve(*data)

print(test(*data, ans))

print(data, ans)

"""
[Error] (18, 3, [18, 8, 11, 2, 17, 10, 15, 5, 16]) (0, 1, 6)
[Error] (80, 5, [61, 2]) (0, 1, 16)

[Error] (9, 3, [9, 1]) (0, 1, 3)

[Error] (2, 2, [1, 2]) (0, 1, 1)

[Error] (8, 8, [3, 6]) (1, 1, 1)

[Error] (24, 3, [4, 5, 17, 23, 7, 24, 12, 10, 8, 2, 9]) (2, 3, 1)

"""