# How to use

## Unix

- Master

```shell
export COMMONAPI_CONFIG=/etc/commonapi-dcus
export VSOMEIP_CONFIGURATION=/etc/vsomeip-dcus-master.json
dcus_master_vsomeip &
```

- Worker

```shell
export COMMONAPI_CONFIG=/etc/commonapi-dcus
export VSOMEIP_CONFIGURATION=/etc/vsomeip-dcus-worker.json
dcus_worker_simple &
```
