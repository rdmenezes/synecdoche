# Introduction #

The release notes for Synecdoche document differences from BOINC,
and things you will want to watch out for when running Synecdoche.

## Changes in 0.1.2 ##

**Fixed a problem when handling command line arguments of project apps. It caused AQUA@Home and PrimeGrid AP26 workunits to crash when starting.** On Mac, World Community Grid should now work. A file required for SSL connections was missing from the package in 0.1.1.
**Improvements to the Linux build system (see the `NEWS` file for details).**

## Differences from BOINC ##

Synecdoche is based on BOINC 6.2.
This means there is no support for running GPU applications,
since it was added in BOINC 6.4.
We have no plans to implement GPU support in the near future.

Changes done to BOINC:

  * Performance and security updates.
  * Separated suspend/resume and allow new tasks / no more tasks buttons. This lets you use these operations on multiple projects or tasks at once.
  * In the attach project wizard, the list of projects is sorted by name.
  * When aborting multiple tasks, the confirmation dialog now has a "Yes to all" button, so you don't have to click "Yes" for every aborted task (also available in BOINC since 6.10.14).
  * Before starting a task, all its input files are verified. (NOTE: causes problems with POEM@Home)
  * There is no network communication to boinc.berkeley.edu at all. This means no version checks and no auto-updating project list.
  * On Unix, the client's data directory is configured at compile time, and sticks to it. When starting the client, it will always use the correct data directory, instead of using the current directory.
  * The scheduler and work fetch algorithms in BOINC are constantly being patched over and over to try (and fail) to make it work properly with GPUs. Synecdoche doesn't even try to support GPUs, so the scheduler works as well as BOINC 6.2 did.

## Platform notes ##

### Windows ###
There is no real installer yet.
Synecdoche is available as a zip archive
containing the needed programs.
For this reason,
there is no sandboxing or
service install.
It also doesn't configure itself to start on boot automatically.

Windows 95/98/ME are **not** supported.

### Mac OS X ###
Synecdoche requires Mac OS X 10.4 or 10.5.
**10.6 Snow Leopard is not currently supported,
will be fixed in 0.1.3.**
It will run on both PowerPC and Intel machines,
and will attempt to make use of 64-bit on supported Intel processors.
It comes packaged as a standard Installer package.

There is currently no uninstaller.
This will be addressed in a future version.

**Warning**: A certificate file to attach to projects that use secure HTTP,
like World Community Grid,
was accidentally left out from 0.1.1.
This is fixed in 0.1.2.

### Linux ###
Making a portable Linux binary is hard,
so we only provide source code for Linux at this time.
Distro-specific binary packages may be available in the future.

Unlike BOINC, the Synecdoche client will always use the same directory,
as configured at compile time,
to store its data,
instead of the current working directory.
This eliminates common confusion when people run the client
from a different place than the previous time
and "lose" their running tasks.

## Limitations ##

Synecdoche cannot be running at the same time as BOINC.
This is not a bug; it would be the same as running two instances of BOINC at the same time.

Some projects may not work correctly.
We try to remain compatible with BOINC,
but sometimes projects rely on more implementation details than they should.
A [project compatibility list](ProjectCompatibilityMatrix.md) is kept to document incompatibilities.
Currently the only project known to have problems is POEM@Home.

The manager is only available in English,
translations are currently disabled.
We plan to bring translations back,
but it needs some work,
especially in the build system
and in how we will work with volunteer translations.