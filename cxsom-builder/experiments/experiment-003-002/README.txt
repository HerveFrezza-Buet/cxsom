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

If you need to restart everything, you have to kill an eventual
running processor and clear the content of the root-dir directory.

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

~> make send-train-rules SAVE_PERIOD=10

You can view the algorithm (architecture and rules)

~> make show-train-archi

You can extend inputs walltime (to 1000 here)

~> make feed WALLTIME=1000

You can view the saved weights evolution

~> make show-weights-history

You can also view the color mapping of the RGB map.

-> make show-rgb-mapping

When you are done with training, you can clean up unsaved stuff.

~> make clear-train

## Restart training from previous execution

This tells how to restart processor and continue the work done until
now. We suppose that no processor is running.

~> make cxsom-launch-processor
~> make send-input-rules
~> make send-train-rules

Then you can extend the walltime for inputs to get new ones. Here, we extend to 1000.

~> make feed WALLTIME=1000


## Testing the learning

Here, we set up the same architecture, but we do not learn, we use
some of the previously saved weights. Moreover, we let RGB values be
found from W and H. So the test architecture is slightly different
from the one we used for learning. Let us display it.

~> make show-test-archi

We have to build up (once, many teste can be done from the same input
sets) the inputs. Its consists in positions spanning the [0,1]x[0,1]
patch.

~> make prediction-inputs-setup

We can send (the processor have to be launched first) the testing
rules, here for the saved_weights at timestep 100 (i.e. train step
100*SAVE_PERIOD).

~> make send-test-rules WEIGHTS_AT=100
~> make show-prediction FRAME_ID=100

When you are done with this test

~> make clear-test










