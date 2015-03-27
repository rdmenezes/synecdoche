﻿#summary Tracking BOINC revisions

## Pending merge ##

| **Changesets** | **Additional comments** |
|:---------------|:------------------------|
|15767|Need to review this mess carefully. (Vista and probably Win7 stuff)|
|15827|Screensaver stuff hidden in there.|
|15831|Manifest files?|
|15840/15874/15877/15885/15967/16001/16252/16253/16372/16385|Screensaver mess|
|15929|Precompiled headers work for the manager, but do they work for the client/lib/etc (windows only)?|
|15945|Do we need the screensaver stuff? The other fix is already applied.|
|15955/15957/15958/15960/15961/15963/15965|Code signing stuff.|
|15959/15964/15966|MGR: Save and restore selected items by key values when refreshing|
|15987|MGR: Simple GUI fixes|
|16208|client: new config flags|
|16280|MGR: Add a new tab status to the Simple GUI for when the client has been instructed to delay starting applications via the 

<delay\_start>

 option in cc\_config.xml (From Kevin Reed)|
|16327|MGR: If simple\_gui\_only flag is set in cc\_config.xml, trigger call of CSimpleFrame::OnConnect()|
|16336/16337/16347|MGR: When removing a row from sorted lists, refresh all rows to ensure proper display update _Does anyone know which bug is fixed by this changeset? Do we need it?_|
|16355/16400|client: the preemptability order was wrong|
|16356|client: cleaned up CPU scheduler logic somewhat|
|16366/16383/16386/16391|client: revise round-robin simulation to take variable avg\_ncpus into account _I'm not sure if we need this or if this change works with our ncpu-fix._|
|16395/16396|client: fix bug that caused occasional assert in pop\_heap() for the preemptable\_task\_list.|
|16405/16413/16414/16417/16419/16421/16428/16429/16446/16448/ 16450/16462/16472/16473|MGR: Create an Exit dialog for the Manager. / Allow the manager to shutdown the CC even when it was installed as a daemon.|
|16406/16415/16420/16426/16427/16434|MGR: Provide a way to enable/disable launching from the BOINC Manager at startup from within the BOINC Manager itself.|
|16432/16433|client: make host CPID a function of: MAC addresses + hostname + IP addr + OS name|
|16479|API: free project prefs before parsing init file|
|16484|MGR: Erase and refresh entire Tasks panel when selecting a new tab to try to fix cosmetic bug reported by David on Windows _Which bug? Is it caused by the RPC thread?_|
|16488/18720/18730/18735|SVCCTRL: Introduce a new binary that is used to start and stop the BOINC service, if it is installed as a service. _Looks like this is only needed for service install on Vista. Do we support/need this?_|
|16539|manager: tweak strings in Options dialog|
|16545|client: in round-robin simulation, only increment CPU shortfall (per-project or overall) if there are no pending tasks.|
|16548/18172|DIAG: On Windows move the symbol store directory under the BOINC data directory structure. _NOTE: Stackwalker still uses boinc.berkeley.edu as server!_|
|16557/16558|MGR: Make the error message processing work for the get\_project\_config rpc, use the same scheme as the ACCOUNT\_OUT structure.|
|16565|client: Fix error from changeset 14415 which caused Mac client to call daemon() if it was passed -daemon argument, causing problems using launchd with KeepAlive option; fixes #782.|
|16582|MGR: fix exit dialog so that both the Mac and Linux clients don't display the 'close core client...' checkbox.|
|16616|client: parse checkpoint\_elapsed\_time from state file; if missing, use checkpoint CPU time.|
|16653|Mac SG: Remove ugly hack added 8/20/07 for slide show alignment problem on Mac.|
|16654|Mac SG: Fix vertical position of project names in tabs on Mac.|
|16670/16671|MGR: word-wrap CompletionBrandedMessage static text in Account manager Wizard Completion page for GridRepublic.|
|16707|MGR: Always reset messages if client restarted; always scroll messages to bottom if connection status changes; Select Computer: localhost relaunches client if needed.|
|16929|client: fix boinc\_copy (again); Mac Mgr: fix bugs showing Mgr when minimized to Dock;|
|16998|client: clear debts when reset project|
|17063/17065/17075|MGR: Add a list of account managers. It's included in all\_projects\_list.xml. As we don't use BOINC's xml file we should either put the account manager list into a separated file or fix the structure of the file. BOINC currently treats account managers as projects in this file, which is wrong.|
|17142|MAC MGR: Use symbolic file name GUI\_RPC\_PASSWD\_FILE for gui\_rpc\_auth.cfg.|
|17195|client: add SCHEDULER\_OP::rpc\_failed(); this gets called when the op fails, either at initialization or later on; it clears the project's sched\_rpc\_pending flag if needed. This fixes a bug that caused user-requested RPCs to retry every 10 seconds when the network is down.|
|17196|client: if master file doesn't have URLs, clear RPC request|
|17269|client: fix bug where master fetch failure doesn't clear request flag, leading to infinite retry without backoff (17195 needs to be applied before this one)|
|17300/17301/17375|client: adjust debts at least every minute. This fixes a bug that can cause debts to NEVER get updated. /  client: added "abort\_jobs\_on\_exit" feature|
|17309/17311/17312/17213|client: add more info to 

