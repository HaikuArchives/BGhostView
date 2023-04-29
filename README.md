Readme for BGhostview by Andreas Raquet, updated by PulkoMandy

Copyright: GPL, see file COPYING
Disclaimer: This is beta-software. Use it at your own risk!

Overview
========

This started as a port of KGhostview for X11/KDE, a long time ago.

However, it has since been largely rewritten, not only to use BeOS and then Haiku APIs, but also to
use better APIs introduced in newer versions of GhostScript.

Thanks to Tim Theisen and Mark Donohoe for the original works.

I'd also like to thank Jake Hamby, whose port of Ghostscript and helpful advice made this port
possible.

Requirements
============

- Ghostscript (tested with versions 9 and up)
- libprefs

Ghostscript and libprefs are available in Haikuports and installable as usual.

Installation
============

Install it using HaikuDepot or pkgman. Nothing special is needed.

Open the Filetype-preferences and set BGhostview as the default-tool for Postscript and
(if you want) for PDF.

Compilation
===========

Install the development files for libprefs and ghostscript.

Goto the src-directory and build BGhostview using the
makefile.

Restrictions of the BeOS version
================================

The rendering is done using gsapi, the standard API for using Ghostscript. This can work on only
one document at a time. So, it is not possible to have multiple documents open in the app at the
same time - but you can simply start the app multiple times instead.

A more severe - sometimes annoying difference to the
X11-Version is the fact that Ghostscript for BeOS always
uses a Background-buffer for rendering and copies the
content into the BGhostview-view when it is finished.
This means that before rendering is complete, you will
not see anything but a white page on BGhostview.
More important, it also means that memory is required
for the whole page, as opposed to only the part
that is currently visible, which can get really problematic
when trying to view large pages in 400% zoom-mode (I have
disabled higher zoom modes to save you from the frustration
of watching your drive swapping for minutes). 
The X11-version does usually not use backingstore (although it can)
and you can watch a ps-file at any magnification as it is 
being rendered.

The current version of gsapi provides the needed functions to do things like on X11, rendering a
small rectangle at a time. However, on modern hardware this may not be as useful as it was when the
paragraph above was originally written.


Bugs
====

There are a few.

Before you report bugs, please make sure that your problem isn't
Ghostscript-related (view the file in question with gs).


Todo
====

- printing support isn't done yet
- I'd like to have some nicer icons (using HVIF)
- enhanced support for PDFs and whatever users request
- Maybe support DVI files as well?

