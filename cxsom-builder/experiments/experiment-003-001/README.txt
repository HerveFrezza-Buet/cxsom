Follow the following instruction to run the tutorial. The symbol '~>' is the prompt, do not type it.

## Description

This experiments builds up 2 1D maps, each receiving a scalar input, and connected one with the others. The two scalar inputs are dependent, they lie on a circle.

## Setup the demo

First setup a root-dir directory for our variables.

~> mkdir root-dir
~> make cxsom-set-config ROOT_DIR=./root-dir HOSTNAME=localhost PORT=10000 NB_THREADS=4

Then we can launch a processor, and scan the root-dir.

~> make cxsom-launch-processor
~> make cxsom-scan-vars

## Compute the data

Now, let us start a simulation. See cxsom-builder example 003-001. To do so, we have first to send the computing rules to the processor.

~> make send-main-rules

We can generate the dot figures corresponding to those rules

~> make main-figs

OK, the computation is stalled since it needs to be fed with inputs. Let us use the feed-main.py script to do so. It is invoked by our makefile.

~> make feed-main-inputs

We can view the weight evolution thanks to view-weights.py

~> make view-weights

## See a relaxation

Let us exhibit the relaxation process. It consists in using a different set of rules. These rules can be viewed.

~> make relax-figs

Let us send them to the processor, for timestep 50000

~> make send-relax-rules TIMESTEP=50000

Few rlx-0050000-* timelines are now avialable for running a relaxation, you have to feed the X,Y input, and eventually the BMU. The X,Y inputs are coordinates of a point in a circle, so the angle theta (in degree) is required for them.

~> make feed-relax-inputs TIMESTEP=50000 THETA=45 XBMU=.5 YBMU=.5

Now, relaxation for timestep step 50000 has been expanded in timelines zrlx-0050000-*, let us visualize it.

~> make view-relaxation TIMESTEP=50000







