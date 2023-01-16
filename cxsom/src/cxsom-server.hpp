#pragma once


#include <cxsomData.hpp>
#include <cxsomJobs.hpp>
#include <cxsomJobRule.hpp>
#include <cxsomOperation.hpp>
#include <cxsomProtocolServer.hpp>
#include <cxsomSymbols.hpp>
#include <cxsomTimeStep.hpp>
#include <cxsomUpdate.hpp>
#include <cxsomVariable.hpp>

/**
 * @example example-005-001-foo-operations.hpp
 * @example example-005-001-foo-processor.cpp
 * @example example-005-001-foo-help.cpp
 */


/**
 * @mainpage
 *
 * @section Overview
 *
 * In the example section, you will find test-* files and example-*
 * files. Ignore the test-* files, they are here for cxsom developping
 * purpose. So only consider the example-* files.
 *
 * As a cxsom user, your job can be either to write viewers, or to
 * write simulation. Writing viewers means accessing the data file
 * produced by the simulation and retrieve data in them... usually for
 * visualization purpose.Reading those files is the only tools cxsom
 * provides for you. Writing a simulation means defining data, and the
 * way one data is computed from another, on so on. This is the core
 * usage of cxsom. Both aspects are discussed here.
 *
 * @section viewers Writing viewers
 *
 * Once a simulation is done, variable are stored in one history file
 * per variable, in some root directory. Such history of variable
 * values is a .var file. The structure of a .var is described in the cxsom.pdf document (see spec section).
 *
 * In C++, you can retrieve the content of such file thanks to cxsom. To do so, you have to
 * @code
#include <cxsom-viewer.hpp>
 * @endcode
 * 
 * and compile with
 * 
 <pre>
g++ -o my-viewer my-viewer.cpp $(pkg-config --cflags --libs cxsom-viewer)
 </pre>
 * Examples example-001-* are examples for writing viewers.
 *
 @section rules Definition of updating rules

 The simulation consists of computing values, that are finally stored into files. A value, as usual, has a name and a type. The type is one of the following string:
 - Scalar
 - Pos1D
 - Pos2D
 - Array=N (N is an integer)
 - Map1D<X>=Y (X in {Scalar, Pos1D, Pos2D, Array=N}, Y is an integer).
 - Map2D<X>=Y.

 The value is referenced by a triplet (string, string, unsigned int). It represents (timeline, varname, time). Let us detail this.

 - All values ("T", *, *) belong to the timeline "T".
 - All values ("T", "X", *) represent the values of the variable "X" in the timeline "T". All values of a variable have the same type.
 - All values ("T", *, n) represent the "timestep" number n of the timeline "T".

 An updating rule consists of applying a function for updating a value (T, X, n), from other values (T', X', n'). The (T', X', n') values are the "arguments" of the update. The arguments such as T=T' and n=n' are called "in" arguments, while the others are "out" arguments.

An value (T, X, n) is "ready" once its value has been definitively computed, otherwise it is "busy". An update computation is triggered when all its "out" arguments are ready. It is updated while the "in" arguments are not stabilized. In other words, the values updatable inside a timestep are evaluated together successively until they all are stable. This is called a timestep "relaxation". Some updating rules handle a "deadline" parameter, that forces a value to be considered as stable after a certain amount of updates, in order to avoid infinite relaxations.

An update of a specific value is :

@code
{
  timeline t("T");

  kwd::at("X", n) << fx::some_operation(.... argument values ....)  | kwd::use("some param", x), .... ;
}
@endcode

or
@code
  kwd::at(kwd::var("T", "X"), n) << fx::some_operation(.... argument values ....)  | kwd::use("some param", x), .... ;
@endcode


You can also define an update that is valuable for any n... by not providing n. The operator <= instead of << enables to define an update that is used only for the first update of the value.

@code
{
  timeline t("T");

  "X" << fx::some_operation(.... argument values ....)  | kwd::use("some param", x), .... ;
}
@endcode

@section Defining your own operators

See the corresponding examples. Inheritance from cxsom classes enable the customization of the operations handled by cxsom.

@section running Running a simulation

You have to launch cxsom-processor, and then launch rule clients. This fills value files. Then, independantly, viewers can be launched in order to read these files as they are produced and visualize the value produced by the processor.

Rule clients can be launged in "graph" mode. In this case, they produce a .dot file that shows what is computed. Otherwise, they can be launched in "send" mode to send updates to the processor.

 

 */
