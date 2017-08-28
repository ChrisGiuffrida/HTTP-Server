/* utils.c: spidey utilities */

#include "spidey.h"

#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

/**
 * Determine mime-type from file extension
 *
 * This function first finds the file's extension and then scans the contents
 * of the MimeTypesPath file to determine which mimetype the file has.
 *
 * The MimeTypesPath file (typically /etc/mime.types) consists of rules in the
 * following format:
 *
 *  <MIMETYPE>      <EXT1> <EXT2> ...
 *
 * This function simply checks the file extension version each extension for
 * each mimetype and returns the mimetype on the first match.
 *
 * If no extension exists or no matching mimetype is found, then return
 * DefaultMimeType.
 *
 * This function returns an allocated string that must be free'd.
 **/
char *
determine_mimetype(const char *path)
{
    char *ext;
    char *mimetype;
    char *tokenFull;
    char *tokenPiece;
    char buffer[BUFSIZ];
    FILE *fs = NULL;

    /* Find file extension */
    ext = strrchr(path, '.');
    if(ext) {
        ext++;
    }
    else {
        goto fail;
    }

    /* Open MimeTypesPath file */
    if((fs = fopen(MimeTypesPath, "r")) == NULL) {
        fprintf(stderr, "fopen MimeTypesPath failed: %s\n", strerror(errno));
        goto fail;
    }

    /* Scan file for matching file extensions */
    while(fgets(buffer, BUFSIZ, fs) != NULL) {
        if((buffer[0] == '#') || (buffer[0] == '\n')) {
            continue;
        }

        if(buffer[strlen(buffer) - 1] == '\n') {
            chomp(buffer);
        }

        tokenFull = strtok(buffer, "\t");
        tokenFull = strtok(NULL, "\t");

        tokenPiece = strtok(tokenFull, " ");
        tokenPiece = strtok(NULL, " ");

        //printf("%s\t%s\t%s\n", buffer, tokenFull, ext);
        if(tokenFull == NULL) {
            continue;
        }

        if(streq(tokenFull, ext)) {
            mimetype = buffer;
            goto done;
        }

        while (tokenPiece != NULL) {
            if(streq(tokenPiece, ext)) {
                mimetype = buffer;
                goto done;
            }

            tokenPiece = strtok(NULL, " ");
        }
    }

fail:
    mimetype = DefaultMimeType;

done:
    if (fs) {
        fclose(fs);
    }
    return strdup(mimetype);
}

/**
 * Determine actual filesystem path based on RootPath and URI
 *
 * This function uses realpath(3) to generate the realpath of the
 * file requested in the URI.
 *
 * As a security check, if the real path does not begin with the RootPath, then
 * return NULL.
 *
 * Otherwise, return a newly allocated string containing the real path.  This
 * string must later be free'd.
 **/
char *
determine_request_path(const char *uri)
{
    char path[BUFSIZ];
    char real[BUFSIZ];

    if(sprintf(path, "%s/%s", RootPath, uri) < 0) {
        fprintf(stderr, "sprintf failed: \n");
        return NULL;
    }

    realpath(path, real);

    if(strncmp(real, RootPath, strlen(RootPath)) != 0) {
        return NULL;
    }

    return strdup(real);
}

/**
 * Determine request type from path
 *
 * Based on the file specified by path, determine what type of request
 * this is:
 *
 *  1. REQUEST_BROWSE: Path is a directory.
 *  2. REQUEST_CGI:    Path is an executable file.
 *  3. REQUEST_FILE:   Path is a readable file.
 *  4. REQUEST_BAD:    Everything else.
 **/
request_type
determine_request_type(const char *path)
{
    struct stat s;
    request_type type;

    if(stat(path, &s) < 0) {
        debug("Stat failed: %s", strerror(errno));
        return REQUEST_BAD;
    }

    type = REQUEST_BAD;

    if (access(path, R_OK) == 0) {
        type = REQUEST_FILE;
    }

    if (access(path, X_OK) == 0) {
        type = REQUEST_CGI;
    }

    if ((s.st_mode & S_IFMT) == S_IFDIR) {
        type = REQUEST_BROWSE;
    }

    return (type);
}

/**
 * Return static string corresponding to HTTP Status code
 *
 * http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
 **/
const char *
http_status_string(http_status status)
{
    const char *status_string;

    if(status == HTTP_STATUS_OK) {
        status_string = "200 OK";
    }
    else if(status == HTTP_STATUS_BAD_REQUEST) {
        status_string = "400 Bad Request";
    }
    else if(status == HTTP_STATUS_NOT_FOUND) {
        status_string = "404 Not Found";
    }
    else if(status == HTTP_STATUS_INTERNAL_SERVER_ERROR) {
        status_string = "500 Internal Server Error";
    }

    return status_string;
}

/**
 * Advance string pointer pass all nonwhitespace characters
 **/
char *
skip_nonwhitespace(char *s)
{
    while(!isspace(s[0])) {
        s++;
    }
    return s;
}

/**
 * Advance string pointer pass all whitespace characters
 **/
char *
skip_whitespace(char *s)
{
    if(s == NULL) {
        return s;
    }
    else {
        while(isspace(s[0])) {
            s++;
        }
    }
    return s;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
