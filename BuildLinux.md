# Building the client on Linux #

You will need:
  * A reasonably modern C++ compiler
  * libcurl 7.17.1+ (with HTTPS support, for some projects)
  * OpenSSL 0.9.8 or later
  * zlib
  * wxWidgets 2.6 (preferably 2.8) for the GUI

To compile, run the usual commands:
  * `./configure --prefix=PREFIX`
  * `make`
  * `make install`

Replace `PREFIX` with the directory where you want to install Synecdoche.
The client will use `PREFIX`/var/synecdoche as data directory by default.

If you get the code from SVN,
or if you want to modify the build system,
you will also need:
  * autoconf 2.58+
  * automake 1.8+
  * libtool 1.4+

and run autoreconf -i to create the configure script.