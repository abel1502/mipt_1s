import subprocess
import io
import random


def test(cmds, ans):
    inData = "{cmds}\n".format(n=len(cmds), cmds='\n'.join(cmds))
    result = subprocess.run("D.exe", input=inData.encode(), stdout=subprocess.PIPE)
    return result.returncode == 0 and list(result.stdout.decode().splitlines()) == ans


def genTest(minN=1, maxN=10, maxA=50):
    n = random.randrange(minN, maxN)
    cmds = []
    ans = []
    values = [[random.randrange(0, maxA), 0] for _ in range(n)]
    for i in range(n):
        values[i][1] = random.randrange(values[i][0], maxA)
    cmds.append("{}".format(n))
    for i in range(n):
        cmds.append("{} {}".format(*values[i]))
    
    answer = 0
    for l1, r1 in values:
        for l2, r2 in values:
            answer += (l1, r1) != (l2, r2) and l1 <= l2 and r2 <= r1
    
    ans.append(str(answer))
        
    return ans, cmds


def stress(count):
    for i in range(count):
        ans, cmds = genTest(maxN=10, maxA=30)
        
        if not test(cmds, ans):
            print("[Error]", cmds, ans)
            #break


stress(100)


"""

[Error] ['9', '13 15', '24 28', '21 27', '24 29', '17 23', '22 27', '21 27', '1 20', '23 29'] ['6']
[Error] ['9', '15 23', '8 25', '2 23', '24 26', '10 15', '2 23', '15 22', '9 25', '26 29'] ['14']
[Error] ['7', '17 18', '17 20', '3 10', '21 26', '20 20', '3 10', '9 21'] ['5']

7
17 18
17 20
3 10
21 26
20 20
3 10
9 21


3[[    10]]  17[[ 18] 20][]  21[ 26]
    9[                       21]


"""

