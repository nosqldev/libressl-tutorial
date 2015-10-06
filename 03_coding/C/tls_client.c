/* © Copyright 2015. All Rights Reserved.
 *
 * +----------------------------------------------------------------------+
 * | tls client                                                           |
 * +----------------------------------------------------------------------+
 * | Author: nosqldev@gmail.com                                           |
 * +----------------------------------------------------------------------+
 * | Created: 2015-09-29 16:06                                            |
 * +----------------------------------------------------------------------+
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "tls.h"

#define PORT "9999"

int
main(void)
{
    struct tls *tls;
    struct tls_config *tls_config;
    char buf[2048] = {0};
    char hello[] = "hello";

    if (tls_init() != 0)
    {
        printf("tls_init() failed\n");
        return -1;
    }

    if ((tls = tls_client()) == NULL)
    {
        printf("tls_client() failed\n");
        return -1;
    }

    if ((tls_config = tls_config_new()) == NULL)
    {
        printf("tls_config_new() failed\n");
        return -1;
    }

    tls_config_insecure_noverifycert(tls_config);
    tls_config_insecure_noverifyname(tls_config);

    if (tls_configure(tls, tls_config) != 0)
    {
        printf("tls_configure() failed\n");
        return -1;
    }

    if (tls_connect(tls, "127.0.0.1", PORT) != 0)
    {
        printf("error: %s\n", tls_error(tls));
        return -1;
    }

    if (tls_write(tls, hello, strlen(hello)) != (ssize_t)strlen(hello))
    {
        printf("tls_write() failed: %s\n", tls_error(tls));
        return -1;
    }

    if (tls_read(tls, buf, 16) < 0)
    {
        printf("tls_read() failed: %s\n", tls_error(tls));
        return -1;
    }

    puts(buf);

    assert(tls_close(tls) == 0);
    tls_free(tls);
    tls_config_free(tls_config);


    return 0;
}

/* vim: set expandtab tabstop=4 shiftwidth=4 foldmethod=marker: */
