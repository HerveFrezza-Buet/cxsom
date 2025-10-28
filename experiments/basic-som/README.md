
Here, we set up a basic SOM, using cxsom-builder. We have customized a
[makefile](makefile) for that purpose, read it.


## Setup the demo

First setup a root-dir directory for our variables.

```
mkdir root-dir
```

Then, as we will use python tools, we need a virtual environment. You can use an existing one, or create a new one somewhere. Whatever the case, you have to declare the path to your environment as the `VENV` variable. Here, let us plan to have the virtual environment in `../cxsom-venv`... but once again, you can use an already existing one.

```
make cxsom-set-config ROOT_DIR=./root-dir VENV=../cxsom-venv HOSTNAME=localhost PORT=10000 SKEDNET_PORT=20000 NB_THREADS=4
```

You can get help by only typing

```
make
```

You can check the config

```
make cxsom-show-config
```

Now, if your environment is not created yet, you have to created it (be sure that your python installation has the venv modules).

```
make cxsom-make-venv 
```

In your environment, the `pycxsom` library and other stuff have to be installed. Let us do this.

```
make cxsom-install-venv
```


Now, you can "open" your environment and use it throughout the experiment. The command line to do so is recalled by

```
make cxsom-show-venv-activation
```

When you are done with experiments, only type

```
deactivate
```

to exit the virtual environment.



## Clearing all

If you need to restart everything, you have to kill an eventual
running processor and clear the content of the root-dir directory.

```
make cxsom-kill-processor
make cxsom-clear-rootdir
```

## Describing the computation

The computation is described in the [som.cpp](som.cpp) file. Have a look at it.

We have to compile it.

```
make som
```

Then we can generate PDFs that describe the computation.

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
```

It is also convinient to have a window scanning your (currently empty) root directory.

```
make cxsom-scan-vars
```

Now we can send the rules to the processor

```
make send-rules
```

## Running the simulation

In order to make the processiing go further, we have to feed it with
new inputs. This is what [feed.py](feed.py) does. You can try any of the filling
(several times). You see the effect on the variable scanning window.

**Nota**: Be sure to have your virtual envoronment opened, since we use python files involving `pycxsom`.

```
make feed-crown
make feed-square
make feed-triangle
```

You can also view the past evolution of weights. We have the last
10000 of them in timeline "wgt", and the last 10000 of them, but saved
every 500 steps, in timeline "save". The file [view.py](view.py) makes the
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

Let us launch a monitored processor:

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
python
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

The `monitoring.md` can be viewed with formatting tools based on markdown.

```
make cxsom-monitoring-to-pdf 
```






