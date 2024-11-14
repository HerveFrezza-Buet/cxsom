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

Then, we can view the weight evolution

```
make view-weights
```

### The relax mode

Here, the idea is to show each relaxation step. Relaxation is
internal. It consist of several updates **during a time step** until
the inter-dependent updated values get stable. So relaxation cannot be
decomposed. The architecture object in C++ builder can produce
**specific** rules, which are not the default ones, in order to build
up the same map interaction, but where the relaxation process is
decomposed through successive timesteps.

As we need new rules, let us clear the existing ones at processor side
(useless if you have just restarted a new processor instance). The new
variables used for these new rules are gathered into timelines prefixed by
zrlx-<timpestep> (see the makefile macro), in order to avoid confusion with the
existing timelines.



```
make cxsom-clear-processor 
```

First, let us display the rules...

```
make relax-figs
```

... and send them, for detailing the relaxation that occurrs at timestep 100. This will rely on the previously saved weights, and here the ones at timestep 100.

```
make send-relax-rules TIMESTEP=100
```

We can start relaxation by feeding the variable which are note computed by relaxation. We use the weights at timestep 100, the initial X/BMU and Y/BMU both initialized at .5 (for example), and X Y inputs initialized with values corresponding to U=.5 in the banana shape.

```
make feed-relax-inputs TIMESTEP=100 U=.5 SHAPE=banana XBMU=.5 YBMU=.5
```

The relaxation story has been played for timestep 100, let us view it.

```
make view-relaxation TIMESTEP=100
```

You can try any other timestep (from 0 tà 2499). When you are done, you can clean everything.

```
make cxsom-clear-processor
make clear-relaxation
```