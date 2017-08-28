#! /usr/bin/env python2.7

import commands

port = raw_input("Enter the port number for the server (single):  ")

##Single Mode - processe 1
# Directory Listings
print 'Benchmarking Directory Listings  (mode=single, requests=100, processes=1)...'
output = commands.getoutput('./thor.py -r 100 http://student00.cse.nd.edu:' + port)
length = output.count('\n')
time1_sin_1 = output.splitlines()[length].split()[4]

# Static Files
print 'Benchmarking Static Files        (mode=single, requests=100, processes=1)...'
output = commands.getoutput('./thor.py -r 100 http://student00.cse.nd.edu:' + port +'/html/index.html')
length = output.count('\n')
time2_sin_1 = output.splitlines()[length].split()[4]

# CGI Scripts
print 'Benchmarking CGI Scripts         (mode=single, requests=100, processes=1)...'
output = commands.getoutput('./thor.py -r 100 http://student00.cse.nd.edu:' + port +'/scripts/cowsay.sh?message=Wuz+Good+pbui&template=koala')
length = output.count('\n')
time3_sin_1 = output.splitlines()[length].split()[4]

##Single Mode - processes 2
# Directory Listings
print 'Benchmarking Directory Listings  (mode=single, requests=100, processes=2)...'
output = commands.getoutput('./thor.py -r 50 -p 2 http://student00.cse.nd.edu:' + port)
length = output.count('\n')
time1_sin_2 = output.splitlines()[length].split()[4]

# Static Files
print 'Benchmarking Static Files        (mode=single, requests=100, processes=2)...'
output = commands.getoutput('./thor.py -r 50 -p 2 http://student00.cse.nd.edu:' + port +'/html/index.html')
length = output.count('\n')
time2_sin_2 = output.splitlines()[length].split()[4]

# CGI Scripts
print 'Benchmarking CGI Scripts         (mode=single, requests=100, processes=2)...'
output = commands.getoutput('./thor.py -r 50 -p 2 http://student00.cse.nd.edu:' + port +'/scripts/cowsay.sh?message=Wuz+Good+pbui&template=koala')
length = output.count('\n')
time3_sin_2 = output.splitlines()[length].split()[4]

##Single Mode - processes 4
# Directory Listings
print 'Benchmarking Directory Listings  (mode=single, requests=100, processes=4)...'
output = commands.getoutput('./thor.py -r 25 -p 4 http://student00.cse.nd.edu:' + port)
length = output.count('\n')
time1_sin_4 = output.splitlines()[length].split()[4]

# Static Files
print 'Benchmarking Static Files        (mode=single, requests=100, processes=4)...'
output = commands.getoutput('./thor.py -r 25 -p 4 http://student00.cse.nd.edu:' + port +'/html/index.html')
length = output.count('\n')
time2_sin_4 = output.splitlines()[length].split()[4]

# CGI Scripts
print 'Benchmarking CGI Scripts         (mode=single, requests=100, processes=4)...'
output = commands.getoutput('./thor.py -r 25 -p 4 http://student00.cse.nd.edu:' + port +'/scripts/cowsay.sh?message=Wuz+Good+pbui&template=koala')
length = output.count('\n')
time3_sin_4 = output.splitlines()[length].split()[4]


while input != "y":
    input = raw_input("Restart the server in forking mode.  Press \"y\" when ready:  ")

port = raw_input("Enter the port number for the server (forking):  ")

##Forking Mode
# Directory Listings
print 'Benchmarking Directory Listings (mode=forking, requests=100, processes=1)...'
output = commands.getoutput('./thor.py -r 100 http://student00.cse.nd.edu:' + port)
length = output.count('\n')
time1_fork_1 = output.splitlines()[length].split()[4]

# Static Files
print 'Benchmarking Static Files       (mode=forking, requests=100, processes=1)...'
output = commands.getoutput('./thor.py -r 100 http://student00.cse.nd.edu:' + port +'/html/index.html')
length = output.count('\n')
time2_fork_1 = output.splitlines()[length].split()[4]

# CGI Scripts
print 'Benchmarking CGI Scripts        (mode=forking, requests=100, processes=1)...'
output = commands.getoutput('./thor.py -r 100 http://student00.cse.nd.edu:' + port +'/scripts/cowsay.sh?message=Wuz+Good+pbui&template=koala')
length = output.count('\n')
time3_fork_1 = output.splitlines()[length].split()[4]

##Forking Mode - processes 2
# Directory Listings
print 'Benchmarking Directory Listings (mode=forking, requests=100, processes=2)...'
output = commands.getoutput('./thor.py -r 50 -p 2 http://student00.cse.nd.edu:' + port)
length = output.count('\n')
time1_fork_2 = output.splitlines()[length].split()[4]

# Static Files
print 'Benchmarking Static Files       (mode=forking, requests=100, processes=2)...'
output = commands.getoutput('./thor.py -r 50 -p 2 http://student00.cse.nd.edu:' + port +'/html/index.html')
length = output.count('\n')
time2_fork_2 = output.splitlines()[length].split()[4]

# CGI Scripts
print 'Benchmarking CGI Scripts        (mode=forking, requests=100, processes=2)...'
output = commands.getoutput('./thor.py -r 50 -p 2 http://student00.cse.nd.edu:' + port +'/scripts/cowsay.sh?message=Wuz+Good+pbui&template=koala')
length = output.count('\n')
time3_fork_2 = output.splitlines()[length].split()[4]


##Forking Mode - processes 4
# Directory Listings
print 'Benchmarking Directory Listings (mode=forking, requests=100, processes=4)...'
output = commands.getoutput('./thor.py -r 25 -p 4 http://student00.cse.nd.edu:' + port)
length = output.count('\n')
time1_fork_4 = output.splitlines()[length].split()[4]

# Static Files
print 'Benchmarking Static Files       (mode=forking, requests=100, processes=4)...'
output = commands.getoutput('./thor.py -r 25 -p 4 http://student00.cse.nd.edu:' + port +'/html/index.html')
length = output.count('\n')
time2_fork_4 = output.splitlines()[length].split()[4]

# CGI Scripts
print 'Benchmarking CGI Scripts        (mode=forking, requests=100, processes=4)...'
output = commands.getoutput('./thor.py -r 25 -p 4 http://student00.cse.nd.edu:' + port +'/scripts/cowsay.sh?message=Wuz+Good+pbui&template=koala')
length = output.count('\n')
time3_fork_4 = output.splitlines()[length].split()[4]

print

print 'Generating Markdown Table...'

print

print 'LATENCY: Requests = 100'
print '| Mode(# Processes)  | Single(1) | Single(2) | Single(4) | Forking(1) | Forking(2) | Forking(4) |'
print '|-------------------:|-----------|-----------|-----------|------------|------------|------------|'
print '|{:20}|{:11.5f}|{:11.5f}|{:11.5f}|{:12.5f}|{:12.5f}|{:12.5f}|'.format('Directory Listings', float(time1_sin_1), float(time1_sin_2), float(time1_sin_4), float(time1_fork_1), float(time1_fork_2), float(time1_fork_4))
print '|{:20}|{:11.5f}|{:11.5f}|{:11.5f}|{:12.5f}|{:12.5f}|{:12.5f}|'.format('Static Files', float(time2_sin_1), float(time2_sin_2), float(time2_sin_4), float(time2_fork_1), float(time2_fork_2), float(time2_fork_4))
print '|{:20}|{:11.5f}|{:11.5f}|{:11.5f}|{:12.5f}|{:12.5f}|{:12.5f}|'.format('CGI Scripts', float(time3_sin_1), float(time3_sin_2), float(time3_sin_4), float(time3_fork_1), float(time3_fork_2), float(time3_fork_4))
