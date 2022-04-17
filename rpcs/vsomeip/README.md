# How to use

## Unix

- Server

```shell
export COMMONAPI_CONFIG=/etc/commonapi-dcus
export VSOMEIP_CONFIGURATION=/etc/vsomeip-dcus-server.json
dcus_server_vsomeip &
```

- Client

```shell
export COMMONAPI_CONFIG=/etc/commonapi-dcus
export VSOMEIP_CONFIGURATION=/etc/vsomeip-dcus-client.json
dcus_client_simple &
```
