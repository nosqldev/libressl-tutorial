/* © Copyright 2015 jingmi. All Rights Reserved.
 *
 * +----------------------------------------------------------------------+
 * | tls server                                                           |
 * +----------------------------------------------------------------------+
 * | Author: jingmi@gmail.com                                             |
 * +----------------------------------------------------------------------+
 * | Created: 2015-09-29 16:48                                            |
 * +----------------------------------------------------------------------+
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include "tls.h"

#define PORT 9999
#define KEY_FILE "../../autogen/key/cakey.pem"
#define CACERT_FILE "../../autogen/key/cacert.pem"

int
main(void)
{
    struct tls *tls;
    struct tls_config *tls_config;
    char buf[2048] = {0};
    char response[] = "world";
    int sockfd;
    int opt;
    struct sockaddr_in server_addr;
    char *key_file = KEY_FILE;
    char *cacert_file = CACERT_FILE;

    if (tls_init() != 0)
    {
        printf("tls_init() failed\n");
        return -1;
    }

    if ((tls = tls_server()) == NULL)
    {
        printf("tls_server() failed\n");
        return -1;
    }

    if ((tls_config = tls_config_new()) == NULL)
    {
        printf("tls_config_new() failed\n");
        return -1;
    }
    tls_config_verify_client_optional(tls_config);

    if (tls_config_set_cert_file(tls_config, cacert_file) == -1)
    {
        printf("unable to set TLS certificate file %s\n", cacert_file);
        return -1;
    }
    if (tls_config_set_ca_file(tls_config, cacert_file) == -1)
    {
        printf("unable to set root CA file %s\n", cacert_file);
        return -1;
    }
    if (tls_config_set_key_file(tls_config, key_file) == -1)
    {
        printf("unable to set TLS key file %s\n", key_file);
        return -1;
    }
    tls_config_set_protocols(tls_config, TLS_PROTOCOLS_ALL);
    tls_config_set_ciphers(tls_config, "legacy");
    tls_config_insecure_noverifyname(tls_config);
    tls_config_insecure_noverifycert(tls_config);

    if (tls_configure(tls, tls_config) != 0)
    {
        printf("tls_configure() failed: %s\n", tls_error(tls));
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        printf("socket() failed: %s\n", strerror(errno));
        return 0;
    }

    opt = 1;
    bzero(&server_addr, sizeof server_addr);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    assert(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) == 0);
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof server_addr) < 0)
    {
        perror("bind()");
        return -1;
    }

    if (listen(sockfd, 128) < 0)
    {
        perror("listen()");
        return -1;
    }

    for (; ;)
    {
        struct sockaddr_in client_addr;
        struct tls *tls_cctx;
        socklen_t len = sizeof client_addr;
        ssize_t read, written;

        int fd = accept(sockfd, (struct sockaddr *)&client_addr, &len);
        if (fd < 0)
        {
            perror("accpet()");
            return -1;
        }

        if (tls_accept_socket(tls, &tls_cctx, fd) < 0)
        {
            printf("tls_accept_socket() failed: %s\n", tls_error(tls));
            tls_cctx = NULL;
            return -1;
        }

        bzero(buf, sizeof buf);
        read = tls_read(tls_cctx, buf, 5);
        if (read < 0)
        {
            printf("tls_read failed: %s\n", tls_error(tls_cctx));
            return -1;
        }
        printf("read: %s\n", buf);
        written = tls_write(tls_cctx, response, strlen(response));
        if (written < 0)
        {
            printf("tls_write failed: %s\n", tls_error(tls_cctx));
            return -1;
        }
        tls_close(tls_cctx);
        tls_free(tls_cctx);
    }

    tls_close(tls);
    tls_free(tls);
    tls_config_free(tls_config);

    return 0;
}

/* vim: set expandtab tabstop=4 shiftwidth=4 foldmethod=marker: */
