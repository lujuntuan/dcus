# How to use

## Unix

- Master

```shell
host_server &
name_server &
dcus_master_fdbus &
```

- Worker

```shell
name_server -u tcp://ip_of_host1:60000 &
dcus_worker_simple &
```
