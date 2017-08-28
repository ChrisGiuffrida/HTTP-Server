/* handler.c: HTTP Request Handlers */

#include "spidey.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

#include <dirent.h>
#include <unistd.h>

/* Internal Declarations */
http_status handle_browse_request(struct request *request);
http_status handle_file_request(struct request *request);
http_status handle_cgi_request(struct request *request);
http_status handle_error(struct request *request, http_status status);

/**
 * Handle HTTP Request
 *
 * This parses a request, determines the request path, determines the request
 * type, and then dispatches to the appropriate handler type.
 *
 * On error, handle_error should be used with an appropriate HTTP status code.
 **/
http_status
handle_request(struct request *r)
{
    http_status result;
    char *path;
    request_type type;
    int parse_error;

    /* Parse request */
    if((parse_error = parse_request(r)) != 0) {
        result = handle_error(r, HTTP_STATUS_BAD_REQUEST);
        log("HTTP REQUEST STATUS: %s", http_status_string(result));
        return result;
    }

    /* Determine request path */
    path = determine_request_path(r->uri);
    r->path = path;
    debug("HTTP REQUEST PATH: %s", r->path);

    /* Dispatch to appropriate request handler type */
    type = determine_request_type(path);
    switch (type) {
        case 0:
            result = handle_browse_request(r);
            break;
        case 1:
            result = handle_file_request(r);
            break;
        case 2:
            result = handle_cgi_request(r);
            break;
        case 3:
            result = handle_error(r, 2);
            break;
    }

    log("HTTP REQUEST STATUS: %s", http_status_string(result));
    return result;
}

/**
 * Handle browse request
 *
 * This lists the contents of a directory in HTML.
 *
 * If the path cannot be opened or scanned as a directory, then handle error
 * with HTTP_STATUS_NOT_FOUND.
 **/
http_status
handle_browse_request(struct request *r)
{
    struct dirent **entries;
    int n;
    const char *status_string = http_status_string(0);

    /* Open a directory for reading or scanning */
    n = scandir(r->path, &entries, NULL, alphasort);
    if (n < 0) {
        fprintf(stderr, "scandir: %s\n", strerror(errno));
        return 3;
    }

    /* Write HTTP Header with OK Status and text/html Content-Type */
    fprintf(r->file, "HTTP/1.0 %s\n", status_string);
    fprintf(r->file, "Content-Type: text/html\n");
    fprintf(r->file, "\r\n");

    /* For each entry in directory, emit HTML list item */

    /* Bootstrap Delivery*/
    fprintf(r->file, "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\">\n");
    fprintf(r->file, "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js\"></script>\n");
    fprintf(r->file, "<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\"></script>\n");


    fprintf(r->file, "<ul>\n");
    for(int i = 0; i < n; i++) {
        if(streq(entries[i]->d_name, ".")) {
            free(entries[i]);
            continue;
        }
        if(streq(r->uri, "/")) {
            if(streq(entries[i]->d_name, "..")) {
                fprintf(r->file, "<li><a href=\"/\" class=\"btn btn-info\" role=\"button\">%s</a></li>\n", entries[i]->d_name);
            }
            else {
                fprintf(r->file, "<li><a href=\"/%s\" class=\"btn btn-info\" role=\"button\">%s</a></li>\n", entries[i]->d_name, entries[i]->d_name);
            }
        }
        else {
            fprintf(r->file, "<li><a href=\"%s/%s\" class=\"btn btn-info\" role=\"button\">%s</a></li>\n", r->uri, entries[i]->d_name, entries[i]->d_name);
            //fprintf(r->file, "<li><a href=\"%s/%s\"                                        > %s</a></li>\n", r->uri, entries[i]->d_name, entries[i]->d_name);
        }
        free(entries[i]);
    }
    fprintf(r->file, "</ul>\n");

    /* Flush socket, return OK */
    free(entries);
    return HTTP_STATUS_OK;
}

/**
 * Handle file request
 *
 * This opens and streams the contents of the specified file to the socket.
 *
 * If the path cannot be opened for reading, then handle error with
 * HTTP_STATUS_NOT_FOUND.
 **/
