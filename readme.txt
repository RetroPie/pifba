=Final Burn Alpha 2x for Raspberry Pi by Squid=

*INTRODUCTION*

This is Final Burn Alpha 2x for the Raspberry Pi ported by Squid. This emulates old archive games using MAME based roms for CPS1, CPS2, NeoGeo, System 16, Toaplan and many other games. It should support most recent versions of the MAME versions of these ROMS but unfortnately there is no definitive version to use.

Some large ROMS require converting to the '.fba' format before they will work. FBACache_windows.zip is included to do this, which runs on Windows not Linux.

A number of games will not work properly, some are so large they appear to run out of memory.

It uses a modified version of the Capex frontend for game selection but you can use your own frontend and simply call the "fba2x" to play the games.

This port supports keyboard and USB joystick input and are configurable. It uses GLES2 graphics API so can run equally well in the Console or under X-Windows. The audio uses the ALSA API and SDL is used for input.

Download it from the official Raspberry Pi Store App:
http://store.raspberrypi.com/projects/pifba

Web page for news, source, additional information:
http://code.google.com/p/pifba/

(No asking for ROMS, problems with ROMS)


*CONTROLS*

Keyboard controls:
{{{
'up arrow'       Up direction
'down arrow'     Down direction
'left arrow'     Left direction
'right arrow'    Right direction
'control'        Button 1 (starts a game in the frontend)
'alt'            Button 2
'space'          Button 3
'shift'          Button 4
'z'              Button 5
'x'              Button 6
'return'         Start button (opens config menu in the frontend)
'tab'            Select button (quits in the frontend)
'escape'         Quit
}}}

The joystick layout defaults to Saitek P380 button layout.

Additionally to quit with the joystick you can press SELECT+START.

All controls are configurable by editing the "pifba.cfg" file.

The frontend displays the list of ROMS in the following colours:
RED = missing
ORANGE = clone rom detected
YELLOW = parent rom detected or clone rom detected & parent rom detected
GREEN = clone rom detected & parent rom detected & cache detected
BLUE = parent rom detected & cache detected


*INSTALLATION*
{{{
fba2x      -> Game binary
fbacapex   -> Frontend binary
fba2x.cfg  -> Keyboard and joystick configuration
rominfo.fba -> required by the frontend
zipname.fba -> required by the frontend
preview/   -> screenshots (BMP 192x112)
roms/      -> ROMs directory (Put your game ROMs in here)
skin/      -> Frontend skins directory
FBACache_windows.zip -> for converting large ROMS to .fba cache format
}}}

To run piFBA simple run the "fbacapex" executable. At the command line "./fbacapex".
This runs the GUI frontend. To simply run piFBA without the GUI enter "./fba2x roms/{gamerom}" where "{gamerom}" is the game rom filename to run.

It will work in X-Windows or in the Console.

For the frontend screenshots you need to put the relevant screenshot files in the preview subdirectory. These files should be BMP and 192x112 dimensions.


*Pi CONFIGURATION*

I highly recommend overclocking your Raspberry Pi to gain maximum performance
as PiFBA is very CPU intensive and overclocking will make most games run at full speed.

My overclocking settings which work well, (/boot/config.txt)
{{{
arm_freq=900
sdram_freq=500
}}}

NOTE: Make sure overclocking is actually working by checking "cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor" should be "ondemand". Later kernels appear to set it to "powersave" by default.

If your sound is too quiet then do the following to fix that:
{{{
First get the playback device, type "amixer controls"
This will show the numid for the playback device, probably 3.
Now set the volume, type "amixer cset numid=3 90%".
Then reboot to make it permanent.
}}}

If you're having problems with HDMI audio then it is likely PulseAudio
is causing the issues as it has problems with the ALSA drivers. To fix
this simply remove PulseAudio:
{{{
sudo apt-get --purge remove pulseaudio
sudo apt-get autoremove
}}}

If you're getting a black screen when running in Console mode with
Composite output, try removing/commenting out the "overscan_" parameters from "/boot/config.txt" as follows (using disable_overscan doesn't appear to fix it):
{{{
#overscan_left=16
#overscan_right=16
#overscan_top=16
#overscan_bottom=16
}}}

*GRAPHICS EFFECTS*

Postprocessing can be enabled by setting "DisplayEffect" in fba2x.cfg. In addition you can disable anti-aliasing smoothing ("DisplaySmoothStretch" in fba2x.cfg). For the scanlines filter it is recommended you switch off "DisplaySmoothStretch".

*PORT CREDITS*

  * Ported to and optimised for the Raspberry Pi by Squid.

*ORIGINAL CREDITS*

  * Final Burn copyright Team FB Alpha.
  * FBA2X version 7.3 by HeadOverHeels.
  * CAPEX frontend originally by JYCET and Juanvvc
  * FB Alpha is based on Final Burn (formally at http://www.finalburn.com/).
  * Musashi MC68000/MC68010/MC68EC020 core by Karl Stenerud (http://www.mamedev.org/).
  * A68K MC68000 core by Mike Coates & Darren Olafson (http://www.mamedev.org/).
  * YM3812 core by Jarek Burczynski & Tatsuyuki Satoh (http://www.mamedev.org/).
  * YM2151 core by Jarek Burczynski (http://www.mamedev.org/).
  * YMF278B core by R. Belmont & O.Galibert (http://www.mamedev.org/).
  * YM2608/YM2610 cores by Jarek Burczynski & Tatsuyuki Satoh (http://www.mamedev.org/).
  * AY8910/YM2149 core by various authors (http://www.mamedev.org/).
  * M6502 emulation core by Marat (http://fms.komkon.org/EMUL8/).
  * I8039 emulation core by Mirko Buffoni (http://www.mamedev.org/).
  * ARM7 emulation core by Steve Ellenoff (http://www.mamedev.org/).
  * Z80 emulation core by Dave, modifications by Team FB Alpha (http://www.finalburn.com/).
  * Alternative Z80 emulation core by Juergen Buchmueller (http://www.mamedev.org/).
  * Alternative Z80 emulation core by Stéphane Dallongeville, modifications by Team FB Alpha.
  * PNG functionality provided by libpng (http://www.libpng.org/) and PNGlib (http://www.madwizard.org/).
  * Zip functionality provided by zlib (http://www.zlib.net/).

*CHANGE LOG*

_July 15, 2013:_
  * Initial release.

