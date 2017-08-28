/* forking.c: Forking HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <signal.h>
#include <string.h>

#include <unistd.h>

/**
 * Fork incoming HTTP requests to handle the concurrently.
 *
 * The parent should accept a request and then fork off and let the child
 * handle the request.
 **/
void
forking_server(int sfd)
{
    struct request *request;
    pid_t pid;

    signal(SIGCHLD, SIG_IGN);

    /* Accept and handle HTTP request */
    while (true) {
    	/* Accept request */
        request = accept_request(sfd);
        if(request == NULL) {
            continue;
        }
        else if (request->file == NULL) {
            continue;
        }

        pid = fork();
        if(pid < 0) {
            debug("fork failed %s", strerror(errno));
            goto finish;
        }

        /* Ignore children */


        /* Fork off child process to handle request */
        if (pid == 0) {         // Child
            handle_request(request);
            free_request(request);
            exit(EXIT_SUCCESS);
        } else {                // Parent
            free_request(request);
        }
    }

    /* Close server socket and exit*/
    finish:
    free_request(request);
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
