/* request.c: HTTP Request Functions */

#include "spidey.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>

int parse_request_method(struct request *r);
int parse_request_headers(struct request *r);

/**
 * Accept request from server socket.
 *
 * This function does the following:
 *
 *  1. Allocates a request struct initialized to 0.
 *  2. Initializes the headers list in the request struct.
 *  3. Accepts a client connection from the server socket.
 *  4. Looks up the client information and stores it in the request struct.
 *  5. Opens the client socket stream for the request struct.
 *  6. Returns the request struct.
 *
 * The returned request struct must be deallocated using free_request.
 **/
struct request *
accept_request(int sfd)
{
    struct request *r;
    struct sockaddr raddr;
    socklen_t rlen = sizeof(struct sockaddr);

    /* Allocate request struct (zeroed) */
    if((r = calloc(1, sizeof(struct request))) == NULL) {
        fprintf(stderr, "Memory allocation for request failed: %s\n", strerror(errno));
        exit(1);
    }

    /* Accept a client */
    int             r_fd;

    if ((r_fd = accept(sfd, &raddr, &rlen)) < 0) {
        fprintf(stderr, "accept failed: %s\n", strerror(errno));
        goto fail;
    }

    /* Lookup client information */
    int  flags = NI_NUMERICHOST | NI_NUMERICSERV;
    int  status;
    if ((status = getnameinfo(&raddr, rlen, r->host, sizeof(r->host), r->port, sizeof(r->port), flags)) != 0) {
        fprintf(stderr, "Client information lookup failed: %s\n", gai_strerror(status));
        goto fail;
    };

    /* Open socket stream */
    FILE *r_file = fdopen(r_fd, "w+");
    if (r_file == NULL) {
        fprintf(stderr, "fdopen failed: %s\n", strerror(errno));
        close(r_fd);
        goto fail;
    }

    r->file = r_file;
    r->fd = r_fd;

    log("Accepted request from %s:%s", r->host, r->port);
    return r;

fail:
    free_request(r);
    return NULL;
}

/**
 * Deallocate request struct.
 *
 * This function does the following:
 *
 *  1. Closes the request socket stream or file descriptor.
 *  2. Frees all allocated strings in request struct.
 *  3. Frees all of the headers (including any allocated fields).
 *  4. Frees request struct.
 **/
void
free_request(struct request *r)
{
    if (r == NULL) {
    	return;
    }

    /* Close socket or fd */
    if (r->file != NULL) {
        fclose(r->file);
    }
    else if (r->fd != 0) {
        close(r->fd);
    }

    /* Free allocated strings */
    free(r->method);
    free(r->uri);
    free(r->path);
    free(r->query);

    /* Free headers */
    if(r->headers != NULL) {
        struct header *next = r->headers->next;
        for (struct header *curr = r->headers; curr != NULL; curr = next) {
            next = curr->next;
            free(curr->name);
            free(curr->value);
            free(curr);
        }
    }

    /* Free request */
    free(r);
}

/**
 * Parse HTTP Request.
 *
 * This function first parses the request method, any query, and then the
 * headers, returning 0 on success, and -1 on error.
 **/
int
parse_request(struct request *r)
{
    /* Parse HTTP Request Method */
    if((parse_request_method(r)) == -1) {
        fprintf(stderr, "parse_request_method failed\n");
        return -1;
    }

    /* Parse HTTP Requet Headers*/
    if((parse_request_headers(r)) == -1) {
        fprintf(stderr, "parse_request_headers failed\n");
        return -1;
    }

    return 0;
}

/**
 * Parse HTTP Request Method and URI
 *
 * HTTP Requests come in the form
 *
 *  <METHOD> <URI>[QUERY] HTTP/<VERSION>
 *
 * Examples:
 *
 *  GET / HTTP/1.1
 *  GET /cgi.script?q=foo HTTP/1.0
 *
 * This function extracts the method, uri, and query (if it exists).
 **/
