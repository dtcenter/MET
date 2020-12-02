from __future__ import print_function

import os
import sys
import re
import numpy as np

###########################################
#
#  Convert input NCL colormap (*.rgb) files to
#  MET-friendly color table (*.ctable) files.
#
#  NCL colormap file source:
#    https://github.com/NCAR/ncl/tree/develop/ni/src/db/colormaps
#
###########################################

if len(sys.argv) == 3:

    # Store the input/output files
    rgb_in     = os.path.expandvars(sys.argv[1])
    ctable_out = os.path.expandvars(sys.argv[2])

    try:

        print("Input RGB:\t"     + repr(rgb_in))
        print("Output CTable:\t" + repr(ctable_out))

        # Locate RGB header line
        n_skip = 0
        with open(rgb_in) as txtfile:
            for num, line in enumerate(txtfile, 1):
                if re.search("^[#,;] *[r,R] *[g,G] *[b,B]", line):
                    n_skip = num

        # Otherwise, find ncolors header line
        if n_skip == 0:
            with open(rgb_in) as txtfile:
                for num, line in enumerate(txtfile, 1):
                    if re.search("^ncolor", line):
                        n_skip = num

        print("Skip " + repr(n_skip) + " header lines.")

        # Read input file
        rgb = np.loadtxt(rgb_in, dtype='float64', skiprows=n_skip, usecols=(0,1,2))

        print("Dimensions:\t" + repr(rgb.shape))

    except NameError:

        print("Trouble reading input RGB file: " + rgb_in)

else:

    print("Must specify one input RGB and output color table file.")
    sys.exit(1)

###########################################

# Write output color table.

ctable = open(ctable_out, 'w')

ctable.writelines("//\n" + \
                  "// Derived from NCL colormap \"" + \
                   os.path.basename(rgb_in) + "\"\n" + \
                  "//\n")

n = rgb.shape[0]
for i in range(n):
    if rgb.max() > 1:
        r, g, b = rgb[i]
    else:
        r, g, b = rgb[i] * 255.0
    line = "{:.4f} {:.4f} {{ {:3.0f}, {:3.0f}, {:3.0f} }}\n".format(i/n, (i+1)/n, r, g, b)
    ctable.writelines(line)

ctable.close()

