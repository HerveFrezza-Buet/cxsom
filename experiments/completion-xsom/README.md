Here, we set up 3 connected SOMs. Each one is connected to the two others by a forward and a feedback connection. Let us expermient this. We have customized a
[makefile](makefile) for that purpose, read it.

## Description

This experiments builds up 3 1D maps W, H and RGB. They are fed with
pixels randomly tossed from an image. A pixel is $(w, h, \rho)$, so the
scalar $w$ feeds W, the scalar $h$ feeds H and the Array=3 $\rho$ feeds
RGB.

The three maps W, H and RGB are reciprocally connected. The idea is to
learn the relation between $w$, $h$ and $\rho$, and then ask the map to
deduce $\rho$ from $w$ and $h$. In a realistic image, colors are not
distributed randomly over the image, so there is a relation between
the position of the picel and its color, that can be learnt.

The demo is organized into several stages, detailes below.


## Set up the demo


First setup a root-dir directory for our variables.

```
mkdir root-dir
make cxsom-set-config ROOT_DIR=./root-dir HOSTNAME=localhost PORT=10000 SKEDNET_PORT=20000 NB_THREADS=4
```

Last we can launch a processor, and scan the root-dir.

```
make cxsom-launch-processor
make cxsom-scan-vars
```

The demo (with its different stages) is define in the [xsom.cpp](xsom.cpp) file.

## The calibration stage.

This relies on two python scripts, [set-calibration.py](set-calibration.py) and [show-calibration.py](show-calibration.py). This consists in exhibiting the tuning curves, as defined by the parameters set in the [xsom.cpp](xsom.cpp) code. This invokes the cxsom-processor, but this is not a real simulation.

```
make clear-calibration
make cxsom-clear-processor
make calibration-setup GRID_SIDE=100
make calibrate
make show-calibration 
```



## The input stage

First, we have to set up the image that corresponds to the function $(w, h) \leadsto \rho$ that we want to learn. The image is [eye.ppm](eye.ppm) (.ppm format is easy to read...), it is a **square-shaped** image a 100x100 ppm file here. The image side (100 here), will have to be provided many times. Keep this value in mind. This uses [build-eye-input.py](build-eye-input.py) and [build-img-coordst.py](build-img-coords.py) and [show-samples.py](show-samples.py).

```
make inputs-setup IMAGE_SIDE=100
make show-samples
```

This computes the $100\times 100$ images positions, and feeds the corresponding  $w$s and the $h$s, and the $\rho$s (rgb variable).

Training consists of choosing a random value in this pre-computed set, and feed the maps with it.



## The training stage


Now we can train. Let us first see how training computation is done.

```
make show-train-rules
```

Let us emplace the training rules at server side (the null walltime warning is ok). Check the variable scanning and wait for the termination of the computing.

```
make make train-setup SAVE_PERIOD=100 IMAGE_SIDE=100
```

Indeed, computation is only done at step 0. This is due to the walltime value of the rule setting train-in/coord. In order to trigger computation until timestep 30000, we just have to send a rule that modifies thes walltime.

```
make feed-train-inputs WALLTIME=30000
```

Once again, check the variable scanning and wait for the termination of the computing.

You can check if the training seems ok after this (thanks to [display.py](display.py), [show-weights-history.py](show-weights-history.py) and [show-rgb-mapping.py](show-rgb-mapping.py)). If not, feed again with a higher walltime.

```
make show-weights-history
make show-rgb-mapping
```

Once you are ok, you can clear the training stuff. Once you do this, the training cannot be continued for further steps. If you do not type 'make clear-training', you can continue the training further on (see Restarting subsection next).

```
make cxsom-clear-processor
make clear-training
```

### Restarting

If you need to restart and continue the computation (up to 100000 for example)

```
make cxsom-launch-processor
make make train-setup SAVE_PERIOD=100 IMAGE_SIDE=100
make feed-train-inputs WALLTIME=100000
```