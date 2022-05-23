from PIL import Image
import numpy as np

filenames = ["img/cards.png", "img/spritesheet.png"]

for filename in filenames:
    array = np.array(Image.open(filename))
    array.tofile(filename.replace(".png", ".raw"))