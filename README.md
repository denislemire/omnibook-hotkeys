# HP OmniBook hotkeys for FreeBSD

Hardware-tested patches for the HP OmniBook 5 16-ba1xxx (SKU `B8MX7UA#ABL`) on FreeBSD 15.2-STABLE.

| Control | What it does | Implementation |
| --- | --- | --- |
| Display brightness | Raises or lowers the internal Intel panel by five percentage points per press, including outside GNOME. | [ACPI video and backlight patch](patches/omnibook-acpi-video.patch) |
| GNOME display brightness | Quick Settings controls the panel and follows hardware-key changes. | [Mutter and Shell patches and build instructions](docs/gnome-brightness.md) |
| Microphone mute | Sends `KEY_MICMUTE` so GNOME can toggle capture. | [AT keyboard patch](patches/omnibook-micmute-key.patch) |
| Microphone-key LED | Lights when PulseAudio mutes the built-in microphone, including through GNOME Settings. | [HDA LED patch](patches/omnibook-micmute-led.patch) and [C session listener](tools/omnibook-micmute/) |
| F5 speaker-mute LED | Lights when the default PulseAudio output is muted. The same C listener drives both indicators. | [Speaker LED patch](patches/omnibook-speaker-led.patch) |
| Keyboard illumination | Shows the firmware-selected level (0%, 50%, or 100%) in the GNOME top bar. | [Kernel module](tools/omnibook_kbdlight/) and [GNOME indicator](gnome/keyboard-indicator/) |

## Building and testing

Check the model with `kenv smbios.system.sku`. From the root of this repository, apply the patches in order to a clean kernel source worktree. The speaker LED patch extends the microphone LED driver.

```sh
src=/path/to/freebsd-src
for patch in omnibook-acpi-video omnibook-micmute-key omnibook-micmute-led omnibook-speaker-led; do
    git -C "$src" apply "$PWD/patches/$patch.patch" || break
done
```

After all four apply, build the kernel and its matching modules, including `acpi_video` and `snd_hda`. Boot the resulting kernel and load its `acpi_video` module. If the original brightness service is enabled, stop and disable it before testing the native handler.

The audio LEDs use an unprivileged C listener linked to libpulse. The local FreeBSD package installs it in `/usr/local/libexec` and starts it at GNOME login. See the [package, permissions, and verification instructions](docs/omnibook-microphone-mute.md).

## Why the brightness patch is needed

The firmware sends ACPI notifications `0x86` (brighter) and `0x87` (dimmer) to the internal display object `\_SB.PC00.GFX0.DD1F`. On this laptop the stock `acpi_video` module attached to the GPU but enumerated no display outputs. The patch binds `DD1F` when enumeration finds no outputs on this SKU.

Binding the panel alone let FreeBSD receive the keys. Two dimmer presses moved the firmware brightness value from 80 to 78 while the Intel backlight stayed at 80%. The patch therefore routes this panel’s brightness reads and writes through the registered `backlight/intel_backlight0` provider. Each hotkey requests a five-percentage-point step, bounded at 0 and 100.

The patch changes both `acpi_video` and the kernel backlight framework. It was tested by booting a kernel and module built together, with the helper stopped. The tester observed the keys changing the panel brightness, and both `sysctl -n hw.acpi.video.lcd0.brightness` and `backlight -q -f /dev/backlight/intel_backlight0` reported 90%. The patch is a local hardware-tested change.

## Unpatched kernels

The original Python and DTrace service remains available for display brightness. [Installation and removal](docs/python-brightness-service.md)
