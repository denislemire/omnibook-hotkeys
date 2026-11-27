# Python brightness service for unpatched kernels


The original DTrace service runs independently of the desktop session. Use only one hotkey handler at a time.

## Hardware and requirements

The tested machine runs FreeBSD 15.2-STABLE on amd64 and reports SKU `B8MX7UA#ABL`. The helper checks that SKU at startup. It requires Python 3.12 at `/usr/local/bin/python3.12`, the native `backlight(8)` utility, an Intel panel backlight device, and root access to the DTrace FBT provider. The probe uses an internal ACPICA function and type whose availability can vary between kernel builds.

Check the prerequisites on the target machine:

```sh
kenv smbios.system.sku
/usr/local/bin/python3.12 --version
backlight -q -f /dev/backlight/intel_backlight0
sudo dtrace -l -n 'fbt::AcpiEvQueueNotifyRequest:entry'
```

## Install and test

From the cloned repository, install the two executables and start the service:

```sh
sudo install -o root -g wheel -m 0555 omnibook-brightness.py /usr/local/libexec/omnibook-brightness.py
sudo install -o root -g wheel -m 0555 omnibook_brightness /usr/local/etc/rc.d/omnibook_brightness
sudo sysrc omnibook_brightness_enable=YES
sudo service omnibook_brightness start
sudo service omnibook_brightness status
```

Run `backlight -q -f /dev/backlight/intel_backlight0`, tap the dimmer key twice, then run it again. From 100% the expected result is 90%. One press of the brighter key should then give 95%. If the service exits, check `/var/log/messages` for messages tagged `omnibook_brightness`. If a future desktop or driver update starts handling the keys itself, stop this service and repeat the test to check for duplicate steps.

## How it works

On the tested firmware the embedded controller's ACPI methods `_Q11` and `_Q12` notify the display object `\_SB.PC00.GFX0.DD1F`. The service filters queued ACPI notifications by the four-byte node name `DD1F` and the two brightness codes. For `0x87` it runs `backlight decr 5`; for `0x86` it runs `backlight incr 5`. Both commands explicitly target the internal panel. It does not need keyboard capture, a GNOME shortcut, or an ambient-light sensor.

During hardware testing on this SKU, two physical dimmer presses produced two `DD1F 0x87` notifications, and the prototype moved the panel from 100% to 90%. The installed rc service was verified running and enabled. A cold-boot test and an isolated brighter-key measurement were still outstanding at publication.

## Uninstall

```sh
sudo service omnibook_brightness stop
sudo sysrc -x omnibook_brightness_enable
sudo rm /usr/local/etc/rc.d/omnibook_brightness /usr/local/libexec/omnibook-brightness.py
```
