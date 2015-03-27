# Installation instructions #

**You cannot run BOINC and Synecdoche at the same time.**
Be especially careful if BOINC is configured
to start automatically when the computer boots.
You will have to either uninstall BOINC first,
or keep it from auto-starting.

## Windows ##
There is no real installer yet.
Synecdoche is available as a zip archive
containing the needed programs.

To "install" it, extract the zip archive into its own folder.
Run "synecmgr.exe" to start Synecdoche.

Project files will be saved inside that same folder.
There is no separate "data directory".
Everything is in the same folder, like BOINC used to do before 6.x.

There is (currently?) no way to migrate data
from an existing BOINC installation.

## Mac OS X ##
Synecdoche requires Mac OS X 10.4 or later.
**Synecdoche does not currently work on 10.6 or later.**
It comes packaged as a standard Installer package.
Run the package, and it will install Synecdoche in the Applications folder for you.
Currently, the install location cannot be changed.
By default, it also installs the Synecdoche screensaver;
however, this can be deselected.

**Warning**: A certificate file to attach to projects that use SSL,
like World Community Grid, should have been included, but was
accidentally left out in 0.1.1. This is fixed in 0.1.2.

  * A temporary fix to the above is to copy the **ca-bundle.crt** file from a BOINC installation (/Library/Application Support/BOINC Data) or from Synecdoche release bundle for another platform, and place it in the Synecdoche data directory (/Library/Application Support/Synecdoche Data).

To start, open the Synecdoche application in the Applications folder.

To make it run on boot,
add Synecdoche.app to your Login Items in System Preferences.

There is currently no uninstaller.
Dragging the application to the Trash is not enough;
the project data and the screensaver would remain in the system.
This will be addressed in a future version.

For now, if you want to completely uninstall Synecdoche,
delete /Applications/Synecdoche,
/Library/Screen Savers/Synecdoche
and the directory "/Library/Application Support/Synecdoche Data".

## Linux/Unix ##
Making a portable Linux binary is hard,
so we only provide source code for Linux.
Packages for specific Linux distributions may be available in future versions.

See BuildLinux for instructions on how to build from source.

Unlike BOINC, the Synecdoche client on Linux will
always use the same directory to store its data,
regardless of the current working directory.
This eliminates common confusion when people run the client
from a different place than the previous time
and "lose their projects".

Once you run `make install`, you cannot move the directory you used as `--prefix`,
since it gets hardcoded into the `synecd` binary.
To change the data directory at runtime, use `synecd --dir path/to/dir`.