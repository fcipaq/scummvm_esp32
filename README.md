# SCUMM-VM for ESP32

This is ScummVM running on the ESP32 (S3). It's a fork of johannesbehr/scumm (all creadits go to him!) I mostly did some tidy work and ported everything from Odroid to a more generic platform. SD card access is done with the ESP-IDF drivers, gfx is handled by LovyanGFX and the PWM audio output is my own "driver" (if that's what you want to call that piece of software). It's intended to run on the "PicoHeld ESP32" - a DIY handheld I designed. But any ESP32 (S3) with an SD card reader and a Lovyan-compatible LCD should be fine.

I mostly tested "Day of the Tentacle" which seems to run fine. Sam'n'Max also seems to run fine with sound, but music sometimes stops. Full Throttle runs with speech but no sound and no music (except for the cut scenes).

Animated cursors work as well. Saving is possible but there is no interface (see below)

![DOTT on the PicoHeld](images/dott.jpg)

For hardware related configs please look at components/backends/platform/esp32:

  osystem-audio.cpp   --> set PIN_SND to fit your setup
  osystem-display.cpp --> pin assignment for the LCD interface and the gfx setup for LovyanGFX
  osystem-events.cpp  --> key combo for gamestate saving/loading (i.e. the UI)
  osystem-input.cpp   --> pin assignment for the buttons
  osystem-sdcard.cpp  --> pin assignment for SD card

I built it using ESP-IDF v5.2.1

Enjoy!

# Original Readme:

# SCUMM-VM for ESP32 / Odroid-Go

This is my try to port the SCUMM-VM (See: https://www.scummvm.org/) to ESP32. Currently it works with a lot of restrictions:
- Only works with SCUMM-Games up to V6 and Cine/Future Wars
- Only available engines are SCUMM-Engine and Cine-Engine
- No sound
- No animated cursor
- Limited load/save function (Menu-Key makes quicksave and Select+Menu load the quicksave again)
...

But it proofs that SCUMM can run on the ESP32!

You can find an installable FW here: https://github.com/johannesbehr/scumm/blob/master/release/Go-Scumm.fw

To try you need the files monkey1.000 and monkey1.001. 
Put them in the folder: roms/scummvm/monkey1/
