import os
import sys

###########################################

print("Python Script:\t" + repr(sys.argv[0]))

   ##
   ##  input file specified on the command line
   ##

if len(sys.argv) != 2:
    print("ERROR: compute_tc_diagnostics.py -> Must specify exactly one input file.")
    sys.exit(1)

# Read the input file
input_file = os.path.expandvars(sys.argv[1])

try:
   print("Input File:\t" + repr(input_file))
except NameError:
   print("Can't find the input file")

# Diagnostics dictionary
tc_diag = {
   'Test100': 100,
   'Test200': 200,
   'TestStr': 'String'
}

print("Diagnostics:\t" + repr(tc_diag))
