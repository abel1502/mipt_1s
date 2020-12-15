import subprocess
import io
import random


def test(cmds, ans):
    inData = "{cmds}\n".format(n=len(cmds), cmds='\n'.join(cmds))
    result = subprocess.run("C.exe", input=inData.encode(), stdout=subprocess.PIPE)
    return result.returncode == 0 and list(result.stdout.decode().splitlines()) == ans


def genTest(minN=1, maxN=10, minM=1, maxM=10, maxA=50):
    n = random.randrange(minN, maxN)
    m = random.randrange(minM, maxM)
    cmds = []
    ans = []
    values = [random.randrange(0, maxA) for _ in range(n)]
    cmds.append("{} {}".format(n, m))
    cmds.append(" ".join(map(str, values)))
    for _ in range(m):
        cmd = random.randrange(0, 2)
        if cmd == 0:
            ind = random.randrange(0, n)
            curVal = random.randrange(0, maxA)
            cmds.append("0 {} {}".format(ind + 1, curVal))
            values[ind] = curVal
        elif cmd == 1:
            ind = random.randrange(0, n)
            curVal = random.randrange(0, maxA)
            cmds.append("1 {} {}".format(ind + 1, curVal))
            
            result = [k + 1 for k in range(ind, n) if values[k] >= curVal]
            if (not result):
                result = [-1]
            result = min(result)
            
            ans.append(str(result))
        
    return ans, cmds


def stress(count):
    for i in range(count):
        ans, cmds = genTest(maxN=10, maxA=30)
        
        if not test(cmds, ans):
            print("[Error]", cmds, ans)
            #break


stress(100)

# [Error] ['1 2', '26', '1 0 0', '1 0 15'] ['0', '0']

"""
[Error]Assertion failed!

Program: D:\Workspace\.MIPT\s1\Algo\006 (Contest)\C.exe
File: D:\Workspace\.MIPT\s1\Algo\006 (Contest)\C.cpp, Line 363

Expression: !isStub()
 ['4 5', '13 21 12 5', '0 0 4', '0 0 23', '1 1 0', '0 2 18', '1 1 24'] ['1', '-1']

4 5
13 21 12 5
0 0 4
0 0 23
1 1 0
0 2 18
1 1 24


['7 2', '24 3 21 10 29 4 10', '0 6 21', '1 4 8'] ['3']
7 2
24 3 21 10 29 4 10
0 6 21
1 4 8

"""