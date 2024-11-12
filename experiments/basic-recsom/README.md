# basic rec-SOM experiment


Here, we set up a basic recurrent SOM, using cxsom-builder. We have
customized a makefile for that purpose, read it.

We recommend to do the 'basic-som' experiment before, since things
presented there are not provided here with the same amount of details.

## Setup the demo

First setup a root-dir directory for our variables.

```
mkdir root-dir
make cxsom-set-config ROOT_DIR=./root-dir HOSTNAME=localhost PORT=10000 SKEDNET_PORT=20000 NB_THREADS=4
```

## Describing the computation

The computation is described in the recsom.cpp file. Have a look at it.

We have to compile it, and use it to check the rules (from graph outputs).

```
make recsom
make recsom-figs
```

## Running

First, start a variable viewer, and then launch the example.

```
make cxsom-scan-vars
make cxsom-launch-processor 
make send-rules 
```
# Experimentation

Experimentation is based on `feed.py` for feeding the inputs, and
`view.py` for showing the weights. Read them both.

Try the following

```
make feed SEQ=ABCDEFEDCB
make show_weights
make feed SEQ=ABCDEFEDCB
make feed SEQ=ABCDEFEDCB
make feed SEQ=ABCDEFEDCB
make feed SEQ=AAACCCFFF
make feed SEQ=AAACCCFFF
make feed SEQ=AAACCCFFF
```




