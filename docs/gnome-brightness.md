# GNOME display brightness on the HP OmniBook

GNOME Shell 51 provides the Brightness slider in Quick Settings and the on-screen brightness indicator. The C patch for Mutter 51.1 connects the OmniBook's internal eDP-1 display in SDR mode to FreeBSD's Intel panel backlight. The signed-in user needs read/write access to `/dev/backlight/intel_backlight0`. On the test laptop the device belongs to the video group.

The slider writes a percentage through the FreeBSD backlight ioctl. Mutter reads the device every 250 ms to pick up changes from the kernel brightness keys, the legacy key service, or backlight(8). A hardware change updates the same brightness property that Shell watches. Use one hotkey handler as described in the main README.

## Build

Build both BSD³ GNOME 51 ports with their respective patches. The Shell patch propagates a hardware-initiated change from the monitor brightness scale to the Quick Settings slider and on-screen indicator. From the hotkeys repository, substitute the path to your GNOME 51 ports overlay below. Use a clean port work directory.

```sh
make -C /path/to/packaging/freebsd/mutter51 \
    EXTRA_PATCHES="$PWD/gnome/mutter-51.1-freebsd-backlight.patch:-p1" package
make -C /path/to/packaging/freebsd/gnome-shell51 \
    EXTRA_PATCHES="$PWD/gnome/gnome-shell-51.1-backlight-sync.patch:-p1" package
```

Install the resulting packages through your usual local package workflow. The running Wayland compositor loads the new library at the next GNOME login.

## Verify

1. Open Quick Settings. Move the display brightness slider and check the value with `backlight -q -f /dev/backlight/intel_backlight0`.
2. Tap the display dimmer once with the slider above 5%. The readout and slider should drop by five percentage points. Tap the brighter key once to restore the starting value.
3. Close Quick Settings and tap either key. Shell should show its brightness indicator. At either limit, a key that leaves the level unchanged produces no change notification.

To inspect the compositor's published outputs, run this in the GNOME session. Look for eDP-1 in the returned backlight array.

```sh
gdbus call --session --dest org.gnome.Mutter.DisplayConfig \
    --object-path /org/gnome/Mutter/DisplayConfig \
    --method org.freedesktop.DBus.Properties.Get org.gnome.Mutter.DisplayConfig Backlight
```

## Test results

The patched Mutter 51.1 shared library built on FreeBSD 15.2-STABLE. An isolated GNOME Shell 51.1 session used a test-only virtual output wired to the physical Intel panel. Moving the real Quick Settings slider set the panel to 85%. Setting the panel back to 90% with backlight(8) updated both the slider and the on-screen indicator. The laptop owner subsequently confirmed the slider worked in the daily GNOME login on the internal panel.
