# cxsom

cxsom is a simulator for multi-soms.

## Installations 

### Dependencies

You will need to have the fftconv package installed (see https://github.com/jeremyfix/fftconv).

```
sudo apt install libfftw3-dev libasio-dev 
cd <somewhere you place source code>
git clone https://github.com/jeremyfix/fftconv.git
cd fftconv/fftconv/
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
sudo make install
cd ~
```

### The C++ materials

You have to install sked, skednet and cxsom. For each one (let us name it xxx)

Go into the `xxx` directory you have git-cloned. Then type

```
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j
sudo make install

```

You have compiled stuff (the c++ examples and some utilities) and installed the headers.

### The python materials

Go into the `cxsom` directory you have git-cloned. Then type

```
cd pycxsom
python3 setup.py install --user
```
This installs (with setup tools) the required python library, as well as some utilities (see the bin directory).

## Unpacking test (example 004-002)

Create an empty working directory somewhere on your disk, launch two
separate terminals, and cd in this directory in the two terminals.

In the first terminal, let us launch the simulator (e.g. listening on port 10000, using 4 threads, and storing data in the 'data' directory.)

```
cxsom-processor data 4 10000
```

Now, we only work on the second terminal. Type the following commands.

```
ls
```

You can see that an empty directory named data, where computation will be stored.

Before going further, we define an environment variable CXSOM_HOME, which is the path to git directory of cxsom on your disk (e.g. `/home/me/somewhere/cxsom`)
```
export CXSOM_HOME=<the-path...>
```

Now, we need to tell the simulator what to do, i.e. to provide it with variable definitions and computational rules. This is detailed in C++ code, that needs to be compiled. For the sake of illustration, copy the cxsom example 004-002 in the current directory, renaming it as `rules.cpp`... as if we had written it by ourselves just now.

```
cp $CXSOM_HOME/examples/example-004-002-cxsom-recsom-maps.cpp rules.cpp
```

We can use the cxsom C++ library to compile it.

```
g++ -o rules rules.cpp $(pkg-config --cflags --libs cxsom-rules) -std=c++17
```

We have now a command that you can try:

```
./rules
```
Without arguments, it gives you a help message. Note that the examples have already been compiled and installed, so the example 004-002 that we have copied is also accessible via:

```
cxsom-example-004-002-cxsom-recsom-maps
```

Ok, let us play with it. First, it can display graphically (install graphviz) the rules. For that purpose, we can ask the command to generate .dot files.

```
./rules graph my-rules
```

You can use the following to have full variable names.

```
./rules graph-full my-rules
```

We can use graphiz to generate pdfs from the two dot files generated. Have a look at the pdfs after running the following (inits.pdf is blank here, since we have no rule defining updates specifically used for the first computation.)

```
dot -Tpdf my-rules-inits.dot -o inits.pdf
dot -Tpdf my-rules-updates.dot -o update.pdf
```

or (better with graph-full)

```
sfdp -Tpdf -Goverlap=prism my-rules-inits.dot -o inits.pdf
sfdp -Tpdf -Goverlap=prism my-rules-updates.dot -o update.pdf
```


Ok, let us start the actual computation. This is done by sending the rules to the simulator.

```
./rules send localhost 10000
```

The directory 'data' contains actual data now, i.e. `.var` files.

```
find data
```

The cxsom package offers you a dedicated way to have a synthetical view of the data directory, rather than using `find`:

```
cxsom-all-instances data
```

A line like "in xi: Scalar: 10000 (2): [0]: <stuff>" reads as follows. "in xi" is the name of the timeline ("in" here) and the name of the variable ("xi" here). Then, the type of the variable is displayed ("Scalar" here). The values "10000 (2)" represent respectively the file size and the cache size. The file size is the size of the circular buffer storing the values of the variable at successive instants. It can be null if no values are expecte dto be stored for that variable. The cache size is the maximal number of timed values stored in the memory of the simulator. Bigger cache avoid file accesses, and a cach is mandatory to keep values alive during the simulation if the file size is null. Next information is a time interval ("[0]" here), that tells which time instants are represented in the file. Then, what is denoted here by "<stuff>" is the status of each value, i.e. ready or busy, represented by a check mark or an empty circle.

If you call again 

```
cxsom-all-instances data
```

You will notice no evolution. Indeed, the simulator applied all the rules until no more application is possible. The bloking here is due to the lack of inputs. You can use a new c++ rule-like program to define the value of some more variables at next time instants, but this is not convenient. The better way is to append data in some blocking `.var` file, and then "ping" the server to notify it that blocking information may not be blocking anymore. Pinging can be done like this (here it will have no effect since we have not added any data anywhere).

```
cxsom-ping localhost 10000
```

Appending new data (and usually end by a ping) can be done by python scripts. Here, we use the one provided as examples for example 004-002.

```
python3 $CXSOM_HOME/data-for-examples/cxsom-make-inputs-004-002.py data/ localhost 10000 ABCDEFEDCB 50
```

You can see that inputs ("in xi") have been added and that the simulator, since it received a ping, has restarted the rule application as far as possible.

```
cxsom-all-instances data
```

Now, go in the first terminal, and kill the simulator (CNTL-C). We have ended our simulation, but the status is still in the files. We can restart... from that point! To do so, relaunch the simulator in the first terminal (the command is the last one in your bash history...).

```
cxsom-processor data 4 10000
```

And, in the second terminal (we stay here from now), remind the simulator with the rules (do not forget this after a restart).

```
./rules send localhost 10000
```

We can check that nothing has changed, send supplementary data, and check that the simulator processes them.
```
cxsom-all-instances data
python3 $CXSOM_HOME/data-for-examples/cxsom-make-inputs-004-002.py data/ localhost 10000 ABCDEFEDCB 50
cxsom-all-instances data
```

Now, it is time to view the results. The best for it is to write a specific graphical interface... do not be afraid, with tk, pycxsom and matplotlib, this is very easy. Let us use existing example scripts for that, and more specially the one dedicated to our example 004-002.

```
python3 $CXSOM_HOME/viewer-for-examples/cxsom-view-004-002.py data &
```

Play with the interface, generate more data (you can change the sequence ABCDEFEDCB to AAAAF for example), refresh the viewers, etc...

## Documentation

There is a pdf documentation (in section 'spec') that contains rather technical aspects. The main documentation rather consists of examples. You can find examples in the `examples` subdirectory (they are included in the doxygen generated doc). Read them following the order. It tells how some C++ code defining the simulation can be written. You have also examples in subdirectory `pycxsom/examples` telling you how to use the python library.

The files that we use with the examples (in both `viewer-for-examples` and `data-for-examples`, as we did for example 004-002) can be read for the sake of understanding pycxsom as well.



