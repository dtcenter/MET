
###########################################

import sys

class logger():

   PROMPT= " PYTHON:"
   ERROR_PROMPT= "ERROR"

   ##
   ##  create the metadata dictionary
   ##

   @staticmethod
   def append_error_prompt(msg):
      return f'{logger.ERROR_PROMPT}: {msg}'

   @staticmethod
   def error_msg(msg):
      msgs = msg if isinstance(msg, list) else [msg]
      msgs.insert(0, '')
      msgs.append('')
      for a_msg in msgs:
         logger.log_msg(logger.append_error_prompt(a_msg))

   #@staticmethod
   #def get_met_fill_value():
   #   return logger.MET_FILL_VALUE

   @staticmethod
   def log_msg(msg):
      print(f'{logger.PROMPT} {msg}')

   @staticmethod
   def quit(msg):
      logger.error_msg([msg, "Quit..."])
      sys.exit(1)

