# ScreenMon
ScreenMon is a WINDOWS ONLY application for monitoring a target program's main window area to see if it's changing. Based on that, it outputs data to STDOUT for other programs and scripts to act upon.

# Usage
In order to work, you have to provide the program name of a program to monitor (e.g. firefox.exe).
ex: screenmon.exe -p firefox.exe

This will cause ScreenMon to open and monitor firefox.exe. If the window isn't changing

If the program has multiple windows open, ScreenMon will currently just grab the parent window (highest in the stack.) By default, it'll also check the whole window region for changes. This can be changed with a couple of switch options at startup:

-x is the x coordinate of the top left corner of the window region you wish to watch.
-y is the y coordinate.
-h is the height of the window region you wish to watch (in pixels)
-w is the width.

ex: screenmon.exe -p obs32.exe -x 150 -y 150 -h 1920 -w 720

Obviously this isn't very safe on a computer that a user could interact with, since they could inadvertantly resize the window. Dealing with that would have to be up to the parent application or script running ScreenMon.

If you wish to change the speed at which ScreenMon checks the target window (the default is 1 second,) you can use the -i switch. The interval time is in milliseconds, so if you want to check every half a second you would use -i 500.
ex. screenmon.exe -p firefox.exe -i 500

I haven't checked how fast the program can go before it starts lagging and missing updates, but 1 second seems pretty safe and works well for my needs.

There's also a verboseness switch that you activate using --verbose. Not useful for running in scripts, but useful for debugging. Produces a lot of console spam.
ex: screenmon.exe -p obs32.exe --verbose

# Warnings, dependencies, etc.
As of this writing, if the program you entered isn't currently running (has no PID,) the program will crash. Once I put in some error handling, it'll exit with a code of 2 to STDOUT. Additionally, there's no check to see if the program is minimized; it'll just always report true to STDOUT-- this will probably also change, since there is a Windows function call to check for this.

To compile this program yourself, you need the TCLAP and Boost libraries installed. A lot of the functions I use are also only available on Windows 7 or newer, and require .NET installed to work. Additionally, you may need the VC++ x86 redist depending on what compiler you use.

I provide this code with no warranties or guarentees. Being a dev isn't my day job. :)
