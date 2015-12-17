#!/usr/bin/env python

# Synopsis
# --------------------------------------------------------------------------------
# Converts a timing node describing XML file into a MIF and HEX file.
# This can be loaded into the IO_CONTROL ROM.

# Imports
# --------------------------------------------------------------------------------
from xml.dom import minidom
from xml.dom.minidom import parse, parseString
import datetime
import os
import sys
import getopt
import math
import pdb
import shutil
  
# Function main(...)
# --------------------------------------------------------------------------------
def main(argv):
  
  # Constants for shifting (see README for details)
  io_dir_shift          = 6
  io_oe_shift           = 5
  io_term_shift         = 4
  io_special_shift      = 3
  io_channel_shift      = 0
  io_logic_level_shift  = 4
  io_reserved_shift     = 0
  io_const_end_of_table = "END___OF___TABLE"
  bin_bytes_per_line    = 4
  mif_target_dir        = "../hdl"
  
  # Check if a file name was given as argument
  if (len(sys.argv) == 2):
    xml_filename = sys.argv[1]
    hex_filename = xml_filename
    bin_filename = xml_filename
    mif_filename = xml_filename
  else:
    print "Missing configuration file! Try: %s configuration.xml" % sys.argv[0]
    exit(1)
  
  # Check if the file is readable
  if os.path.isfile(xml_filename) and os.access(xml_filename, os.R_OK):
    print "Name          Delay[ns]   Internal ID   Direction   OE    Term   Special   Channel   Logic Level"
    print "------------------------------------------------------------------------------------------------"
  else:
    print "Either configuration file is missing or is not readable!"
    exit(1)
    
  # Create hex version
  hex_filename = hex_filename.replace("xml", "hex")
  fp_hex = open(hex_filename, "wb+")
  
  # Create device to hex file (32 bytes)
  hex_header = hex_filename.replace(".hex", "") 
  if len(hex_header) > 31: # Check file name lentj
    print "Please use a sorter input file name (less then 31 characters)!"
    exit(1)
  else:
    # Add pads to reach the 32 bytes
    fp_hex.write(hex_header.ljust(32, '\0'));
  
  # Open file
  doc = minidom.parse(xml_filename)
  
  # Parse file
  ios = doc.getElementsByTagName("io")
  for io in ios:
    # Get elements
    name        = io.getAttribute('name')
    delay      = io.getAttribute('delay')
    internal_id = io.getAttribute('internal_id')
    direction   = io.getAttribute('direction')
    oe          = io.getAttribute('oe')
    term        = io.getAttribute('term')
    special     = io.getAttribute('special')
    channel     = io.getAttribute('channel')
    logic_level = io.getAttribute('logic_level')
    
    # Format properties
    print "%s %s      %s           %s      %s   %s    %s       %s    %s" % ('{0: <13}'.format(name), '{:<6}'.format(delay), 
    '{:<3}'.format(internal_id), '{0: <6}'.format(direction), '{0: <3}'.format(oe),
    '{0: <3}'.format(term), '{0: <3}'.format(special), '{0: <6}'.format(channel),
    '{0: <10}'.format(logic_level))
    
    # Write properties to the hex file
    fp_hex.write(name.ljust(12, '\0'));
    fp_hex.write(chr(int(delay)));
    fp_hex.write(chr(int(internal_id)));
    
    # Check the direction
    if direction == 'Output':
      b_io_dir = 0
    elif direction == 'Input':
      b_io_dir = 1
    elif direction == 'Inout':
      b_io_dir = 2
    else:
      print "IO direction unknown. Please use Output, Input or Inout!"
      exit(1)
      
    # Check if oe is available
    if oe == 'Yes':
      b_io_oe = 1
    elif oe == 'No':
      b_io_oe = 0
    else:
      print "OE (output enable) availability unknown. Please use Yes or No!"
      exit(1)
    
    # Check if termination is available 
    if term == 'Yes':
      b_io_term = 1
    elif term == 'No':
      b_io_term = 0
    else:
      print "Termination availability unknown. Please use Yes or No!"
      exit(1)
    
    # Check if special function is available 
    if special== 'Yes':
      b_io_special = 1
    elif special == 'No':
      b_io_special = 0
    else:
      print "Special function availability unknown. Please use Yes or No!"
      exit(1)
      
    # Check channel
    if channel == 'GPIO':
      b_io_channel = 0
    elif channel == 'LVDS':
      b_io_channel = 1
    elif channel == 'FIXED':
      b_io_channel = 7
    else:
      print "Channel unknown. Please use GPIO, LVDS or FIXED!"
      exit(1)
      
    # Check logic level
    if logic_level== 'TTL':
      b_io_logic_level = 0
    elif logic_level == 'LVTTL':
      b_io_logic_level = 1
    elif logic_level == 'LVDS':
      b_io_logic_level = 2
    elif logic_level== 'NIM':
      b_io_logic_level = 3
    else:
      print "Logic level property unknown. Please use TTL, LVTTL, LVDS or NIM"
      exit(1)
    
    # Write miscellaneous information to file
    io_config_info = (b_io_dir << io_dir_shift) | (b_io_oe << io_oe_shift) | (b_io_term << io_term_shift) | (b_io_special << io_special_shift ) | (b_io_channel << io_channel_shift)
    io_logic_res   = (b_io_logic_level << io_logic_level_shift)
    fp_hex.write(chr(io_config_info))
    fp_hex.write(chr(io_logic_res))
  
  # Add "end of table"
  fp_hex.write(io_const_end_of_table.ljust(32, '\0'));
  fp_hex.close;
    
  # Create BIN file (HEX2BIN)
  bin_filename = bin_filename.replace("xml", "bin")
  fp_bin = open(bin_filename, "wb+")
  byte_count = 0
  read_line = []
  
  # Convert hex/string to binary
  with open(hex_filename,'rb+') as fp_hex:
    while True:
      read_byte=fp_hex.read(1)
      if not read_byte: break
      read_byte = ''.join('{0:08b}'.format(ord(x), 'b') for x in read_byte)
      read_line.append(read_byte)
      # Add a new line after 16 bytes
      if byte_count == bin_bytes_per_line-1:
        fp_bin.write(''.join(read_line))
        fp_bin.write('\n')
        read_line = [] # Reset array
        byte_count = 0
      else:
        byte_count = byte_count + 1
        
  # Close all files
  fp_hex.close
  fp_bin.close
  
  # Create BIN file
  mif_filename = xml_filename.replace("xml", "mif") 
  fp_mif = open(mif_filename, "wb+")
  fp_mif.write("DEPTH = 256;\n")
  fp_mif.write("WIDTH = 32;\n")
  fp_mif.write("ADDRESS_RADIX = DEC;\n")
  fp_mif.write("DATA_RADIX = BIN;\n")
  fp_mif.write("CONTENT\n")
  fp_mif.write("BEGIN\n")
  line_count = 0
  byte_count = 0
  
  # Convert hex/string to mif
  with open(hex_filename,'rb+') as fp_hex:
    while True:
      read_byte=fp_hex.read(1)
      if not read_byte: break
      read_byte = ''.join('{0:08b}'.format(ord(x), 'b') for x in read_byte)
      read_line.append(read_byte)
      # Add a new line after 16 bytes
      if byte_count == 3:
        line_begin_print = str(line_count).zfill(3) 
        fp_mif.write(line_begin_print)
        fp_mif.write(" : ")
        fp_mif.write(''.join(read_line))
        fp_mif.write(";")
        fp_mif.write('\n')
        read_line = [] # Reset array
        byte_count = 0
        line_count = line_count + 1
      else:
        byte_count = byte_count + 1
  
  fp_mif.write("END;\n")
  
  # Close all files
  fp_hex.close
  fp_mif.close
  
  # Move mif file to the hdl directory
  file_to_remove = mif_target_dir + "/" + mif_filename
  if (os.path.isfile(file_to_remove)):
    os.remove(file_to_remove)
  shutil.move(mif_filename, mif_target_dir)
  
  # Done
  exit(0)

# Main
# --------------------------------------------------------------------------------
if __name__ == "__main__":
  main(sys.argv[1:])
