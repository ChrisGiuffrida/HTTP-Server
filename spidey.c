/* spidey: Simple HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>

/* Global Variables */
char *Port	      = "9898";
char *MimeTypesPath   = "/etc/mime.types";
char *DefaultMimeType = "text/plain";
char *RootPath	      = "www";
mode  ConcurrencyMode = SINGLE;
char * PROGRAM_NAME = NULL;

/**
 * Display usage message.
 */
void
usage(const char *progname, int status)
{
    fprintf(stderr, "Usage: %s [hcmMpr]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -h            Display help message\n");
    fprintf(stderr, "    -c mode       Single or Forking mode\n");
    fprintf(stderr, "    -m path       Path to mimetypes file\n");
    fprintf(stderr, "    -M mimetype   Default mimetype\n");
    fprintf(stderr, "    -p port       Port to listen on\n");
    fprintf(stderr, "    -r path       Root directory\n");
    exit(status);
}

/**
 * Parses command line options and starts appropriate server
 **/
int
main(int argc, char *argv[])
{
    int sfd;
    PROGRAM_NAME = argv[0];
    int argind = 1;

    /* Parse command line options */
    while (argind < argc && strlen(argv[argind]) > 1 && argv[argind][0] == '-') {
        char *arg = argv[argind++];

        if (streq(arg, "-c")) {
            if(argind < argc) {
                char *string_mode = argv[argind++];
                if(strcasecmp(string_mode, "single") == 0) {
                    ConcurrencyMode = SINGLE;
                }
                else if (strcasecmp(string_mode, "forking") == 0) {
                    ConcurrencyMode = FORKING;
                }
                else {
                    ConcurrencyMode = UNKNOWN;
                }
            }
        }
        else if (streq(arg, "-m")) {
            if(argind < argc) {
                MimeTypesPath = argv[argind++];
            }
        }
        else if (streq(arg, "-M")) {
            if(argind < argc) {
                DefaultMimeType = argv[argind++];
            }
        }
        else if (streq(arg, "-p")) {
            if(argind < argc) {
                Port = argv[argind++];
            }
        }
        else if (streq(arg, "-r")) {
            if(argind < argc) {
                RootPath = argv[argind++];
            }
        }
        else if (streq(arg, "-h")) {
            usage(PROGRAM_NAME, 0);
        }
        else {
            usage(PROGRAM_NAME, 1);
        }
    }

    /* Listen to server socket */
    sfd = socket_listen(Port);
    if (sfd < 0) {
        fatal("Unable to listen on port: %s", Port);
        return EXIT_FAILURE;
    }

    /* Determine real RootPath */
    if((RootPath = realpath(RootPath, NULL)) == NULL) {
        fprintf(stderr, "spidey.c: realpath failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    log("Listening on port %s", Port);
    debug("RootPath        = %s", RootPath);
    debug("MimeTypesPath   = %s", MimeTypesPath);
    debug("DefaultMimeType = %s", DefaultMimeType);
    debug("ConcurrencyMode = %s", ConcurrencyMode == SINGLE ? "Single" : "Forking");

    /* Start either forking or single HTTP server */
    if(ConcurrencyMode == SINGLE) {
        single_server(sfd);
    }
    else {
        forking_server(sfd);
    }

    return EXIT_SUCCESS;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
