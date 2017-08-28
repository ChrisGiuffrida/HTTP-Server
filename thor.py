#!/usr/bin/env python2.7

import multiprocessing
import os
import requests
import sys
import time

# Globals

PROCESSES = 1
REQUESTS  = 1
VERBOSE   = False
URL       = None

# Functions

def usage(status=0):
    print '''Usage: {} [-p PROCESSES -r REQUESTS -v] URL
    -h              Display help message
    -v              Display verbose output

    -p  PROCESSES   Number of processes to utilize (1)
    -r  REQUESTS    Number of requests per process (1)
    '''.format(os.path.basename(sys.argv[0]))
    sys.exit(status)

def do_request(pid):
    total_time = 0;
    for request in range(REQUESTS):
        start = time.time()
        response = requests.get(URL)
        end = time.time()
        if VERBOSE:
            print response.content
        print "Processes: {}, Request: {}, Elapsed Time: {:.5f}".format(pid, request, end - start)
        total_time = total_time + (end - start)

    print "Processes: {}, AVERAGE   , Elapsed Time: {:.5f}".format(pid, total_time / REQUESTS)

    return (total_time / REQUESTS)




args = sys.argv[1:]

# Main execution
if __name__ == '__main__':
    # Parse command line arguments
    while len(args) and args[0].startswith('-') and len(args[0]) > 1:
        arg = args.pop(0)
        if arg == '-v':
            VERBOSE = True
        elif arg == '-p':
            PROCESSES = int(args.pop(0))
        elif arg == '-r':
            REQUESTS = int(args.pop(0))
        elif arg == '-h':
            usage(0)
        else:
            usage(1)

    if len(args) == 1:
        URL = args.pop(0)
    else:
        usage(1)

    # Create pool of workers and perform requests
    pool = multiprocessing.Pool(PROCESSES)
    average_times = pool.map(do_request, range(PROCESSES))

    average_time = sum(average_times) / PROCESSES

    print "TOTAL AVERAGE ELAPSED TIME: {:.5f}".format(average_time)




# vim: set sts=4 sw=4 ts=8 expandtab ft=python:
