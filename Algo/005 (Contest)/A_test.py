import subprocess
import io
import random


def test(cmds, ans):
    inData = "{cmds}\n".format(n=len(cmds), cmds='\n'.join(cmds))
    result = subprocess.run("A.exe", input=inData.encode(), stdout=subprocess.PIPE)
    return result.returncode == 0 and list(result.stdout.decode().splitlines()) == ans


def genTest(minN=1, maxN=10, maxA=10):
    n = random.randrange(minN, maxN)
    cmds = []
    ans = []
    values = set()
    while len(cmds) < n:
        cmd = random.randrange(0, 3) #5)
        curVal = random.randrange(-maxA, maxA)
        if cmd == 0:
            cmds.append("insert {}".format(curVal))
            values.add(curVal)
        elif cmd == 1:
            cmds.append("delete {}".format(curVal))
            values.discard(curVal)
        elif cmd == 2:
            cmds.append("exists {}".format(curVal))
            ans.append("true" if curVal in values else "false")
        #elif cmd == 3:
        #    cmds.append("next {}".format(curVal))
        #    ans.append(str())
        #elif cmd == 4:
        #    cmds.append("prev {}".format(curVal))
        #    ans.append(str())
        
    return ans, cmds


def stress(count):
    for i in range(count):
        ans, cmds = genTest(maxN=50, maxA=10)
        
        if not test(cmds, ans):
            print("[Error]", cmds, ans)
            #break


stress(100)

"""
Assertion failed!

Program: D:\Workspace\.MIPT\s1\Algo\005 (Contest)\A.exe
File: D:\Workspace\.MIPT\s1\Algo\005 (Contest)\A.cpp, Line 293

Expression: rightHeight - 2 <= leftHeight && rightHeight + 2 >= leftHeight
[Error] ['delete 0', 'delete 2', 'insert 9', 'delete 3', 'delete 3', 'insert -7', 'insert 2', 'delete -4', 'insert -4', 'insert -7', 'insert -4', 'exists 4', 'delete -9', 'insert -10', 'insert -6', 'insert 0', 'exists 2', 'insert -4', 'exists -8', 'delete 0', 'delete -8', 'insert -1', 'exists -6', 'delete -4', 'exists -4', 'insert -2', 'delete -4', 'exists -5', 'insert 0', 'exists -1', 'delete -6', 'insert -3', 'insert -7', 'delete 3', 'insert 3', 'insert -3', 'insert 2', 'insert 3', 'exists 9', 'delete 4', 'exists -10', 'insert -5', 'exists -3'] ['false', 'true', 'false', 'true', 'false', 'false', 'true', 'true', 'true', 'true']
[Error]Assertion failed!

Program: D:\Workspace\.MIPT\s1\Algo\005 (Contest)\A.exe
File: D:\Workspace\.MIPT\s1\Algo\005 (Contest)\A.cpp, Line 293

Expression: rightHeight - 2 <= leftHeight && rightHeight + 2 >= leftHeight
 ['exists 2', 'insert 1', 'exists -2', 'insert -2', 'exists -2', 'exists -10', 'exists 1', 'delete 4', 'insert 7', 'insert -10', 'exists -4', 'exists -10', 'delete -10', 'delete 6', 'exists 6', 'insert -1', 'exists 3', 'insert 9', 'delete 9', 'delete 7', 'exists 9', 'insert -8', 'insert -3', 'exists -10', 'exists -10', 'insert 2', 'exists 2', 'exists 0', 'delete 4', 'insert -2', 'exists 2', 'delete -1', 'exists 3', 'insert 2', 'delete 3', 'delete -2', 'exists -4', 'delete 3', 'delete -9', 'exists 2', 'delete -9', 'exists -6', 'exists -3', 'exists -8', 'insert 0'] ['false', 'false', 'true', 'false', 'true', 'false', 'true', 'false', 'false', 'false', 'false', 'false', 'true', 'false', 'true', 'false', 'false', 'true', 'false', 'true', 'true']
[Error] ['exists -9', 'insert 1', 'exists 7', 'insert -7', 'exists -1', 'insert -7', 'exists 1', 'insert -3', 'insert -6', 'delete 2', 'insert 6', 'exists -9', 'delete 4', 'delete 4', 'insert 0', 'exists 9', 'exists -4', 'insert -8', 'exists -1', 'delete -7', 'delete -9', 'exists -10', 'insert -2', 'delete -4', 'delete -10', 'exists 1', 'insert -5', 'insert 0', 'exists -6', 'insert -6', 'exists -1', 'delete -5', 'delete -7', 'insert -8', 'insert 9', 'delete 5', 'insert 3', 'exists 3', 'exists 3', 'exists 0', 'delete 7', 'delete -3', 'exists -7'] ['false', 'false', 'false', 'true', 'false', 'false', 'false', 'false', 'false', 'true', 'true', 'false', 'true', 'true', 'true', 'false']





insert 8
insert -5
insert -9
insert -1
insert 4
insert -10
insert 2
delete -5

"""