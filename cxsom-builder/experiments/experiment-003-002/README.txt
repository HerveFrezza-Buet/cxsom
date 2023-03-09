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



