# cxsom

The basics of CXSOM is in the cxsom directory. Start here.

cxsom-builder is a set of tools using cxsom.

sked (and skednet) is a library used by cxsom to schedule interactions.

Visit the [cxsom web page](https://frezza.pages.centralesupelec.fr/cxsom-web/index.html)



## Install




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

### The python materials

First option: go into the `cxsom` directory you have git-cloned. Then type

```
cd pycxsom
pip install .
```

Second option: install directly from the remote repository.

```
pip install git+https://github.com/HerveFrezza-Buet/cxsom#subdirectory=pycxsom
```

This installs (with setup tools) the required python library, as well as some utilities (see the bin directory).