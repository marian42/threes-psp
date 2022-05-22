from PIL import Image
import numpy as np

filename = "cards.png"

array = np.array(Image.open(filename))
array.tofile(filename.replace(".png", ".raw"))