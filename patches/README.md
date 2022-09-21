
Debian
====================
This directory contains files used to package lexdd/lexcoin-qt
for Debian-based Linux systems. If you compile lexcoind/lexcoin-qt yourself, there are some useful files here.

## lexcoin: URI support ##


lexcoin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install lexcoin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your lexcoin-qt binary to `/usr/bin`
and the `../../share/pixmaps/lexcoin128.png` to `/usr/share/pixmaps`

lexcoin-qt.protocol (KDE)

