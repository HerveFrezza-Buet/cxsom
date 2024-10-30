# basic SOM experiment

Here, we set up a basic SOM, using cxsom-builder


## Setup the demo

First setup a root-dir directory for our variables.

```
mkdir root-dir
make cxsom-set-config ROOT_DIR=./root-dir HOSTNAME=localhost PORT=10000 SKEDNET_PORT=20000 NB_THREADS=4
```

You can get help by only typing

```
make
```

You can check the config

```
make cxsom-show-config
```


## Clearing all

If you need to restart everything, you have to kill an eventual
running processor and clear the content of the root-dir directory.

```
make cxsom-kill-processor
make cxsom-clear-rootdir
```

## Describing the computation

The computation is described in the som.cpp file. Have a look at it.

We have to compile it.

```
make som
```

The we can generate PDFs that describe the computation.

```
make som-figs
```

## Launching the simulation

Then we can launch a processor, and scan the root-dir. You may need to
check if a processor is already running first (if so, kill it, se
above "clearing all" section).


```
make cxsom-is-processor-running 
```

We launch a processor (since none is already running)

```
make cxsom-launch-processor
make cxsom-scan-vars
```

Now we can send the rules to the processor

```
make send-rules
```

## Running the simulation

In order to make the processiing go further, we have to feed it with
new inputs. This is what feed.py does. You can try any of the folling
(several times).

```
make feed-crown
make feed-square
make feed-triangle
```

You can also view the pasw evolution of weights. We have the last
10000 of them in timeline "wgt", and the last 10000 of them, but saved
every 500 steps, in timeline "save". The file visu.py makes the
vizualization. Choose the ones you want to display.

```
make view-last-weights
make view-weights-history
```



