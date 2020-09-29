def addAP(l, r, b, d):
    delta[l] += b
    delta[r + 1] -= b + d * (r + 1 - l)
    deltaDiff[l] += d
    deltaDiff[r + 1] -= d

def applyAll():
    curDelta = 0
    curDiff = 0
    
    for i in range(n):
        curDelta += delta[i] + curDiff
        curDiff += deltaDiff[i]
        #a[i] += curDelta
        a[i] = curDelta
        

def log():
    applyAll()
    print(' '.join(map(str, a)))

n = 5
a = [0 for _ in range(n)]
delta = [0 for _ in range(n + 1)]
deltaDiff = [0 for _ in range(n + 1)]


addAP(1, 3, 5, 1)
log()

addAP(0, 2, 0, 2)
log()

