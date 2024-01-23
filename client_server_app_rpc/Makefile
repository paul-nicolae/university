GCC = g++
CFLAGS = -Wall -w -lnsl -ltirpc -g -I/usr/include/tirpc

.PHONY: all clean

all: server client

generate: rpc_oauth.x
	rpcgen -C rpc_oauth.x

server: rpc_oauth_server.cpp rpc_oauth_svc.c rpc_oauth_xdr.c
	$(GCC) -o rpc_oauth_server rpc_oauth_server.cpp rpc_oauth_svc.c rpc_oauth_xdr.c $(CFLAGS)

client: rpc_oauth_client.cpp rpc_oauth_clnt.c rpc_oauth_xdr.c
	$(GCC) -o rpc_oauth_client rpc_oauth_client.cpp rpc_oauth_clnt.c rpc_oauth_xdr.c $(CFLAGS)

clean:
	rm -f rpc_oauth_server rpc_oauth_client