int
parse_request_method(struct request *r)
{
    /* Read line from socket */
    char buffer[BUFSIZ];
    if (fgets(buffer, BUFSIZ, r->file) == NULL) {
        fprintf(stderr, "fgets failed: %s\n", strerror(errno));
        goto fail;
    }

    /* Parse method and uri */
    char *method = strtok(buffer, WHITESPACE);
    char *uri = strtok(NULL, WHITESPACE);

    /* Parse query from uri */
    char *query;
    if (strcmp(uri, "/") == 0) {
        query = "";
    }
    else {
        if((query = strchr(uri, '?')) == NULL) {
            query = "";
        }
        else {
            query = strtok(uri, "?");
            query = strtok(NULL, "?");
        }
    }

    /* Record method, uri, and query in request struct */
    if((r->method = strdup(method)) == NULL) {
        fprintf(stderr, "String duplication for method failed: %s\n", strerror(errno));
        goto fail;
    }

    if((r->uri = strdup(uri)) == NULL) {
        fprintf(stderr, "String duplication for uri failed: %s\n", strerror(errno));
        goto fail;
    }

    if((r->query = strdup(query)) == NULL) {
        fprintf(stderr, "String duplication for query failed: %s\n", strerror(errno));
        goto fail;
    }

    debug("HTTP METHOD: %s", r->method);
    debug("HTTP URI:    %s", r->uri);
    debug("HTTP QUERY:  %s", r->query);

    return 0;

fail:
    return -1;
}

/**
 * Parse HTTP Request Headers
 *
 * HTTP Headers come in the form:
 *
 *  <NAME>: <VALUE>
 *
 * Example:
 *
 *  Host: localhost:8888
 *  User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:29.0) Gecko/20100101 Firefox/29.0
 *  Accept: text/html,application/xhtml+xml
 *  Accept-Language: en-US,en;q=0.5
 *  Accept-Encoding: gzip, deflate
 *  Connection: keep-alive
 *
 * This function parses the stream from the request socket using the following
 * pseudo-code:
 *
 *  while (buffer = read_from_socket() and buffer is not empty):
 *      name, value = buffer.split(':')
 *      header      = new Header(name, value)
 *      headers.append(header)
 **/
int
parse_request_headers(struct request *r)
{
    struct header *curr = NULL;
    char buffer[BUFSIZ];
    char *name;
    char *value;
    struct header *newHeader;
    struct header *tailHeader;
    bool firstHeader = true;

    /* Parse headers from socket */
    while((fgets(buffer, BUFSIZ, r->file) != NULL) && (strlen(buffer) != 2)) {
        // Split buffer to get name and value
        name = buffer;

        value = strtok(buffer, ":");
        value = strtok(NULL, "");
        value = skip_whitespace(value);

        if(value != NULL) {
            if(value[strlen(value) - 1] == '\n') {
                chomp(value);
            }
        }

        if((newHeader = calloc(1, sizeof(struct header))) == NULL) {
            fprintf(stderr, "Memory allocation for new header failed: %s\n", strerror(errno));
            goto fail;
        }

        if((newHeader->name = strdup(name)) == NULL) {
            fprintf(stderr, "String duplication for header name failed: %s\n", strerror(errno));
            goto fail;
        }

        if(value != NULL) {
            if((newHeader->value = strdup(value)) == NULL) {
                fprintf(stderr, "String duplication for header value failed: %s\n", strerror(errno));
                goto fail;
            }
        }
        else {
            newHeader->value = "";
        }

        if(firstHeader == true) {
            r->headers = newHeader;
            firstHeader = false;
        }
        else {
            for (curr = r->headers; curr != NULL; curr = curr->next) {
                    tailHeader = curr;
            }
            tailHeader->next = newHeader;
        }

    }

#ifndef NDEBUG
    for (struct header *header = r->headers; header != NULL; header = header->next) {
    	debug("HTTP HEADER %s = %s", header->name, header->value);
    }
#endif
    return 0;

fail:
    return -1;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
