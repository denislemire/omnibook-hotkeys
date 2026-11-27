# HP OmniBook 5 microphone mute key and indicator

Verified on the 16-ba1xxx, SKU `B8MX7UA#ABL`, with the Realtek ALC245 codec and HP subsystem `103c:8e0a`.

The internal keyboard sends extended Set 1 scan codes `E0 02` on press and `E0 82` on release. The [atkbd patch](../patches/omnibook-micmute-key.patch) maps them to `KEY_MICMUTE` on this SKU. GNOME toggled the built-in microphone after booting the patched kernel.

The microphone-key LED is wired to **HDA codec GPIO 2, active low**. On this machine, setting `dev.hdaa.0.gpio_config` to `2=clear` lit the LED, and `2=set` turned it off. Both states were confirmed visually with the key untouched. The other GPIOs remained disabled.

GNOME controls the PulseAudio source `oss_input.dsp0`. While that source reported muted, the FreeBSD mixer still reported `mic.mute=off` and `rec.mute=off`. A desktop indicator should read the built-in source's mute state at startup and on source changes, including changes through GNOME settings. A keypress alone cannot determine the indicator state.

## Installed integration

The [HDA driver patch](../patches/omnibook-micmute-led.patch) exposes `/dev/led/omnibook-micmute` on the ALC245 with subsystem ID `103c:8e0a`. Write `1` to light it and `0` to turn it off. The driver handles the active-low wiring. Apply both patches to the matching HPBSD source tree and rebuild the kernel and snd_hda module together.

The [session listener](../tools/omnibook-micmute/omnibook-micmute.c) is a C client of libpulse. It subscribes to source changes, reads the initial state after subscribing, and reconnects if PulseAudio restarts. GNOME starts it through XDG autostart. Both GNOME and PulseAudio use their existing interfaces.

On the target FreeBSD system, run `tools/omnibook-micmute/build-package.sh` from the checkout and install the resulting local package with `pkg add ./omnibook-micmute-0.2.pkg`. It installs the executable in `/usr/local/libexec` and its desktop entry in `/usr/local/etc/xdg/autostart`. Grant the desktop user's group write access to this single LED. On the test laptop the user belongs to operator, with these entries in `/etc/devfs.conf`:

```
own led/omnibook-micmute root:operator
perm led/omnibook-micmute 0660
own led/omnibook-speaker-mute root:operator
perm led/omnibook-speaker-mute 0660
```

After GNOME login, toggle the microphone from the key and from Settings. Compare the key LED with `pactl get-source-mute oss_input.dsp0`. The temporary GPIO test verified both polarities and desktop synchronization. Repeat this check after booting the packaged integration.

## F5 speaker mute indicator

Apply the [speaker LED patch](../patches/omnibook-speaker-led.patch) after the microphone LED patch. The same local package (version 0.2) watches PulseAudio's default output and server changes as well as the built-in microphone. The two indicators are independent.

On HP subsystem `103c:8e0a`, setting Realtek ALC245 vendor node `0x20`, coefficient `0x0b`, bits 3:2 to binary `10` lit F5. The original unmuted setting was binary `01`. The driver exposes this control as `/dev/led/omnibook-speaker-mute`. After desktop login, compare F5 with `pactl get-sink-mute @DEFAULT_SINK@` while toggling output mute in GNOME.

The HPBSD-audio-leds boot test used package 0.2. Both LED device nodes appeared, one GNOME listener process was running, and the tester reported the indicators working.
