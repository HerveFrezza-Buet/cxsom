

Here, we set up a basic recurrent SOM, using cxsom-builder. We have
customized a makefile for that purpose, read it.

We recommend to do the 'basic-som' experiment before, since things
presented there are not provided here with the same amount of details.

We provide sequences of scalar inputs. The scalar input values are
coded as A=0, B=.2, C=.4, D=.6, E=.8 and F=1.  So we can feed with,
for example ABCDEFABCDEFABCDE..., cycling on the ABCDEF
pattern. Scalars will also be represented as grayscale values, black
for 0, white for 1.

A basic recurrent som is a SOM fed with a scalar input and the BMU
position in the map at previous timestep. Once organized, a position
in the map reflects the rank in the sequence, even if the scalar value
us ambiguous. For example, cycling on AAAAAFFFFF will visit 10 BMUs,
while only 2 scalar values (A=0 and F=1) are presented.

This is show by the demos.

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
## Experimentation

Experimentation is based on `feed.py` for feeding the inputs, `view.py` and
`history.py` for showing the weights. Read them both.

Try the following

```
make feed SEQ=ABCDEFEDCB
make show_weights-sequence
make feed SEQ=ABCDEFEDCB
make feed SEQ=ABCDEFEDCB
make feed SEQ=ABCDEFEDCB
make feed SEQ=AAACCCFFF
make feed SEQ=AAACCCFFF
make feed SEQ=AAACCCFFF
make feed SEQ=AAACCCFFF
make feed SEQ=AAAAAF
make feed SEQ=AAAAAF
make feed SEQ=AAAAAF
make feed SEQ=AAAAAF
make show_weights-history
```




