#!/bin/sh
set -eu
src=$(realpath "$(dirname "$0")")
out=$(pwd)
stage=$(mktemp -d)
trap 'rm -rf "$stage"' EXIT
prefix=$stage/usr/local
mkdir -p "$prefix/libexec" "$prefix/etc/xdg/autostart"
cc -O2 -g -Wall -Wextra $(pkg-config --cflags libpulse) \
    "$src/omnibook-micmute.c" -o "$prefix/libexec/omnibook-micmute" \
    $(pkg-config --libs libpulse)
install -m 0444 "$src/omnibook-micmute.desktop" "$prefix/etc/xdg/autostart/"
version=$(pkg query '%v' pulseaudio)
cat > "$stage/manifest" <<MANIFEST
name: omnibook-micmute
version: "0.1"
origin: local/omnibook-micmute
comment: Synchronize the HP OmniBook microphone LED with PulseAudio
desc: GNOME session client for the HPBSD microphone indicator.
maintainer: local@localhost
www: https://github.com/denislemire/omnibook-hotkeys
prefix: /usr/local
deps: {pulseaudio: {origin: audio/pulseaudio, version: "$version"}}
files: {
 "/usr/local/libexec/omnibook-micmute": "-",
 "/usr/local/etc/xdg/autostart/omnibook-micmute.desktop": "-"
}
MANIFEST
pkg create -M "$stage/manifest" -r "$stage" -o "$out"
