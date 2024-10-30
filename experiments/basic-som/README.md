# basic SOM experiment

Here, we set up a basic SOM, using cxsom-builder


## Setup the demo

First setup a root-dir directory for our variables.

```
mkdir root-dir
make cxsom-set-config ROOT_DIR=./root-dir HOSTNAME=localhost PORT=10000  SKEDNET_PORT=20000 NB_THREADS=4
```

You can get help by only typing

```
make
```