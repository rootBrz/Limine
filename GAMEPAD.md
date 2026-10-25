# Gamepad support (UEFI)

This fork adds a small USB gamepad driver to the UEFI boot menu. It reads a
wired Xbox 360 class controller directly over the firmware's USB I/O
protocol and turns button presses into menu keys. It was written for the
ASUS ROG Ally, whose built-in controller shows up as USB `045e:028e`.

## Status

- Tested on one ROG Ally (RC71L, not the Ally X): the controls work on a
  direct cold boot into Limine.
- Without this driver, the same device only gets working controls in Limine
  after a visit to the BIOS setup. The firmware only exposes a second
  absolute pointer then.
- Untested on any other device.

## Supported controllers

Any USB device with an interface of class `0xff`, subclass `0x5d`, protocol
`0x01`. That is the wired Xbox 360 controller protocol, used by genuine
Xbox 360 pads and many clones and handheld built-in pads.

Not supported: Xbox One and Series pads (they use a different protocol),
the Xbox 360 wireless receiver, Bluetooth pads, and generic HID gamepads.
Only the first matching controller is used.

To check a device from Linux:

```
lsusb -v 2>/dev/null | grep -B8 -A3 'bInterfaceSubClass *93'
```

A match shows `bInterfaceClass 255`, `bInterfaceSubClass 93` and
`bInterfaceProtocol 1`.

## Buttons

| Button  | Action                   |
|---------|--------------------------|
| D-pad   | Move selection           |
| A       | Boot the entry           |
| Start   | Boot the entry           |
| B       | Escape                   |
| X       | Edit the entry           |
| LB / RB | Page up / page down      |

Held buttons do not repeat.

To turn the feature off, add `gamepad: no` to `limine.conf`.

## Building

You need `git`, `clang` with `lld` (or `gcc` with binutils), `nasm`, GNU
`make`, `autoconf` and `automake`. On Arch based systems such as CachyOS:

```
sudo pacman -Syu --needed base-devel git clang lld llvm nasm autoconf automake
```

Then:

```
git clone --depth 1 -b uefi-gamepad https://github.com/rootBrz/Limine.git
cd Limine
./bootstrap
./configure --enable-uefi-x86-64 --disable-uefi-cd
make
```

The result is `bin/BOOTX64.EFI`.

## Installing

Find where your system keeps Limine. On CachyOS it is usually
`/boot/EFI/limine/limine_x64.efi`; check with `efibootmgr -v` if unsure.

Back up the old file first (the backup is only made once, so repeating the
step keeps your original), then replace it:

```
sudo cp -n /boot/EFI/limine/limine_x64.efi /boot/EFI/limine/limine_x64.efi.bak
sudo cp bin/BOOTX64.EFI /boot/EFI/limine/limine_x64.efi
sync
```

If you use Secure Boot or `limine enroll-config`, a plain copy will lose
the signature or the enrolled config checksum. Do not follow these steps in
that case.

A later update of the system's Limine package may overwrite the file. Copy
it again afterwards.

Optional, Arch based systems only: stop `pacman` upgrading the `limine`
package, so an update cannot overwrite your build. This adds `IgnorePkg =
limine` to `/etc/pacman.conf` (running it twice is harmless):

```
grep -q '^IgnorePkg = limine$' /etc/pacman.conf || sudo sed -i '/^\[options\]/a IgnorePkg = limine' /etc/pacman.conf
```

While it is set, you get no updates or security fixes for the `limine`
package. Another package's hook may still replace the file; this only stops
`pacman` upgrading `limine` itself. To undo it:

```
sudo sed -i '/^IgnorePkg = limine$/d' /etc/pacman.conf
```

## Going back

Restore the backup:

```
sudo cp /boot/EFI/limine/limine_x64.efi.bak /boot/EFI/limine/limine_x64.efi
```

If Limine no longer starts, restore the same file from the UEFI shell, from
Windows or from a live USB. Your firmware boot menu can still start other
entries meanwhile.
