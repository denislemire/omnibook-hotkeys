# HP OmniBook keyboard illumination

This FreeBSD kernel module reads the keyboard illumination selected by the embedded controller on the HP OmniBook 5 16-ba1xxx, SKU `B8MX7UA#ABL`. Build with kernel sources matching your running system.

```sh
make
sudo kldload ./omnibook_kbdlight.ko
sysctl -n hw.omnibook_kbdlight_percent
sudo kldunload omnibook_kbdlight
```

On board 8E0A with BIOS F.16, the physical key invoked ACPI EC query method `_Q41`. That method calls `\\_SB.PC00.LPCB.EC0.ERCD` with the six bytes `45 60 00 00 00 00`. The first byte of the returned eight-byte buffer is the selected percentage. The module issues the same query on demand. Two complete key cycles produced 50, 100, 0, 50, 100, 0, 50.

## Software control

The read-only sysctl can verify future attempts to change the actual light level. A verified firmware setter is still needed before exposing a writable GNOME control through UPower.

The `_Q41` method reports the new level to WMI after the EC handles the key. Evaluating it from software would repeat the notification. A separate method, `_Q55`, cycles the timeout field `KBBT`; WMI method `SBKT` also writes that field. Neither method sets the illumination level. The keyboard light is separate from the display panel and the microphone and speaker mute LEDs.
