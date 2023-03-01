/*

  Here, we consider 3 connected maps one handling W, the other H (the
  coordinates in an image), and the last the RGB value of the
  pixel. We train the map with (W, H, RGB) triplets, and then we let
  the architecture retrieve thr RGB value from the coordinates.

  This serves as a rule generator for the experiment 003-002 in the
  experimental section.
*/

#include <cxsom-builder.hpp>
#include <fstream>
#include <sstream>
