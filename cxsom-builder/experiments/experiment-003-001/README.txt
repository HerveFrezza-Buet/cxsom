Follow the following instruction to run the tutorial. The symbol '~>'
is the prompt, do not type it.

The cxsom-builder/examples/example-003-001-experiment.cpp shows the
architecture definition. The makefile used here is also an example, as
the *.py files.

## Description

This experiments builds up 2 1D maps, each receiving a scalar input,
and connected one with the others. The two scalar inputs are
dependent, they lie on a circle.


## Setup the demo

First setup a root-dir directory for our variables.

~> mkdir root-dir
~> make cxsom-set-config ROOT_DIR=./root-dir HOSTNAME=localhost PORT=10000 NB_THREADS=4

You can get help by only typing

~> make

You can check the config

~> make cxsom-show-config

Then we can launch a processor, and scan the root-dir.

~> make cxsom-launch-processor
~> make cxsom-scan-vars

## Clearing all

If you need to restart everything, you have to kill an eventual running processor and clear the content of the root-dir directory.

~> make cxsom-kill-processor
~> make cxsom-clear-rootdir

## Compute the data

Now, let us start a simulation. See cxsom-builder example 003-001. To
do so, we have first to send the computing rules to the processor.

~> make send-main-rules DURATION=2500

We can generate the dot figures corresponding to those rules. View the
pdf files generated.

~> make main-figs

OK, the computation is stalled since it needs to be fed with
inputs. Let us use the feed-main.py script to do so. It is invoked by
our makefile.

~> make feed-main-inputs SHAPE=banana

The computation starts

We can view the weight evolution thanks to view-weights.py

~> make view-weights

Once computation is done, it could be a good idea to flush the computing rules that are handled by the processor.

~> make cxsom-clear-processor 

## See a relaxation

Let us exhibit the relaxation process. It consists in using a
different set of rules. These rules can be viewed.

~> make relax-figs

Let us send them to the processor, for timestep 100

~> make send-relax-rules TIMESTEP=100

Few rlx-02499-* timelines are now avialable for running a
relaxation, you have to feed the X,Y input, and eventually the
BMU. The X,Y inputs are dependent on U, so we provide some U.

~> make feed-relax-inputs TIMESTEP=100 U=.5 SHAPE=banana XBMU=.5 YBMU=.5

Now, relaxation for timestep step 100 has been expanded in timelines
zrlx-00000100-*, let us visualize it.

~> make view-relaxation TIMESTEP=100

You can try any other timestep (from 0 tà 2499). When you are done, you can clean everything.

~> make cxsom-clear-processor
~> make clean-relaxation


## Analyse the map

Here, let us freeze the map status, and compute statistics about the
computation. It consists in using a different set of rules. These
rules can be viewed.

~> make frozen-figs

Let us build up a bunch of inputs for testings. The X,Y inputs are
coordinates of a point in a circle (while we fed with a banana...)

~> make cxsom-clear-processor
~> make declare-frozen-inputs
~> make feed-frozen-inputs SHAPE=circle
~> make view-frozen-inputs

Now, we can make a frozen test at some specific timestep.
Let us send the corresponding rules to the processor, for timestep 2499

~> make send-frozen-rules TIMESTEP=2499
~> make cxsom-ping-processor


Now, the statistics for timestep step 2499 have been computed in
timelines zfrz-02499-*, let us visualize it (it may be weird, since we
fed with a circle while we have learnt from a banana).

~> make view-frozen TIMESTEP=2499
~> evince snap-00002499.pdf

You can restart and see another shape (the appropriate one here)
~> make cxsom-clear-processor
~> make clear-frozen-inputs
~> make clear-frozen
~> make declare-frozen-inputs
~> make feed-frozen-inputs SHAPE=banana
~> make view-frozen-inputs
~> make send-frozen-rules TIMESTEP=2499
~> make cxsom-ping-processor
~> make view-frozen TIMESTEP=2499
~> evince snap-00002499.pdf

Ok, now, let us make a nice movie, with 1 frame every 10
timesteps. This uses python3 threading. Be sure you have it enabled on
your system. If not "pip3 install thread6" should install it.

~> make cxsom-kill-processor
~> make clear-frozen

You can close the variable scanning windows, since it reads the files periodically.

~> make frames EVERY=5 NEXT_FRAME=0

If the process get stalled, you can restart it from the next frame you
want. Let say that you need to restart from frame 123.

~> make frames EVERY=5 NEXT_FRAME=123

When you are done, you can make the movie and clear the frames.
~> make movie
~> rm frame-*.png












