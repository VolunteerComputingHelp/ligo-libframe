#!/usr/bin/env python
import frgetvect

# test general call
output = frgetvect.frgetvect("../data/test.gwf","Adc1",925484675,10.,1)

# test call with default values
output = frgetvect.frgetvect("../data/test.gwf","Adc1")

print "len(output[0]) = %d" % len(output[0])
print "len(output[1]) = %d" % len(output[1])
print "len(output[2]) = %d" % len(output[2])
print "output[3:] = " + str(output[3:])

print output[0][:5]  #should print the first 5 elements of the y-axis.
