
import numpy as np

###########################################

   ##
   ##  load the data into the numpy array
   ##
try:
    met_data = np.loadtxt("/var/autofs/mnt/dakota_d3/projects/MET/MET_test_data/unit_test/python/letter.txt")
except NameError:
    print("Can't find the input file")


print met_data.shape

print met_data.dtype

#print met_data


###########################################

   ##
   ##  create the metadata dictionary
   ##

attrs = {

   'valid': '20050807_120000', 
   'init':  '20050807_000000', 
   'lead':   120000, 
   'accum': '120000', 

   'name': 'Foo', 

   'grid': {


      'type': 'Lambert Conformal', 
      'hemisphere': 'N', 

      'name': 'FooGrid', 

      'scale_lat_1': 25.0, 
      'scale_lat_2': 25.0, 

      'lat_pin': 12.19, 
      'lon_pin': -135.459, 

      'x_pin': 0.0, 
      'y_pin': 0.0, 

      'lon_orient': -95.0, 

      'd_km': 40.635, 
      'r_km': 6371.2, 

      'nx': 185, 
      'ny': 129, 
    

   }

}


print attrs


