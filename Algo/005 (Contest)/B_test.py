import subprocess
import io
import random


def test(cmds, ans):
    inData = "{cmds}\n".format(n=len(cmds), cmds='\n'.join(cmds))
    result = subprocess.run("B.exe", input=inData.encode(), stdout=subprocess.PIPE)
    return result.returncode == 0 and list(result.stdout.decode().splitlines()) == ans


def genTest(minN=1, maxN=10, maxA=10):
    n = random.randrange(minN, maxN)
    cmds = []
    ans = []
    values = set()
    cmds.append(str(n))
    while len(cmds) < n + 1:
        cmd = random.randrange(0, 2)
        if cmd == 0:
            curVal = random.randrange(0, maxA)
            cmds.append("+ {}".format(curVal))
            values.add(curVal)
        elif cmd == 1:
            left = random.randrange(-5, maxA)
            right = left + random.randrange(maxA - left)
            cmds.append("? {} {}".format(left, right))
            ans.append(str(sum([i for i in values if left <= i <= right])))
        
    return ans, cmds


def stress(count):
    for i in range(count):
        ans, cmds = genTest(maxN=10, maxA=30)
        
        if not test(cmds, ans):
            print("[Error]", cmds, ans)
            #break


stress(100)

# [Error] ['3', '+ 12', '+ 26', '? -1 13'] ['12']

# [Error] ['6', '+ 14', '? 11 22', '+ 11', '? 22 25', '? 19 27', '? -4 13'] ['14', '0', '0', '11']