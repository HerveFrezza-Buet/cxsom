# Basic scheduler experiment

The scheduling ("sked" in cxsom) is a feature that enables to interact with a running simulation. Indeed, the computation is stored in .var files in the root directory, and the cxsom-processor reads and writes these files. The same stands for viewers, that read the files as well, or when new inputs are provided (when we 'feed' some variables), which is a write in some files.

There is a concurrency issue in file access, that may lead to errors (freezings). In order to safely perform concurrent r/w accesses, cxsom provides a scheduler.

It is much more than a mutex, since reading ad writings are not treated the same way. The idea is that writings are exclusive. When a writer writes, reading requests are put in a queue. When writing ends, only the pending readers are given access to the files, new incoming readers will have to wait "for next round'. This prevents very demanding readers from blocking the read-write alternation (see examples in the sked library).

We do not detail these synchronizing issues here, we just rely on them to perform interactive cxsom.


This is show by the demos.

## Setup the demo

First setup a root-dir directory for our variables.

```
mkdir root-dir
make cxsom-set-config ROOT_DIR=./root-dir HOSTNAME=localhost PORT=10000 SKEDNET_PORT=20000 NB_THREADS=4
```

## Describing the computation

```
make minmax-figs
```


## Launching the simulation

We need first a skednet xrsw synchronizer. The name 'xrsw' means "multiple readers, single writer".

```
make skednet-launch-xrsw
make skednet-is-xrsw-running
```

Then we have to run a processor that uses the wrsw locker.

```
make skednet-launch-processor
<make cxsom-is-processor-running
make cxsom-scan-vars
```

and then

```
make send-rules
```
