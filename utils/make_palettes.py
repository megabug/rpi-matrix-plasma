#!/usr/bin/env python3

import os
import os.path
import sys

import matplotlib.cm


COLORMAP_NAMES = {
    "viridis",
    "plasma",
    "inferno",
    "magma",
    "cividis",
    "Greys",
    "Purples",
    "Blues",
    "Greens",
    "Oranges",
    "Reds",
    "YlOrBr",
    "YlOrRd",
    "OrRd",
    "PuRd",
    "RdPu",
    "BuPu",
    "GnBu",
    "PuBu",
    "YlGnBu",
    "PuBuGn",
    "BuGn",
    "YlGn",
    "binary",
    "gist_yarg",
    "gist_gray",
    "gray",
    "bone",
    "pink",
    "spring",
    "summer",
    "autumn",
    "winter",
    "cool",
    "Wistia",
    "hot",
    "afmhot",
    "gist_heat",
    "copper",
    "PiYG",
    "PRGn",
    "BrBG",
    "PuOr",
    "RdGy",
    "RdBu",
    "RdYlBu",
    "RdYlGn",
    "Spectral",
    "coolwarm",
    "bwr",
    "seismic",
    "twilight",
    "twilight_shifted",
    "hsv",
    "Pastel1",
    "Pastel2",
    "Paired",
    "Accent",
    "Dark2",
    "Set1",
    "Set2",
    "Set3",
    "tab10",
    "tab20",
    "tab20b",
    "tab20c",
    "flag",
    "prism",
    "ocean",
    "gist_earth",
    "terrain",
    "gist_stern",
    "gnuplot",
    "gnuplot2",
    "CMRmap",
    "cubehelix",
    "brg",
    "gist_rainbow",
    "rainbow",
    "jet",
    "turbo",
    "nipy_spectral",
    "gist_ncar",
}

PALETTE_SIZE = 2048

COLOR_ADJUST = (1.0, 0.9, 0.9)

PALETTE_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "palettes")


os.makedirs(PALETTE_DIR, exist_ok=True)

for colormap_name in COLORMAP_NAMES:
    print(f"{colormap_name}...", file=sys.stderr)

    colormap = matplotlib.cm.get_cmap(colormap_name, PALETTE_SIZE)
    with open(os.path.join(PALETTE_DIR, f"{colormap_name}.txt"), "w") as f:
        for i in range(PALETTE_SIZE):
            c = colormap(i)
            print(
                " ".join(str(int(v * a * 255)) for v, a in zip(c[:3], COLOR_ADJUST)),
                file=f,
            )
