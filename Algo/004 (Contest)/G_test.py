import subprocess
import io
import random


def test(cmds, ans):
    inData = "{n}\n{cmds}\n".format(n=len(cmds), cmds='\n'.join(cmds))
    result = subprocess.run("G.exe", input=inData.encode(), stdout=subprocess.PIPE)
    return result.returncode == 0 and list(map(int, result.stdout.decode().split('\r\n')[:-1])) == ans


def genTest(minN=1, maxN=16, maxA=100):
    n = random.randrange(minN, maxN)
    cmds = []
    ans = []
    values = []
    while len(cmds) < n:
        cmd = random.randrange(0, 3 if values else 1)
        if cmd == 0:
            curVal = random.randrange(1, maxA)
            cmds.append("Insert({})".format(curVal))
            values.append(curVal)
        elif cmd == 1:
            cmds.append("GetMax")
            ans.append(max(values))
            values.remove(max(values))
        elif cmd == 2:
            cmds.append("GetMin")
            ans.append(min(values))
            values.remove(min(values))
    return ans, cmds


def stress(count):
    for i in range(count):
        ans, cmds = genTest(maxA=100)
        
        if not test(cmds, ans):
            print("[Error]", cmds, ans)
            #break


stress(100)

# [Error] ['Insert(13)', 'GetMin', 'Insert(96)', 'Insert(25)', 'Insert(81)', 'Insert(93)', 'GetMax', 'GetMin', 'Insert(92)', 'GetMin', 'GetMax', 'GetMax', 'Insert(20)'] [13, 96, 25, 81, 93, 92]

"""
12
Insert(13)
GetMin
Insert(96)
Insert(25)
Insert(81)
Insert(93)
GetMax
GetMin
Insert(92)
GetMin
GetMax
GetMax

"""

"""
13
Insert(13)
GetMin
Insert(96)
Insert(25)
Insert(81)
Insert(93)
dump
GetMax
dump
GetMin
dump
Insert(92)
dump
GetMin
dump
GetMax
dump
GetMax
dump

"""

# [13, 96, 25, 81, 93, 92]
