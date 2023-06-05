#import pillow
from PIL import Image


def outputCArray(img, name):
    print("const uint8_t " + name + "[" + str(width * height) + "] = {")
    for y in range(height):
        for x in range(width):
            print(str(img.getpixel((x, y))) + ", ", end="")
        print()
    print("};")