# GNOME keyboard brightness indicator

This GNOME Shell 51 extension shows the OmniBook keyboard-light level next to a keyboard icon in the top bar. The C widget reads the kernel module on a worker thread and refreshes the percentage after hardware-key presses. The indicator hides when the kernel reading is unavailable.

Build and load [the read-only kernel module](../../tools/omnibook_kbdlight/README.md). The widget needs the GNOME Shell 51 source and configured build directories for the private St widget headers. From this directory, run the following with the matching local paths. Set EXTDIR if your GNOME session uses a custom XDG data directory.

```sh
make SHELL_SRC=/path/to/gnome-shell-51.1 SHELL_BUILD=/path/to/build install
```

At the next GNOME login, enable it with `gnome-extensions enable kbdlight@omnibook`. The keyboard-light key cycles between 0%, 50%, and 100%.
