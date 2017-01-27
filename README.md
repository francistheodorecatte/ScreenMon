# ScreenMon
ScreenMon is a WINDOWS ONLY application for monitoring a target program's main window area to see if it's changing. Based on that, it outputs data to STDOUT for other programs and scripts to act upon.

# Behavior
While running, ScreenMon will output TRUE or FALSE to STDOUT depending on whether it detects the target program's window contents changing. State changes across the check interval trigger a TRUE (1), no state changes trigger a FALSE (0).

If the target program isn't running (or doesn't exist), ScreenMon will exit with a code of 2. This isn't printed to the console, but any application that invoked ScreenMon should be able to catch that. Similarly, if the target program hangs, ScreenMon will exit with a code of 3.

# Usage
In order to work, you have to provide the program name of a program to monitor (e.g. firefox.exe).

ex: screenmon.exe -p firefox.exe


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
To compile this program yourself, you need the TCLAP and Boost libraries installed. A lot of the functions I use are also only available on Windows 7 or newer, and require .NET installed to work. Additionally, you may need the VC++ x86 redist depending on what compiler you use.

I provide this code with no warranties or guarentees. Being a dev isn't my day job. :)
