# Overview

The goal for this project is to create a stack that lets you use WinAmp input plugins on Android. WinAmp input plugins support loading & decoding different file formats like MP3, NSF, SPC, etc.

I anticipate the stack will look like this:

    Android app
     + QEMU x86 emulator
       + Minimal x86 Linux distribution
         + Wine
           + Win32 WinAmp plugin host app

## Win32 WinAmp plugin host app

The plugin host app emulates WinAmp, as far as input plugins are concerned. It will need to manage the input plugin, and also emulate an output plugin for the audio playback.

Since the emulated portion will be headless, the Windows UI won't be visible and our UI doesn't really matter.

To communicate with the Android app, the host app will provide an RPC mechanism, effectively allowing the input plugins to be used over a network socket – possibly ZeroMQ + msgpack.

# Out of scope

## Input plugins that don't use output plugins

Most input plugins decode audio and pass it directly to some WinAmp-provided output plugin for the actual playback (or WAV/MP3 creation, etc), but input plugins are also allowed to output audio directly. We want to pass the audio back to the Android app for playback using the Android audio API, so input plugins that don't use output plugins won't work.

## UI provided by input plugins

Input plugins can show windows, and particularly they often have at least _About_, _File Info_, and _Config_ windows. However, wine will be running in a headless environment, so these windows won't be visible.

This limitation might become a problem, as configuration is sometimes (albeit rarely) important, and some file formats that have multiple songs in a single file are controlled using plugin UI.

