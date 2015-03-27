# Building the client on Windows #

You will need:

  * Visual C++ 2005
  * A Subversion client
  * wxWidgets 2.8 for the GUI
  * WiX 3.0 for the installer

## Get the code ##

  1. Checkout the branch you want to build.
  1. Checkout svn/synecdoche\_depends\_win\_vs2005/

## Environment ##

| **Variable**    | **Path** |
|:----------------|:---------|
| WXWIN         | The folder containing wxWidgets |
| SYNEC\_DEPENDS | The folder containing synecdoche\_depends\_win\_vs2005 |

Ensure that PATH includes the Subversion client, WiX, and the Platform SDK (usually the installers will take care of this).

## Building ##
First, build the debug and release configurations for wxWidgets.

Finally, build the Synecdoche solution.

## Other versions of Visual Studio ##
If you are building against anything other than Visual C++ Runtime 8.0.50727.762 then you will have to rebuild all the dependencies and change the merge module. The version in SVN will always reflect the official build environment.