http_status
handle_file_request(struct request *r)
{
    FILE *fs;
    char buffer[BUFSIZ];
    char *mimetype = NULL;
    size_t nread;
    const char *status_string = http_status_string(0);

    /* Open file for reading */
    if((fs = fopen(r->path, "r+")) == NULL) {
        fprintf(stderr, "opening file failed: %s\n", strerror(errno));
        return 3;
    };

    /* Determine mimetype */
    mimetype = determine_mimetype(r->path);

    /* Write HTTP Headers with OK status and determined Content-Type */
    fprintf(r->file, "HTTP/1.0 %s\n", status_string);
    fprintf(r->file, "Content-Type: %s\n", mimetype);
    fprintf(r->file, "\r\n");

    /* Read from file and write to socket in chunks */
    while((nread = fread(buffer, sizeof(char), BUFSIZ, fs)) > 0) {
        fwrite(buffer, sizeof(char), nread, r->file);
    }

    /* Close file, flush socket, deallocate mimetype, return OK */
    fflush(r->file);
    fclose(fs);
    free(mimetype);
    return HTTP_STATUS_OK;
}

/**
 * Handle file request
 *
 * This popens and streams the results of the specified executables to the
 * socket.
 *
 *
 * If the path cannot be popened, then handle error with
 * HTTP_STATUS_INTERNAL_SERVER_ERROR.
 **/
http_status
handle_cgi_request(struct request *r)
{
    FILE *pfs;
    char buffer[BUFSIZ];
    struct header *header;

    /* Export CGI environment variables from request:
    * http://en.wikipedia.org/wiki/Common_Gateway_Interface */
    setenv("DOCUMENT_ROOT", RootPath, 1);
    setenv("REMOTE_ADDR", r->host, 1);
    setenv("REQUEST_URI", r->uri, 1);
    setenv("SCRIPT_FILENAME", r->path, 1);
    setenv("SERVER_PORT", Port, 1);
    setenv("QUERY_STRING", r->query, 1);
    setenv("REMOTE_PORT", r->port, 1);
    setenv("REQUEST_METHOD", r->method, 1);


    /* Export CGI environment variables from request headers */
    for (header = r->headers; header != NULL; header = header->next) {
        if(streq("Connection", header->name)) {
            setenv("HTTP_CONNECTION", header->value, 1);
        }
        if(streq("User-Agent", header->name)) {
            setenv("HTTP_USER_AGENT", header->value, 1);
        }
        if(streq("Host", header->name)) {
            setenv("HTTP_HOST", header->value, 1);
        }
        if(streq("Accept", header->name)) {
            setenv("HTTP_ACCEPT", header->value, 1);
        }
        if(streq("Accept-Language", header->name)) {
            setenv("HTTP_ACCEPT_LANGUAGE", header->value, 1);
        }
        if(streq("Accept-Encoding", header->name)) {
            setenv("HTTP_ACCEPT_ENCODING", header->value, 1);
        }
    }

    /* POpen CGI Script */
    pfs = popen(r->path, "r");

    /* Copy data from popen to socket */
    while(fgets(buffer, BUFSIZ, pfs)) {
        fputs(buffer, r->file);
    }

    /* Close popen, flush socket, return OK */
    pclose(pfs);
    fflush(r->file);
    return HTTP_STATUS_OK;
}

/**
 * Handle displaying error page
 *
 * This writes an HTTP status error code and then generates an HTML message to
 * notify the user of the error.
 **/
http_status
handle_error(struct request *r, http_status status)
{
    const char *status_string = http_status_string(status);

    /* Write HTTP Header */
    fprintf(r->file, "HTTP/1.0 %s\n", status_string);
    fprintf(r->file, "Content-Type: text/html\n");
    fprintf(r->file, "\r\n");

    /* Bootstrap Delivery*/
    fprintf(r->file, "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\">\n");
    fprintf(r->file, "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js\"></script>\n");
    fprintf(r->file, "<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\"></script>\n");

    /* Write HTML Description of Error*/
    fprintf(r->file, "<div class=\"container\"><div class=\"jumbotron\">\n");
    fprintf(r->file, "<h1>%s</h1>", status_string);
    fprintf(r->file, "<h2>Stuff's all borked.  I blame nargles.</h2>\n");
    fprintf(r->file, "</div></div>\n");

    /* Return specified status */
    return status;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
