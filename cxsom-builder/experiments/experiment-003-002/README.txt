Follow the following instruction to run the tutorial. The symbol '~>'
is the prompt, do not type it.

The cxsom-builder/examples/example-003-002-experiment.cpp shows the
architecture definition. The makefile used here is also an example, as
the *.py files.

### Description

This experiments builds up 3 1D maps W, H and RGB. They are fed with
pixels randomly tossed from an image. A pixel is (w, h, rgb), so the
scalar w feeds W, the scalar h feeds H and the Array=3 rgb feeds
RGB.

The three maps W, H and RGB are reciprocally connected. The idea is to
learn the relation between w, h and rgb, and then ask the map to deduce
rgb from w and h.

There are several modes during the experiments, corresponding to several
stages.
- inputs mode : This sets w, h, rgb samples, with (w,h) spanning [0,1]^2.
  These are used for experiments when we test if the architecture can
  rebuild the image.
- train mode : This trains the map from random (w, h) and assorted rgb.
  The weights of the maps are periodically saved.
- check mode : We test test here how the (h, w, rgb) are well represented
  in the map, by the external weights.
- predict mode : We ask the map to retrieve rgb from (w, h) and rebuild
  the picture.


### Setup the demo

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

### Clearing all

If you need to restart everything, you have to kill an eventual running processor and clear the content of the root-dir directory.

~> make cxsom-kill-processor
~> make cxsom-clear-rootdir

Then, see the "Restart" subsection in "Train mode" section


### Input Mode

First, we have to set up the image that corresponds to the function (w, h) -> rgb that we want to learn. The image is eye.ppm, it is a 100x100 ppm file.

~> make input-setup IMG_SIDE=100
~> make show-samples

### Train mode

Now we can train. Let us first see how training computation is done.

~> make show-train-rules

Let us emplace the training rules at server side (the null walltime warning is ok).

~> make make train-setup SAVE_PERIOD=100 IMAGE_SIDE=100

Indeed, computation is only done at step 0. This is due to the walltime value of the rule setting train-in/coord. In order to trigger computaion until timestep 50000, we just have to send a rule that modifies thes walltime.

~> make feed-train-inputs WALLTIME=50000

You can check if the training seems ok after this. If not, feed again with a higher walltime.

~> make show-weights-history
~> make show-rgb-mapping

Once you are ok, you can clear the training stuff. Once you do this, the training cannot be continued for further steps.

~> make cxsom-clear-processor
~> make clear-training

* Restarting

If you need to restart and continue the computation (up to 100000 for example)

~> make cxsom-launch-processor
~> make make train-setup SAVE_PERIOD=100 IMAGE_SIDE=100
~> make feed-train-inputs WALLTIME=100000

### Check mode

In this mode, we check wether the map is able to encode in its external weights a good representation of the (w, h, rgb) triplets. To do so, we feed the map with (w, h, rgb) triplets, with (w, h) spanning [0, 1]^2, and we get the (W/We-0, H/We-0, RGB/We-0) at corresponding bmus (W/BMU, H/BMU, RGB/BMU). The rules for this are the following:

~> make show-check-rules

We can, for example, build up the checking.

~> make cxsom-clear-processor
~> make clear-checks
~> make check WEIGHTS_AT=500 IMAGE_SIDE=100

Then, let us display the prediction

~> make show-checks

Clearing the checkings can be done as well.

~> make cxsom-clear-processor
~> make clear-checks


### Predict mode

Here, we ask the map to retrieve RGB from (w, h) values un [0,1]^2. In other words, this will draw the image. Let us first display the prediction rules.

~> make show-predict-rules

An then let us build-up a prediction for the saved weights at 500.

~> make cxsom-clear-processor
~> make clear-predict
~> make predict WEIGHTS_AT=500 IMAGE_SIDE=100

Then, let us display the prediction

~> make show-predictions

Clearing the prediction can be done as well.

~> make cxsom-clear-processor
~> make clear-predict












