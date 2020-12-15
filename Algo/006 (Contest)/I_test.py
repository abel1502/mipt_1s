import subprocess
import io
import random


def test(cmds, ans):
    inData = "{cmds}\n".format(n=len(cmds), cmds='\n'.join(cmds))
    result = subprocess.run("I.exe", input=inData.encode(), stdout=subprocess.PIPE)
    return result.returncode == 0 and list(result.stdout.decode().splitlines()) == ans


def genTest(minN=1, maxN=10, minK=1, maxK=10, maxA=50):
    n = random.randrange(minN, maxN)
    k = random.randrange(minK, maxK)
    cmds = []
    ans = []
    values = [random.randrange(0, maxA) for _ in range(n)]
    cmds.append(str(n))
    cmds.extend(map(lambda x: f"{x} 0 0", values))
    cmds.append(str(k))
    for _ in range(k):
        curVal = random.randrange(0, maxA)
        l1 = random.randrange(0, n)
        r1 = random.randrange(l1, n)
        l2 = random.randrange(0, n)
        r2 = random.randrange(l2, n)
        
        cmds.append(f"{l1} {r1} {curVal} 0 0 {l2} {r2}")
        
        values[l1:r1 + 1] = [curVal] * (r1 - l1 + 1)
        
        ans.append(str(min(values[l2:r2 + 1])))
        
    return ans, cmds


def stress(count):
    for i in range(count):
        ans, cmds = genTest(maxN=10, maxA=20)
        
        if not test(cmds, ans):
            print("[Error]", cmds, ans)
            #break


stress(100)


"""
[Error] ['2', '0 0 0', '4 0 0', '1', '0 0 16 0 0 1 1'] ['4']
2
0 0 0
4 0 0
1
0 0 16 0 0 1 1


[Error] ['7', '16 0 0', '7 0 0', '18 0 0', '4 0 0', '4 0 0', '14 0 0', '9 0 0', '6', '0 6 2 0 0 4 5', '4 4 13 0 0 0 6', '1 6 2 0 0 6 6', '2 2 5 0 0 5 5', '0 3 2 0 0 1 4', '5 5 16 0 0 3 4'] ['2', '2', '2', '2', '2', '2']
[Error] ['6', '17 0 0', '15 0 0', '12 0 0', '3 0 0', '3 0 0', '17 0 0', '4', '2 4 18 0 0 1 2', '0 2 11 0 0 3 3', '5 5 19 0 0 2 3', '3 3 3 0 0 5 5'] ['15', '18', '11', '19']
[Error] ['6', '14 0 0', '2 0 0', '4 0 0', '19 0 0', '10 0 0', '17 0 0', '8', '4 5 18 0 0 2 4', '1 5 1 0 0 3 3', '0 4 11 0 0 0 5', '5 5 3 0 0 5 5', '4 5 11 0 0 3 5', '5 5 3 0 0 1 4', '1 5 17 0 0 1 4', '4 4 18 0 0 0 3'] ['4', '1', '1', '3', '11', '11', '17', '11']

[Error] ['5', '16 0 0', '17 0 0', '4 0 0', '11 0 0', '10 0 0', '8', '3 3 17 0 0 4 4', '4 4 19 0 0 0 3', '3 3 12 0 0 1 1', '2 3 17 0 0 0 0', '2 2 14 0 0 1 3', '0 0 13 0 0 0 0', '0 4 6 0 0 1 4', '1 1 11 0 0 4 4'] ['10', '4', '17', '16', '14', '13', '6', '6']
[Error] ['9', '16 0 0', '1 0 0', '9 0 0', '3 0 0', '16 0 0', '15 0 0', '11 0 0', '10 0 0', '7 0 0', '2', '6 7 12 0 0 2 4', '3 8 6 0 0 5 7'] ['3', '6']
[Error] ['7', '12 0 0', '4 0 0', '10 0 0', '15 0 0', '10 0 0', '16 0 0', '7 0 0', '8', '0 4 18 0 0 3 4', '5 6 19 0 0 5 6', '0 5 10 0 0 5 6', '5 6 10 0 0 2 5', '0 4 2 0 0 3 4', '0 6 8 0 0 0 0', '0 1 7 0 0 2 4', '5 5 8 0 0 2 4'] ['18', '19', '10', '10', '2', '8', '8', '8']
[Error] ['5', '9 0 0', '11 0 0', '0 0 0', '2 0 0', '10 0 0', '5', '0 4 4 0 0 3 3', '4 4 15 0 0 1 1', '4 4 13 0 0 0 1', '1 3 13 0 0 0 4', '3 3 15 0 0 3 4'] ['4', '4', '4', '4', '13']

[Error] ['5', '9 0 0', '9 0 0', '4 0 0', '5 0 0', '19 0 0', '3', '0 1 19 0 0 3 4', '2 3 7 0 0 0 2', '2 4 5 0 0 3 3'] ['5', '7', '5']
5
9 0 0
9 0 0
4 0 0
5 0 0
19 0 0
3
0 1 19 0 0 3 4
2 3 7 0 0 0 2
2 4 5 0 0 3 3

"""