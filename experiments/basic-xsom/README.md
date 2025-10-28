

Here, we set up two interconnected SOMs, using cxsom-builder. We have
customized a [makefile](makefile) for that purpose, read it.

We recommend to do the [basic-som](basic-som) experiment before, since things
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

First setup a root-dir and the virtual env directories (see. [basic-som](basic-som)).

```
mkdir root-dir
make cxsom-set-config ROOT_DIR=./root-dir VENV=../cxsom-venv/ HOSTNAME=localhost PORT=10000 SKEDNET_PORT=20000 NB_THREADS=4
make cxsom-show-venv-activation 
```

## Understand inputs

Inputs are 2D curves (i.e M(u) = (x(u), y(u))). You can choose one in
the following (the examples are given for the banana curve). Let us
display them before starting, using [show-input-shape.py](show-input-shape.py).

```
make show-shape SHAPE=circle
make show-shape SHAPE=banana
make show-shape SHAPE=racket
```

## Running

First, start a variable viewer, and then launch the example.

```
make cxsom-scan-vars
make cxsom-launch-processor 
```

## Experimentation

### The main mode

The rules are encoded in [xsom.cpp](xsom.cpp). First, let us display the rules...

```
make main-figs
```

... and send them.

```
make send-main-rules TRACE=2500
```
Here, the TRACE value is the history size. In this experiment, feeding with inputs beyond this history window is not expected.

Then we can feed the inputs (see [feed-main.py](feed-main.py)) (let us use a banana shape). You can try other shapes, available ones are 'circle', 'racket' and 'banana'.

```
make feed-main SHAPE=banana
```

Then, we can view the weight evolution (see [view-weights.py](view-weights.py)):

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

... and send them, for detailing the relaxation that occurrs at
timestep 100. This will rely on the previously saved weights, and here
the ones at timestep 100.

```
make send-relax-rules TIMESTEP=100
```

We can start relaxation by feeding the variable which are note
computed by relaxation. We use the weights at timestep 100, the
initial X/BMU and Y/BMU both initialized at .5 (for example), and X Y
inputs initialized with values corresponding to U=.5 in the banana
shape (see [feed-relax.py](feed-relax.py)).

```
make feed-relax-inputs TIMESTEP=100 U=.5 SHAPE=banana XBMU=.5 YBMU=.5
```

The relaxation story has been played for timestep 100, let us view it.

```
make view-relaxation TIMESTEP=100
```

You can try any other timestep (from 0 to 2499). When you are done,
you can clean everything.

```
make cxsom-clear-processor
make clear-relaxation
```

### The frozen mode

This mode is used to analyze the architecture at a given timestep. We
freeze the learning, initialize the weight of the desierd timestep,
and run the architecture with inputs. Only relaxation occurs in this
case, at each timestep. This can be usefull to compute statistical
reults (i.e. which BMUs are visited, etc...).

The architecture is able to provide rules for this "learning frozen"
use. We prefix the timelines by zfrz, as we did previously with zrlx
for relaxation (see [feed-frozen.py](feed-frozen.py)).

```
make cxsom-clear-processor
make frozen-figs
make declare-frozen-inputs
make feed-frozen-inputs SHAPE=banana
make view-frozen-inputs
```

Now, we can make a frozen test at some specific timestep.
Let us send the corresponding rules to the processor, for timestep 2499


```
make send-frozen-rules TIMESTEP=2499
make cxsom-ping-processor
```

Now, the statistics for timestep step 2499 have been computed in
timelines zfrz-00002499-*, let us visualize it (it may be weird, since we
fed with a circle while we have learnt from a banana). See [view-frozen-inputs.py](view-frozen-inputs.py).

```
make view-frozen TIMESTEP=2499
```

A `snap-*.pdf` file has been generated. 

And then 

```
make cxsom-clear-processor
make clear-frozen
```
If you do not intend to make a movie, you can also

```
make clear-frozen-inputs 
```



## Making a movie

You can close the variable scanning windows, since it reads the files periodically. Frozen input settings still need to be there (see previous section).

If you are restarting from scratch, do the following to be able to
generate a movie. Otherwise, skip these commands.

```
make cxsom-clear-rootdir
make cxsom-kill-processor
make cxsom-launch-processor 
make send-main-rules TRACE=2500
make feed-main SHAPE=racket
make view-weights
make declare-frozen-inputs
make feed-frozen-inputs SHAPE=racket

```


Let us use [make-frames.py](make-frames.py) to make a movie.
```
make frames EVERY=5 NEXT_FRAME=0
```

If the process get stalled, you can restart it from the next frame you
want. Let say that you need to restart from frame 123.

```
make frames EVERY=5 NEXT_FRAME=123
```

When you are done, you can make the movie and clear the frames.
```
make movie
rm frame-*.png
```