/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <squareball.h>

#include "httpd.h"


static void
print_help(const char *default_host, const char *default_port)
{
    printf(
        "usage:\n"
        "    blogc-runserver [-h] [-v] [-t HOST] [-p PORT] [-m THREADS] DOCROOT\n"
        "                    - A simple HTTP server to test blogc websites.\n"
        "\n"
        "positional arguments:\n"
        "    DOCROOT       document root directory\n"
        "\n"
        "optional arguments:\n"
        "    -h            show this help message and exit\n"
        "    -v            show version and exit\n"
        "    -t HOST       set server listen address (default: %s)\n"
        "    -p PORT       set server listen port (default: %s)\n"
        "    -m THREADS    set maximum number of threads to spawn (default: 20)\n",
        default_host, default_port);
}


static void
print_usage(void)
{
    printf("usage: blogc-runserver [-h] [-v] [-t HOST] [-p PORT] [-m THREADS] DOCROOT\n");
}


void sigint_handler(int sig) {
    printf("\n");
    exit(0);
}


int
main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, sigint_handler);

    int rv = 0;
    char *host = NULL;
    char *port = NULL;
    char *docroot = NULL;
    size_t max_threads = 20;
    char *ptr;
    char *endptr;

    char *tmp_host = getenv("BLOGC_RUNSERVER_DEFAULT_HOST");
    char *default_host = sb_strdup(tmp_host != NULL ? tmp_host : "127.0.0.1");
    char *tmp_port = getenv("BLOGC_RUNSERVER_DEFAULT_PORT");
    char *default_port = sb_strdup(tmp_port != NULL ? tmp_port : "8080");

    size_t args = 0;

    for (size_t i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'h':
                    print_help(default_host, default_port);
                    goto cleanup;
                case 'v':
                    printf("%s\n", PACKAGE_STRING);
                    goto cleanup;
                case 't':
                    if (argv[i][2] != '\0')
                        host = sb_strdup(argv[i] + 2);
                    else
                        host = sb_strdup(argv[++i]);
                    break;
                case 'p':
                    if (argv[i][2] != '\0')
                        port = sb_strdup(argv[i] + 2);
                    else
                        port = sb_strdup(argv[++i]);
                    break;
                case 'm':
                    if (argv[i][2] != '\0')
                        ptr = argv[i] + 2;
                    else
                        ptr = argv[++i];
                    max_threads = strtoul(ptr, &endptr, 10);
                    if (*ptr != '\0' && *endptr != '\0')
                        fprintf(stderr, "blogc-runserver: warning: invalid value "
                            "for -m argument: %s. using %zu instead\n", ptr, max_threads);
                    break;
                default:
                    print_usage();
                    fprintf(stderr, "blogc-runserver: error: invalid "
                        "argument: -%c\n", argv[i][1]);
                    rv = 3;
                    goto cleanup;
            }
        }
        else {
            if (args > 0) {
                print_usage();
                fprintf(stderr, "blogc-runserver: error: only one positional "
                    "argument allowed\n");
                rv = 3;
                goto cleanup;
            }
            args++;
            docroot = sb_strdup(argv[i]);
        }
    }

    if (docroot == NULL) {
        print_usage();
        fprintf(stderr, "blogc-runserver: error: document root directory "
            "required\n");
        rv = 3;
        goto cleanup;
    }

    if (max_threads <= 0 || max_threads > 1000) {
        print_usage();
        fprintf(stderr, "blogc-runserver: error: invalid value for -m. "
            "Must be integer > 0 and <= 1000\n");
        rv = 3;
        goto cleanup;
    }

    rv = br_httpd_run(
        host != NULL ? host : default_host,
        port != NULL ? port : default_port,
        docroot, max_threads);

cleanup:
    free(default_host);
    free(default_port);
    free(host);
    free(port);
    free(docroot);

    return rv;
}
