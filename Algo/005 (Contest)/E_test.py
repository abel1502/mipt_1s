import subprocess
import io
import random


def test(cmds, m, ans):
    inData = "9223372036854775807 {m}\n{k}\n{cmds}\n".format(m=m, k=len(cmds), cmds='\n'.join(cmds))
    result = subprocess.run("E.exe", input=inData.encode(), stdout=subprocess.PIPE)
    return result.returncode == 0 and list(map(lambda x: x.strip(), result.stdout.decode().splitlines())) == ans


def genTest(minK=1, maxK=10, minM=1, maxM=10, maxA=10):
    k = random.randrange(minK, maxK)
    m = random.randrange(minM, maxM)
    cmds = []
    ans = []
    values = [set() for _ in range(m + 1)]
    while len(cmds) < k:
        cmd = random.randrange(0, 5)
        if cmd == 0:
            s = random.randrange(0, m + 1)
            e = random.randrange(0, maxA)
            cmds.append("ADD {} {}".format(e, s))
            values[s].add(e)
        elif cmd == 1:
            s = random.randrange(0, m + 1)
            if not values[s]:
                continue
            e = random.choice(list(values[s]))
            cmds.append("DELETE {} {}".format(e, s))
            values[s].remove(e)
        elif cmd == 2:
            s = random.randrange(0, m + 1)
            cmds.append("CLEAR {}".format(s))
            values[s].clear()
        elif cmd == 3:
            s = random.randrange(0, m + 1)
            cmds.append("LISTSET {}".format(s))
            ans.append(" ".join(map(str, sorted(values[s]))) if values[s] else "-1")
        elif cmd == 4:
            e = random.randrange(0, maxA)
            cmds.append("LISTSETSOF {}".format(e))
            tmp = [str(i) for i in range(m + 1) if e in values[i]]
            ans.append(" ".join(tmp) if tmp else "-1")
        
    return cmds, m, ans


def stress(count):
    for i in range(count):
        curTest = genTest(maxK=20, maxA=7)
        
        if not test(*curTest):
            print("[Error]", *curTest)
            print("9223372036854775807 {m}\n{k}\n{cmds}\n".format(m=curTest[1], k=len(curTest[0]), cmds='\n'.join(curTest[0])))
            #break


stress(100)

"""
[Error] ['

CLEAR 3
LISTSETSOF 3
CLEAR 3
CLEAR 4
ADD 4 6
LISTSETSOF 7
LISTSET 0
DELETE 4 6
LISTSETSOF 4
LISTSET 6
LISTSET 7
ADD 4 8
LISTSET 2
CLEAR 5
ADD 4 2
CLEAR 5
LISTSETSOF 3
ADD 1 6
ADD 4 7
CLEAR 7
CLEAR 4
ADD 7 2
LISTSETSOF 4
DELETE 4 8
ADD 7 6



'] 8 ['-1', '-1', '-1', '-1', '-1', '-1', '-1', '-1', '2 8']



[Error] ['ADD 4 0', 'LISTSET 1', 'LISTSETSOF 4', 'ADD 6 0', 'LISTSET 0', 'LISTSETSOF 1', 'LISTSETSOF 0', 'ADD 4 0', 'ADD 2 0', 'DELETE 4 0', 'CLEAR 0', 'LISTSETSOF 1', 'LISTSETSOF 6', 'ADD 5 0', 'LISTSETSOF 3'] 1 ['-1', '0', '4 6', '-1', '-1', '-1', '-1', '-1']
9223372036854775807 1
15
ADD 4 0
LISTSET 1
LISTSETSOF 4
ADD 6 0
LISTSET 0
LISTSETSOF 1
LISTSETSOF 0
ADD 4 0
ADD 2 0
DELETE 4 0
CLEAR 0
LISTSETSOF 1
LISTSETSOF 6
ADD 5 0
LISTSETSOF 3


"""