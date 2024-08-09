# cxsom

The basics of CXSOM is in the cxsom directory. Start here.

cxsom-builder is a set of tools using cxsom.

sked and skednet are libraries used by cxsom to schedule interactions.

## Installation

### Dependencies

You will need to have the fftconv package installed (see https://github.com/jeremyfix/fftconv).

```
sudo apt install libfftw3-dev
cd <somewhere you place source code>
git clone https://github.com/jeremyfix/fftconv.git
cd fftconv/fftconv/
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
sudo make install
cd ~

### Libraries providied by this repo.

Install in this order:
- sked
- skednet
- csxom 
- cxsom-builder

Each of these are a directory inside this git repository. Jump into each of them and follow the README instructions.

Visit the [cxsom web page](https://frezza.pages.centralesupelec.fr/cxsom-web/cxsom/index.html)

