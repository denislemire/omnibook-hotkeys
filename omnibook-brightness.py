#!/usr/local/bin/python3.12
"""Translate this OmniBook's internal-panel ACPI notifications to brightness steps."""

import signal
import subprocess
import sys


# ACPI_NAMESPACE_NODE.Name contains four ASCII bytes in native byte order.
panel = int.from_bytes(b"DD1F", sys.byteorder)
probe = f"""
fbt::AcpiEvQueueNotifyRequest:entry
/(arg1 == 0x86 || arg1 == 0x87) &&
 ((ACPI_NAMESPACE_NODE *)arg0)->Name.Integer == {panel}/
{{ printf("%d\\n", arg1); }}
"""

if subprocess.check_output(["/bin/kenv", "smbios.system.sku"]).strip() != b"B8MX7UA#ABL":
    sys.exit("Unsupported firmware SKU")

def stop(signum, frame):
    raise SystemExit(0)

signal.signal(signal.SIGTERM, stop)
with subprocess.Popen(
    ["/usr/bin/stdbuf", "-oL", "/usr/sbin/dtrace", "-x", "switchrate=10hz", "-qn", probe],
    stdout=subprocess.PIPE, text=True,
) as trace:
    try:
        for line in trace.stdout:
            direction = {"134": "incr", "135": "decr"}.get(line.strip())
            if direction:
                subprocess.run(["/usr/bin/backlight", "-q", "-f",
                                "/dev/backlight/intel_backlight0", direction, "5"], check=True)
        sys.exit(trace.wait())
    finally:
        trace.terminate()
