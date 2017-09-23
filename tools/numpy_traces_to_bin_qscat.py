# This Python script is compatible with Python 2.7.x or 3.x (tested with Python2.7 & Python 3.6)
# This script is based on https://wiki.newae.com/CHES2016_CTF => Converting Traces to Binary Format (used by Daredevil attack)

import numpy as np
import struct
import argparse
import os
import sys

def getFileNameWithoutExtension(path):
  return path.split('\\').pop().split('/').pop().rsplit('.', 1)[0]

parser = argparse.ArgumentParser(description='Convert numpy traces/textin to binary for qscat(daredevil).')
parser.add_argument('-t', '--traces', help='[required param] traces.npy file')
parser.add_argument('-i', '--textin', help='[required param] textin.npy file')
parser.add_argument('-c', '--config', action='store_true', help='[optional param] output daredevil config info')
args = parser.parse_args()

traces_file_size = 0
traces_file_size = 0

if(args.traces):
	traces_file = args.traces
	traces_file_size = os.path.getsize(traces_file)
	if not traces_file_size:
		sys.exit("Error to open file "+traces_file)

if(args.textin):
	textin_file = args.textin
	textin_file_size = os.path.getsize(textin_file)
	if not textin_file_size:
		sys.exit("Error to open file "+textin_file)

if((traces_file_size > 0) and (textin_file_size > 0)):

	print("Convert Trace file in progress...")
	traces = np.load(traces_file)
	file_ext = "_x%dx%dx4.bin"%(traces.shape[0], traces.shape[1])
	tracefile_out = getFileNameWithoutExtension(traces_file)+file_ext
	f = open(tracefile_out, 'wb')
	for t in traces.flatten():
		f.write(struct.pack('f', t))
	f.close()
	print("Convert to file "+tracefile_out+ " done")

	print("Convert TextIn file in progress...")
	textin = np.load(textin_file)
	file_ext = "_x%dx%dx4.bin"%(textin.shape[0], textin.shape[1])
	textinfile_out = getFileNameWithoutExtension(textin_file)+file_ext
	f = open(textinfile_out, 'wb')
	for t in textin.flatten():
		f.write(struct.pack('B', t))
	f.close()
	print("Convert to file "+textinfile_out+ " done")

	if(args.config):
		print("")
		print("Add following to CONFIG:\n")
		print("[Traces]")
		print("files=1")
		print("trace_type=f")
		print("transpose=true")
		print("index=0")
		print("nsamples=%d"%traces.shape[1])
		print("trace=%s %d %d"%(tracefile_out, traces.shape[0], traces.shape[1]))
		print("")
		print("[Guesses]")
		print("files=1")
		print("guess_type=u")
		print("transpose=true")
		print("guess=%s %d %d"%(textinfile_out, textin.shape[0], textin.shape[1]))

else:
	print("traces.npy and textin.npy are mandatory use -h option for help")

