# basic xSOM experiment


Here, we set up two interconnected SOMs, using cxsom-builder. We have
customized a makefile for that purpose, read it.

We recommend to do the 'basic-som' experiment before, since things
presented there are not provided here with the same amount of details.

The idea is to randomly toss u in [0,1], and provide to inputs, (x(u),
y(u)) respectively to a X and Y map. The BMU of each map is got by the
others, so that the maps now about the coordinate it doesn't
"see". The maps need to relax in order to find the appropriate BMU.

What is obtained at the end is that the BMU in each map reflects u,
even if none of the map has been fed with u, and even if there are
many identical inputs for the same u in each map.

This is show by the demos.

## Setup the demo

First setup a root-dir directory for our variables.

```
mkdir root-dir
make cxsom-set-config ROOT_DIR=./root-dir HOSTNAME=localhost PORT=10000 SKEDNET_PORT=20000 NB_THREADS=4
```


## Running

First, start a variable viewer, and then launch the example.

```
make cxsom-scan-vars
make cxsom-launch-processor 
```

## Experimentation

### The main mode

First, let us display the rules...

```
make main-figs
```

... and send them.

```
make send-main-rules TRACE=2500
```

Then we can feed the inputs (let us use a banana shape).

```
make feed-main SHAPE=banana
```