<work\_fetch\_debug>

 messages|
|17310|client: don't complain that master URLs differ if it's only in case _NOTE: **Only** the domain name is case insensitive - this changeset needs to be fixed before beeing applied._|
|17388|_A lot of different, partly unrelated fixes that need to be reviewed carefully one by one before applying any of these._|
|17389|client: if you put 

<host\_venue>

 in global\_prefs\_override.xml, it should select the venue from the network prefs. Now it does.|
|17391|API: file descriptor leak in obscure case. fixes #103|
|17396|client: make timeout values into #defines _Note: The defines should be turned into constants before applying this changeset._|
|17409|manager: fix roundoff error in Advanced Prefs; fixes #613 _Note: This is not a real fix but a simple workaround._|
|17410|MGR: Make CTRL-SHIFT-A the accelerator in the simple GUI that switches back to the advanced view. refs #147|
|17456/17467|manager: when filtering messages by project, show messages not tagged with a project (fixes #852)|
|17486/17487|client: change garbage-collect logic. old: reference-count files involved in a PERS\_FILE\_XFER new: if a PERS\_FILE\_XFER refers to an unreferenced file, delete it (and the associated FILE\_XFER and HTTP\_OP if present) May fix #366|
|17507|client: if using anonymous platform, ignore (and complain about) app versions in scheduler reply client: when reporting anonymous platform apps in sched request, don't include 

<file\_info>

s (not relevant to server)|
|17564/17565/17590/17591/17592|_Multiple fixes for the Manager._|
|17567|Mac MGR: Do RestoreState() before Show() so Disk tab doesn't flash briefly on manual launch.|
|17617|In linux client, Added code to detect capability to run 64-bit binaries from 32-bit BOINC client, and capability to run 32-bit binaries on a 64-bit linux system. _This one might need some cleanup before applying._|
|17637/19040|client: initial support for detecting the CPU L2 cache size on Windows.|
|17639| client: add --no\_priority\_change cmdline arg (and 

<no\_priority\_change>

 flag in cc\_config.xml). If set, run apps at same priority as client.|
|17711|client: (unix): if host name lookup fails, call res\_init(). This is an attempt to fix a problem on Linux where, if the client starts before a VPN is set up, it can never communicate|
|17765/17831/17838|client: for each app version, keep track of the largest WSS of tasks using it. In checking whether tasks fit in RAM, use this as an estimate for tasks that haven't started yet. This avoids a situation where the client starts a lot of tasks in sequence, only to find that each one doesn't fit in RAM.|
|17826/17846|Mac MGR: Add keyboard shortcuts command-shift-S, command-shift-A to switch views|
|17992/17998|client: simplify enforce\_schedule(), and maybe fix bugs. _NOTE: Review this carefully before applying!_|
|18042|client, Mac: don't do res\_init(). It causes a crash. / client (Unix): if client crashes while benchmark processes are going, make sure they detect this and exit. _NOTE: 17711 needs to be applied before this one._|
|18054|Mac client: fix parent died test in benchmark\_time\_to\_stop() _NOTE: 18042 needs to be applied before this one._|
|18135|MGR: Fix bug which prevented skin change from being saved if exiting Manager while in Simple View. _16405 and 17564 need to apply before this one (because these two probably introduce the bug fixed here)._|
|18291|MGR: Accept Unicode input for usernames and password in the manager now that the manager is Unicode enabled. Input will show up as UTF-8 strings in the CC and project.|
|18340|lib: FILE\_LOCK::lock() makes lockfile group-writable so both client and manager can create and write it under sandbox security|
|18427|client: don't require that file upload URLs contain "file\_upload\_handler".|
|18593/18601/18602/18603/18604/18662/18663/18753/18786/18790|client: restored code for project-wide backoff on file|
|18595|libraries: Moved crypt.o out of libboinc and into libboinc\_crypt.|
|18605/18606/18737|client: changed file upload logic|
|18772/18785/18798/18799/18819/18893/(18936?)/18941/18971/18972/18973/18974/18997/19090|client: Initial swipe at automatic proxy server configuration detection on Windows. Fixes #35 _This probably needs some cleanup first._|
|18800/18803/18805/18811/18812/18813|client: refactor the CC startup process, move majority of the windows code into sysmon\_win.cpp, .h|
|18845|client: in the final stage of CPU scheduling, give preference to multi-threaded jobs. Avoid running N-1 1-thread jobs and 1 N-thread job on N CPUs / client: change file transfer giveup time from 14 to 90 days|
|18915/18916|client: when writing XML, entity-encode the following fields / client: XML\_PARSER::parse\_str() now does xml\_unescape(), same as ::parse\_str().|
|18917/18919/18925|MGR: on Linux, accept 2 optional args: -clientdir or -e and -datadir or -d, use when launching client _18925 containes a bugfix for this without a note in the commit message_|
|18963/18964|Mac installer: add logic for setting BOINC as screensaver under OS 10.6|
|18978/18981|Mac Sandbox: Security fixes for Mac OS 10.6 Snow Leopard|
|19044|client: in startup msgs, show resource shares, don't show prefs details _Should we show both?_|
|19050|client: add 

<fraction\_done>

 to boinc\_task\_state.xml|
|19051|client: change order in poll loop|


## Changesets for the (new OpenGL) screensaver (Might be useful some day) ##
|17128|Checkin the files needed for the new OpenGL based BOINC screensaver.|
|:----|:-------------------------------------------------------------------|
|17131|renamed boinc\_ss\_opengl to ss\_app.|
|17141/17144|SS: Begin work on upgrading screensaver coordinator for new functionality: add file names, sandbox security|
|17146/17147|SS: Implement basic new functionality in screensaver coordinator (tested on Mac only so far)|
|17148|SS: Add ss\_app to XCode Project, with a temporary icon|
|17162|screensaver: flesh out new SS a bit|
|17163/17164/17165/17167|SS: Fix ss\_app compile errors on Mac, continue work on screensaver coordinator|
|17168|SS: Screensaver coordinator reads ss\_config.xml file; if default ss ran during science phase, shorten next default phase|
|17193/17194|SS: Fix new screensaver coordinator to work properly on Windows|
|17199/17204/17205|tweak screensaver|
|17201|SS: Fix new screensaver coordinator sandbox permissions|
|17202|ss\_app: Replace temporary icon with standard BOINC icon|
|17217/17221|screensaver tweaks|
|17218/17219/17220|ss\_app: Remove code which adds a Mac icon, restore previous permissions for boincscr|
|17222|ss\_app: Change Windows build executable file name to boincscr.exe|
|17223/17224/17225|SS: Minor fix to screensaver coordinator|
|17255|ss\_app: Under sandbox security, set boincscr permissions the same as for BOINC Manager|
|17256|SS: On Windows, get paths to BOINC data and executable directories from Windows registry|
|17273|SS: On Windows, make path strings Unicode compatible in ss coordinator|
|17302/17303|screensaver|
|17321|screensaver: Implement --test and --retry\_connect command-line arguments as requested by Rom|
|17322|screensaver: Implement new screensaver coordinator logic as requested by Rom.|
|17336|WINSCR: Fix the BOINC text displayed is the screensaver control panel applet / WINSCR: [...]|
|17339/17340/17346/17349|screensaver: Code consolidation, fix bug terminating default graphics app on screensaver exit|
|17370/17393|WINSCR: Remove unneeded header files SCR: Shuffle headers around so that it'll build on both Windows and Mac without having to put in duplicate entries.|
|17405/17406|screensaver: show correct info if CPU throttling; show correct window title; show "no projects" message.|
|17408|screensaver: remove error codes & messages handled by new default gfx app, add new error codes; better logic when coordinator window covered on Mac|
|17444|WINSCR: fix a bug. Mac screensaver: logic to stop Data Management thread even if hung on an RPC.|
|17646|new screensaver: don't show jobs if suspended|
|17720|SS: Under Mac Sandbox security, gfx\_switcher launches default ss app as user and group boinc\_master; don't setgid boincscr|
|17722/17723|SS: Under Mac Sandbox security, terminate default screensaver graphics app via gfx\_switcher|
|17728/17731|WINSCR: It appears newer notebook models with multiple video chipsets exhibit an interesting situation. It appears as though in certain conditions a single monitor machine actually reports itself as having three monitors. Normally the monitor that contains the primary window (coord 0,0) is on monitor 0, but on these machines coord 0,0 is actually on monitor 2. This led to the screensaver not properly exiting when keyboard and/or mouse activity was detected. Now when we detect that keyboard and/or mouse activity has happened we send the WM\_INTERRUPTSAVER event to all windows on all monitors.|
|17815|SS: launch default screensaver graphics app as user and group boinc\_project, not boinc\_master|
|18222|SS: Add new optional boolean xml tag 

<default\_ss\_first>

|
|18444|SS: remove unused func|
|18867|Mac SS: Changes for compatibility with OS 10.6|

## Rejected changes ##

Changesets that are irrelevant to Synecdoche are not shown. Some decisions listed here make subsequent changesets irrelevant.

| **Changeset** | **Reason** |
|:--------------|:-----------|
|[15537](http://boinc.berkeley.edu/trac/changeset/15537)|Properties. This feature is nowhere near ready.|
|[15559](http://boinc.berkeley.edu/trac/changeset/15559)|Message filtering. This is not a good UI for filtering.|
|[15561](http://boinc.berkeley.edu/trac/changeset/15561)|Already fixed.|
|[15568](http://boinc.berkeley.edu/trac/changeset/15568)|Concatenated strings.|
|[15573](http://boinc.berkeley.edu/trac/changeset/15573)|Solaris. This addition appears to be in the wrong file.|
|[15574](http://boinc.berkeley.edu/trac/changeset/15574)|Already fixed. Correctly!|
|[15764](http://boinc.berkeley.edu/trac/changeset/15764) `*`|API|
|[15585](http://boinc.berkeley.edu/trac/changeset/15585)|Undocumented. This will be moot when we use wxTaskBarIcon.|
|[15587](http://boinc.berkeley.edu/trac/changeset/15587)|Event ids. This is pointless. See [r75](https://code.google.com/p/synecdoche/source/detail?r=75).|
|[15588](http://boinc.berkeley.edu/trac/changeset/15588)|Solved better in [r76](https://code.google.com/p/synecdoche/source/detail?r=76). Ours won't cycle the logs if an error is logged when parsing the config file.|
|[15591](http://boinc.berkeley.edu/trac/changeset/15591)|CUDA. We don't support this yet.|
|[15610](http://boinc.berkeley.edu/trac/changeset/15610)|Reboot. This is just wrong.|
|[15690](http://boinc.berkeley.edu/trac/changeset/15690)|Already fixed.|
|[15693](http://boinc.berkeley.edu/trac/changeset/15693) `*`|API|
|[15694](http://boinc.berkeley.edu/trac/changeset/15694) `*`|API|
|[15793](http://boinc.berkeley.edu/trac/changeset/15793)|Already fixed in [r171](https://code.google.com/p/synecdoche/source/detail?r=171).|
|[15801](http://boinc.berkeley.edu/trac/changeset/15801)|Already fixed in [r152](https://code.google.com/p/synecdoche/source/detail?r=152). Correctly!|
|[15808](http://boinc.berkeley.edu/trac/changeset/15808)|Already fixed in [r194](https://code.google.com/p/synecdoche/source/detail?r=194).|
|[15850](http://boinc.berkeley.edu/trac/changeset/15850)|Proposed async solution is not typesafe. Grid view was already fixed (removed).|
|[15891](http://boinc.berkeley.edu/trac/changeset/15891)|Useless and already fixed correctly|
|[15920](http://boinc.berkeley.edu/trac/changeset/15920)|Already fixed in [r265](https://code.google.com/p/synecdoche/source/detail?r=265).|
|[15942](http://boinc.berkeley.edu/trac/changeset/15942)|Already fixed in [r265](https://code.google.com/p/synecdoche/source/detail?r=265).|
|[15944](http://boinc.berkeley.edu/trac/changeset/15944)|Already fixed in [r236](https://code.google.com/p/synecdoche/source/detail?r=236).|
|[15948](http://boinc.berkeley.edu/trac/changeset/15948)|Already fixed in [r298](https://code.google.com/p/synecdoche/source/detail?r=298).|
|[15970](http://boinc.berkeley.edu/trac/changeset/15970)|Flickering of the MGR - Some additional changesets: 15978/15981/15990/15991/15993/15994/15995/15996/15998/16006/16185|
|[16029](http://boinc.berkeley.edu/trac/changeset/16029)|WCG "feature" for the wizards. No need for that.|
|[16038](http://boinc.berkeley.edu/trac/changeset/16038) `*`|API: Fix for boinc\_exit()|
|[16061](http://boinc.berkeley.edu/trac/changeset/16061) `*`|Exclude list for proxy. Useful feature but bad implementation. Additional changesets: 16066|
|[16087](http://boinc.berkeley.edu/trac/changeset/16087) `*`|"exclusive app" feature. Additional changesets: 16084/16085/16090/16175/16354/16552|
|[16095](http://boinc.berkeley.edu/trac/changeset/16095)|RPC calls are not obsolete just because the official manager does not use them any more.|
|[16118](http://boinc.berkeley.edu/trac/changeset/16118)|Sample wrapper, useless Makefile fix, debug code already removed in [r254](https://code.google.com/p/synecdoche/source/detail?r=254)|
|[16119](http://boinc.berkeley.edu/trac/changeset/16119)|Already fixed.|
|[16130](http://boinc.berkeley.edu/trac/changeset/16130)|Documentation fixes for doxygen|
|[16188](http://boinc.berkeley.edu/trac/changeset/16188)|Report job resources even when no active task. Coproc feature.|
|[16268](http://boinc.berkeley.edu/trac/changeset/16268)|Already fixed in [r449](https://code.google.com/p/synecdoche/source/detail?r=449).|
|[16357](http://boinc.berkeley.edu/trac/changeset/16357)|There is no need for this mess (except the change for rr\_sim.cpp).|
|[16358](http://boinc.berkeley.edu/trac/changeset/16358)|Already fixed in [r72](https://code.google.com/p/synecdoche/source/detail?r=72).|
|[16560](http://boinc.berkeley.edu/trac/changeset/16560)|No need for this new page and account key page already removed in [r526](https://code.google.com/p/synecdoche/source/detail?r=526).|
|[16754](http://boinc.berkeley.edu/trac/changeset/16754)|Another useless hack: "new work fetch logic". Additional changesets: 16755/16756/16765|
|[16900](http://boinc.berkeley.edu/trac/changeset/16900)|Adds platform information to the project list used when attaching to a new project. Platforms should be checked directly at the project not in a file maintained by boinc.berkeley.edu. Additional changesets: 16903/16905|
|[17056](http://boinc.berkeley.edu/trac/changeset/17056)|client: remove the deadlines\_missed" and "overworked" clauses - keeping them seems to be the better solution.|
|[17067](http://boinc.berkeley.edu/trac/changeset/17067)|MGR: Provide a way for skin creators to suppress error messages for missing resources. - This is obsolete, see changeset 17080.|
|[17076](http://boinc.berkeley.edu/trac/changeset/17076)|MGR: Filter project list so that it shows only projects supported by the core client. This relies on an up-to-date all\_projects\_list.xml instead of asking the projects about supported platforms. Additional changesets: 17077/17143|
|[17078](http://boinc.berkeley.edu/trac/changeset/17078)|Looks like we didn't break this.|
|[17212](http://boinc.berkeley.edu/trac/changeset/17212)|Already fixed in [r575](https://code.google.com/p/synecdoche/source/detail?r=575). Additional changesets: 17213/17214|
|[17335](http://boinc.berkeley.edu/trac/changeset/17335)|Already fixed in [r650](https://code.google.com/p/synecdoche/source/detail?r=650) and [r651](https://code.google.com/p/synecdoche/source/detail?r=651).|
|[17394](http://boinc.berkeley.edu/trac/changeset/17394)|Already fixed in [r511](https://code.google.com/p/synecdoche/source/detail?r=511).|
|[17398](http://boinc.berkeley.edu/trac/changeset/17398)|See comments in [#852](http://boinc.berkeley.edu/trac/ticket/852)|
|[17512](http://boinc.berkeley.edu/trac/changeset/17512)|Seems to work correctly even without this changeset.|
|[17519](http://boinc.berkeley.edu/trac/changeset/17519) `*`|Setting a fixed size seems to be a bad solution - Aren't there any better solutions?|
|[17542](http://boinc.berkeley.edu/trac/changeset/17542)|This should not be needed. Looks like the faulty changes were not merged and are not on the pending list. Additional changeset: 17560|
|[17501](http://boinc.berkeley.edu/trac/changeset/17501)|As jm7 noted on boinc\_dev on 03/15/2009 this factor is there for a good reason.|
|[17208](http://boinc.berkeley.edu/trac/changeset/17208)|Obsolete due to [r706](https://code.google.com/p/synecdoche/source/detail?r=706). Additional changesets: 17209/17270|
|[17596](http://boinc.berkeley.edu/trac/changeset/17596)|We don't need these tabs, do we?|
|[18077](http://boinc.berkeley.edu/trac/changeset/18077) `*`|MGR: Include support for Firefox 3.x cookie support for the attach to project wizard.|
|[18088](http://boinc.berkeley.edu/trac/changeset/18088)|MGR: Merge "Attach to account manager" functionality into "Attach to project" wizard; Synchronize and Remove Acct Mgr not yet merged. Additional changesets: 18112/18113/18114/18115 _Doesn't make any sense to me._|
|[18311](http://boinc.berkeley.edu/trac/changeset/18311)|MGR: !CBOINCClientManager::ProcessExists?() finds the process by name if we don't have a pid (Mac, Linux) or process HANDLE (Windows) _What is the purpose of this?!?_ Additional changesets: 18316/18317/18318/18319|
|[18592](http://boinc.berkeley.edu/trac/changeset/18592) `*`|file\_info changes - Nicolas should review this, he knows more about this stuff.|
|[18607](http://boinc.berkeley.edu/trac/changeset/18607) `*`|client: change the way a resource's "estimated delay" (passed to server for crude deadline check) is computed. _Doesn't seem to be a real fix but might still be a large improvement._ Additional changeset: 18629|
|[18816](http://boinc.berkeley.edu/trac/changeset/18816)|MGR: Implement "Show active tasks / Show all tasks" button _We should implement some real filtering mechanism_|

`*` These changesets may still be needed.

Reviewed up to: [19100](http://boinc.berkeley.edu/trac/changeset/19100)