all:
	cc -Wall -Wextra tls_client.c -I../../autogen/libressl/include/ -L../../autogen/libressl/lib/ -o tls_client -ltls
	cc -Wall -Wextra tls_server.c -I../../autogen/libressl/include/ -L../../autogen/libressl/lib/ -o tls_server -ltls

clean:
	rm -f tls_client tls_server
