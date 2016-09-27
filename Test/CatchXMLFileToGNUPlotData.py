#  input,
#  1. The path to the XML file created by catch
#  2. Name of output file

# this generates data
# run it inside the python script?
#Test.exe SmallAllocationComparison  -n StackPerf-release -r xml -d "yes" -o StackAllocatorPerfData.xml

# run test once
# -> transform xml data into gnuplot data
# -> append to output text file
# rerun until enough test runs have run

import xml.etree.ElementTree as etree
import sys
#len(sys.argv) != 2 or

if(len(sys.argv) != 3):
    print("Usage: arg1 [arg2]\n")
    print("arg1 is the path to the XML file created by catch.")
    print("arg2 is the path of the ouput file.")
    sys.exit(0)


infile = sys.argv[1]
if(len(sys.argv) == 3):
    outfile = sys.argv[2]
#else:
    #outfile =

tree = etree.parse(infile)
root = tree.getroot()
testgroup = root.getchildren()[0]
testcases = testgroup.findall("TestCase")

# figure out how many lines already exist in this file, so that the next line is
# appened with the next linenumber
try:
	with open(outfile) as f:
		testrun = len(f.readlines())
except FileNotFoundError:
		testrun = 1

with open(outfile, 'a') as f:
    for test in testcases:
        result = test.find("OverallResult")
        dur = result.attrib["durationInSeconds"]
        f.write(str(testrun) + " " + str(dur) + "\n")
        testrun += 1