Follow the following instruction to run the tutorial. The symbol '~>'
is the prompt, do not type it.

The cxsom-builder-example-003-001-experiment.cpp shows the
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

Then we can launch a processor, and scan the root-dir.

~> make cxsom-launch-processor
~> make cxsom-scan-vars

## Compute the data

Now, let us start a simulation. See cxsom-builder example 003-001. To
do so, we have first to send the computing rules to the processor.

~> make send-main-rules

We can generate the dot figures corresponding to those rules. View the
pdf files generated.

~> make main-figs

OK, the computation is stalled since it needs to be fed with
inputs. Let us use the feed-main.py script to do so. It is invoked by
our makefile.

~> make feed-main-inputs

We can view the weight evolution thanks to view-weights.py

~> make view-weights

## See a relaxation

Let us exhibit the relaxation process. It consists in using a
different set of rules. These rules can be viewed.

~> make relax-figs

Let us send them to the processor, for timestep 29999

~> make send-relax-rules TIMESTEP=29999

Few rlx-0029999-* timelines are now avialable for running a
relaxation, you have to feed the X,Y input, and eventually the
BMU. The X,Y inputs are coordinates of a point in a circle, so the
angle theta (in degree) is required for them.

~> make feed-relax-inputs TIMESTEP=29999 THETA=45 XBMU=.5 YBMU=.5

Now, relaxation for timestep step 29999 has been expanded in timelines
zrlx-0029999-*, let us visualize it.

~> make view-relaxation TIMESTEP=29999


## Analyse the map

Here, let us freeze the map status, and compute statistics about the
computation. It consists in using a different set of rules. These
rules can be viewed.

~> make frozen-figs

Let us build up a bunch of inputs for testings. The X,Y inputs are
coordinates of a point in a circle

~> make declare-frozen-inputs
~> make feed-frozen-inputs

Now, we can make a frozen test at some specific timestep.
Let us send the corresponding gules to the processor, for timestep 29999

~> make send-frozen-rules TIMESTEP=29999
~> make cxsom-ping-processor


Now, the statistics for timestep step 29999 have been computed in
timelines zfrz-0029999-*, let us visualize it.

~> make view-frozen TIMESTEP=29999
~> evince evince snap-029999.pdf

Ok, now, let us make a nice movie...

~> make movie








