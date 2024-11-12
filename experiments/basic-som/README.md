# basic SOM experiment

Here, we set up a basic SOM, using cxsom-builder. We have customized a
makefile for that purpose, read it.


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
new inputs. This is what feed.py does. You can try any of the filling
(several times). You see the effect on the variable scanning window.

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

## Restarting from previous status

This is easy... just restart. We first kill the processor here.

```
make cxsom-kill-processor 
```

and restart.

```
make cxsom-launch-processor
make send-rules
```

## Debugging

When no more updating rules can be triggered, the computation gets
stalled. If you do not understand why (you may have missed some
rules), it can be convenient to debug. To do so, we have to start a
"monitored" processor, that will log rules activations for you. Let us
kill the eventally running processor, and provide new inputs in the
"xi" variable.

```
make cxsom-kill-processor
make feed-crown
```

Nota: feeding ends with an error... the scripts pings the running
processor, that is not running currently. But the xi variable has been
fed.

Let us launch a verbose monitor:

```
make cxsom-launch-monitored-processor 
make send-rules
```

The processor has run computation as soon as we have sent the
rules. But now, it waits for new inputs. If you need to investigate
the current scheduling of rules (and understand while the processing
is stalled), you can use the generated `monitoring.data` file. It is a
text file, but cxsom provides you with tools for exploring that file.

First you can use the `pycxsom.monitor` module for interactive
debugging inside a python console. Try this:

```
python3
```

and then, in the python shell.

```
from pycxsom.monitor import Monitor
m = Monitor()
m.help()
```

Quit the python interpreter when you are done.

Another way to use the generated `monitoring.data` file is to generate a report into a `monitoring.md` file.

```
make cxsom-monitoring-report 
```

The `monitoring.md` can be viewed with formatting tools base on markdown.







