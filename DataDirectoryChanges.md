# Current functionality #

## Windows ##

On Windows, when the core client started up, it looked up a key in the registry
to know where the data directory was, and used `chdir` to change to it.

Why is this in the past tense? Because Didactylos commented it out in [r299](https://code.google.com/p/synecdoche/source/detail?r=299).

So the **real** current status is: the Windows client considers the current
directory as its data directory.

## Linux ##

On Linux, there has never been any special handling of the data directory.
Whatever is the current directory is used. There is a `--dir` switch (on all
platforms) to set the data directory, which simply makes the client `chdir`
there before doing anything.

Most distro packages change the data directory in their `init.d` scripts, either
with `--dir` or by changing the current directory first.

## Mac ##

The daemon itself doesn't do anything to locate the data directory;
current directory is data directory.
However, one usually doesn't start the daemon by itself,
not even automatically at startup.
The normal way is starting the manager,
or letting the system start the screensaver.
Both the manager and the screensaver
`chdir` to the data directory before running the daemon.
The path to this directory is _hardcoded in the manager/screensaver code_.

# Planned changes #

## Linux ##

### Problem ###

It sometimes happens that people (usually new users) run the daemon from some
random place, like their home directory.  This is easy to do if the daemon is in
the `$PATH`. Sometimes they really wanted to run the daemon, but they were in
the wrong directory, or didn't even know the current directory mattered.
Sometimes they wanted to attach a project or something like that, and run the
daemon instead of `boinccmd`/`syneccmd`. The latter mistake is frequent because
BOINC daemon has an `--attach-project` switch (which is really for debugging).
It has been removed in Synecdoche ([r439](https://code.google.com/p/synecdoche/source/detail?r=439)).

The consequences are the user will see "his projects are gone", and there will
be a complete file clutter in wherever it was run.

### Solution ###

Like in other Unix daemons, I think the data directory path should be hardcoded
in the binary, set by the build system based on the `--prefix` that was used in
`configure`. No matter what the current directory is when the daemon is run, it
would always know where to find the data. The main disadvantage to this is that
the user can't move his Synecdoche installation to a different place without
recompiling; however, this is a "problem" that _a lot_ of other Unix programs
have, especially servers.

We can also add an extra level of indirection: hardcode the path to a
configuration file (in `$PREFIX/etc`) that contains the path to the data
directory.

However, the `--dir` switch will still exist, to let the client run under a
different directory than it was originally configured to use.

As of 2009-06-24, this solution has been implemented in synecd ([r892](https://code.google.com/p/synecdoche/source/detail?r=892)),
but not yet in syneccmd or synecmgr.