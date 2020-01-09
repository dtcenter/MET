

################################################
#
#    Adapted from a script provided by George McCabe
#    Adapted by Randy Bullock
#
#    usage:  /path/to/python write_pickle.py pickle_output_filename <user_python_script>.py <args>
#
################################################


import sys
import pickle

pickle_filename = sys.argv[1];

print('old args')
print(sys.argv)

pyembed_module_name = sys.argv[2].replace('.py','')
sys.argv = sys.argv[2:]

print('new args')
print(sys.argv)

met_in = __import__(pyembed_module_name)

#met_info = { 'attrs': met_in.attrs, 'met_data': met_in.met_data }
#
#print(met_info)
#
#pickle.dump( met_info, open( pickle_filename, "wb" ) )


pickle.dump( met_in.point_data, open( pickle_filename, "wb" ) )




