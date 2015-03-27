# First Release #

In order to get the first release available quickly, Synecdoche will not diverge far from BOINC. Tasks for the first milestone are:
  * Replacing the BOINC branding, and correcting situations where the BOINC branding is hardcoded.
  * Fixing known bugs.
  * Incremental performance improvements for the GUI.
  * Opportunistic usability improvements. Usability will be addressed properly in the second release.

# Second Release #
  * Installation docs
  * Release note template (what to do, and where)
  * Complete installers/uninstallers for all platforms
    * Single package application (OS X only?)
    * Did we decide on WiX for Windows?

# Third Release #
  * Consistent error notification
  * Improved CLI
  * Flexible columns (allow user to change what columns to show)
    * More information allowed in the manager (project DCF comes to mind)

# Tasks not yet assigned to a milestone #

  * Multi-threaded communications.
  * New theme engine for simple view.
  * Multi-computer view.
  * Standard XML library.
  * Fix Unicode for user names, team names, etc.

  * Short term
    * New GUI RPC protocol
    * Improved message reporting (message hierarchy, templated messages)
  * One day
    * GPU support (CUDA, Stream, OpenCL)
    * Goal-based CPU scheduler

## Ongoing tasks ##
  * Code cleanup and documentation.
  * Unit testing (requires refactoring)