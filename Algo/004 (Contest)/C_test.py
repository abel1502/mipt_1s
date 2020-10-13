import subprocess
import io
import random


def test(arr, k):
    inData = "{n} {k}\n{arr}\n".format(n=len(arr), k=k, arr='\n'.join(map(str, arr)))
    result = subprocess.run("C.exe", input=inData.encode(), stdout=subprocess.PIPE)
    return result.returncode == 0 and int(result.stdout) == sorted(arr)[k]


def genTest(minN=1, maxN=16, minA=0, maxA=10 ** 9):
    n = random.randrange(minN, maxN)
    k = random.randrange(0, n)
    arr = []
    for i in range(n):
        arr.append(random.randrange(minA, maxA))
    
    return arr, k


def stress(count):
    for i in range(count):
        arr, k = genTest(maxA=25)
        
        if not test(arr, k):
            print("[Error]", arr, k)
            #break


assert test([3, 5, 7, 9], 3)
assert test([3, 6, 5, 7, 2, 9, 8, 10, 4, 1], 0)

stress(100)
# [Error] [34, 20, 90] 1
"""
3 1
34
20
90

"""

# [Error] [5, 24, 89, 92, 24] 3
"""
5 3
5
24
89
92
24

"""

# [Error] [4, 6, 3, 17] 0
"""
4 0
4
6
3
17

"""
