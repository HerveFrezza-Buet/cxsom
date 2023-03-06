Follow the following instruction to run the tutorial. The symbol '~>'
is the prompt, do not type it.

The cxsom-builder/examples/example-003-002-experiment.cpp shows the
architecture definition. The makefile used here is also an example, as
the *.py files.

## Description

This experiments builds up 3 1D maps W, H and RGB. They are fed with
pixels randomly tossed from an image. A pixel is (w, h, rgb), so the
scalar w feeds W, the scalar h feeds H and the Array=3 rgb feeds
RGB.

The three maps W, H and RGB are reciprocally connected. The idea is to
learn the relation betwee,w, h and rgb, and then ask the map to deduce
rgb from w and h.


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

Then, see the "Restart section'

## Compute the data

First, let us build-up the input generator (the null walltime warning is ok).

~> make input-setup

Then, we can ask for inputs until some timestep.

~> make feed WALLTIME=500

You can visualize the current inputs

~> make show-inputs


## Training

~> make send-train-rules

You can view the algorithm (architecture and rules)

~> make show-train-archi

## Restart training from previous execution

This tells how to restart processor and continue the work done until now. We suppose that no processor is running.

~> make cxsom-launch-processor
~> make send-input-rules
~> make send-train-rules

Then you can extend the walltime for inputs to get new ones. Here, we extend to 1000.

~> make feed WALLTIME=1000






