#! /usr/bin/env python2.7
import commands


port = raw_input("Enter the port number for the server (single):  ")


##Single - Small
# Directory Listings
output = commands.getoutput('./thor.py -r 10 -p 1 http://student00.cse.nd.edu:' + port + '/files/small.txt')
length = output.count('\n')
single_small = 1000 / float(output.splitlines()[length].split()[4])


##Single - Medium
# Directory Listings
output = commands.getoutput('./thor.py -r 10 -p 1 http://student00.cse.nd.edu:' + port + '/files/medium.txt')
length = output.count('\n')
single_medium = 1000000 / float(output.splitlines()[length].split()[4])

##Single - Large
# Directory Listings
output = commands.getoutput('./thor.py -r 10 -p 1 http://student00.cse.nd.edu:' + port + '/files/large.txt')
length = output.count('\n')
single_large = 1000000000 / float(output.splitlines()[length].split()[4])

while input != "y":
    input = raw_input("Restart the server in forking mode.  Press \"y\" when ready:  ")

port = raw_input("Enter the port number for the server (forking):  ")

##Forking - Small
# Directory Listings
output = commands.getoutput('./thor.py -r 10 -p 1 http://student00.cse.nd.edu:' + port + '/files/small.txt')
length = output.count('\n')
fork_small = 1000 / float(output.splitlines()[length].split()[4])

##Forking - Medium
# Directory Listings
output = commands.getoutput('./thor.py -r 10 -p 1 http://student00.cse.nd.edu:' + port + '/files/medium.txt')
length = output.count('\n')
fork_medium = 1000000 / float(output.splitlines()[length].split()[4])

##Forking - Large
# Directory Listings
output = commands.getoutput('./thor.py -r 10 -p 1 http://student00.cse.nd.edu:' + port + '/files/large.txt')
length = output.count('\n')
fork_large = 1000000000 / float(output.splitlines()[length].split()[4])

print 'THROUGHPUT: Requests = 10, Processes = 1  (BYTES/SECOND)'
print '|               |      Single      |      Forking      |'
print '|--------------:|------------------|-------------------|'
print '|{:15}|{:18.5f}|{:19.5f}|'.format('Small  (1 KB)', float(single_small), float(fork_small))
print '|{:15}|{:18.5f}|{:19.5f}|'.format('Medium (1 MB)', float(single_medium), float(fork_medium))
print '|{:15}|{:18.5f}|{:19.5f}|'.format('Large  (1 GB)', float(single_large), float(fork_large))
