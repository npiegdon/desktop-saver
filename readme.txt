DesktopSaver
http://desktopsaver.sourceforge.net/

Copyright (c)2012 Nicholas Piegdon
See license.txt for license information

DESCRIPTION
===========

DesktopSaver is a Windows application that keeps track of desktop icon positions over time.  If your icon positions are disturbed (after a video resolution change, an explorer crash, changing video device settings, docking your laptop, etc) DesktopSaver can restore their original positions in a single click.

DesktopSaver is especially good at keeping track of icon positions over time without forcing you to manually create desktop profiles yourself.  It takes zero configuration and zero maintenance.


REQUIREMENTS
============

Win7, Vista, XP: should work.
Win2k: might work.
Win98: won't work.

In general, DesktopSaver is a good neighbor using a tiny memory footprint of maybe 1-3MB of RAM.


USAGE
=====

Run the program.  It will create a system tray icon.  All of the application's functionality can be accessed by clicking this icon. 

The first time you click the icon, you'll see "Initial History" at the top of the history list.  This is the first icon "snapshot".

Try clicking the system tray icon again after moving a desktop icon or two.  You should see a new entry at the top of the list.  You can then choose "Initial History" to your icons back to the first "snapshot".

"Run at Startup" will toggle the usual registry entry, and "Clear History" does just that - including wiping out the hard-drive file... no traces left.

If your icons are disturbed frequently and you find that the icon history list is scrolling off your last "good" snapshot too quickly, you can use the "named profile" feature to create a permanent snapshot.


A NOTE ABOUT "POLLING"
======================

DesktopSaver exchanges windows messages with the Windows Explorer process to obtain (and set) the positions of your desktop icons.  This "polling" will occur after the following:

- The application starts.
- The application closes.
- You manually click the system tray icon.
- At the poll rate set in the options menu.

Polling takes about 2ms if no changes have been made to the desktop, and only 15ms (involving a disk write) if they have.  So, for example, if you make a change to your desktop just before playing a game, within the first poll rate of playing you might suddenly lose 1 fps.  After that however, any following "polls" should be virtually undetectable (assuming 1 fps is "detectable" in the first place).


CHANGE LOG
==========

DesktopSaver 2.1:

NEW: Official 64-bit support, built against the Windows SDK v7.1.

NEW: Auto-start profile option in menu to restore a named profile when the
     program is started.  (Patch submitted by Iwan Mouwen.)

CHG: Dropped backward compatibility support for old ANSI save format that
     hasn't been in use for six years.

------------------------------------------------------------------------------

DesktopSaver 2.0.4:

NEW: You can now add a command-line argument to auto-load a profile, say,
     from a script.  It will only work in XP or later, and there is no
     output aside from a return code of 0 if the profile was found and
     loaded or 1 if it wasn't.  Spaces and Unicode are allowed.

------------------------------------------------------------------------------

DesktopSaver 2.0.3:

CHG: Extra-long filenames are now truncated in the history list view.

NEW: Unicode desktop icon names and profile names are now supported.
    (Remaining profile history from previous versions will be silently
    backed up and upgraded.)

------------------------------------------------------------------------------

DesktopSaver 2.0.2:

CHG: Converted project files to use Microsoft Visual C++ 2005 Express Edition.

CHG: Updated the NSIS script to make the installer's attempt at stopping any 
     running instances of DesktopSaver much more graceful and cross-platform.

FIX: Issue 1467204: Fixed.  Replaced the WinXP-only call to
     SHGetFolderPathAndSubDir with the more compatible SHGetFolderPath.

------------------------------------------------------------------------------

DesktopSaver 2.0.1:

CHG: Made a few small changes to how the version number was handled in
     the source and in the install script.  Version numbers (while still
     set by hand) should be more consistent with the versioning scheme
     being used on the SourceForge page.

NEW: Made uninstaller attempt to stop any running instances of DesktopSaver
     in hopes of a cleaner uninstall.  (This only works in Windows XP).

NEW: Made installer attempt to stop any running instances of DesktopSaver
     in hopes of successfully installing over an old version.  (This only
     works in Windows XP).

NEW: Added a section to the Installer to "Run after Install".

------------------------------------------------------------------------------

DesktopSaver 2.0.0:

CHG: Rebranded the application "DesktopSaver".

NEW: Added "Named Snapshots" feature.

NEW: Added "Poll Speed" and "History Count" options.

NEW: Added a Nullsoft Install System script.

NEW: Created a new (albeit, amateur) application icon in order to free all
     assets from proprietary licenses.

CHG: Gave the source a once-over to prepare it as an open source project.

CHG: Made history file write to limited-account writeable location.

------------------------------------------------------------------------------

IconSaver 1.1:

FIX: Fixed an issue where a new ("initial") history slice wasn't being
     generated after clearing the history.

------------------------------------------------------------------------------

IconSaver 1.0:

NEW: Initial release.

------------------------------------------------------------------------------
