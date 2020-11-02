import random

screenWidth = 28
screenHeight = 28

drawBuf = ['\0\0' for _ in range(screenWidth * screenHeight)]

def cldb():
    for i in range(len(drawBuf)):
        drawBuf[i] = '  '

def draw():
    for y in range(screenHeight):
        print(''.join(drawBuf[screenWidth * y:screenWidth * (y + 1)]))

def drawPixel(x, y, pixel):
    drawBuf[y * screenWidth + x] = pixel

def drawRect(x1, y1, x2, y2, pixel):
    for x in range(y1, y2 + 1):
        for y in range(x1, x2 + 1):
            drawPixel(x, y, pixel)

def drawRooms(x1, y1, x2, y2, orientation):
    #orientation = random.randrange(0, 2)
    if orientation == 0 and x2 - x1 >= 8:
        split = random.randrange(x1 + 4, x2 - 4 + 1)
        drawRooms(x1, y1, split, y2, 1 - orientation)
        drawRooms(split, y1, x2, y2, 1 - orientation)
        return
    if orientation == 1 and y2 - y1 >= 8:
        split = random.randrange(y1 + 4, y2 - 4 + 1)
        drawRooms(x1, y1, x2, split, 1 - orientation)
        drawRooms(x1, split, x2, y2, 1 - orientation)
        return
    if random.randrange(0, 6) >= 1:
        drawRect(x1 + 1, y1 + 1, x2 - 1, y2 - 1, '  ')
    return


cldb()
drawRect(0, 0, screenWidth - 1, screenHeight - 1, '##')
drawRooms(0, 0, screenWidth - 1, screenHeight - 1, 1)
draw()