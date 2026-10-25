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

A later update of the system's Limine package may overwrite the file. On
CachyOS, see below; elsewhere, copy it again afterwards.

Keeping it across updates (CachyOS): `limine-install` runs after updates of
`limine` or `limine-mkinitcpio-hook`, and copies the package's stock
`BOOTX64.EFI` over yours. To make it install this build instead, keep the
build in a permanent place and point `limine-install` at it. Run these from the
Limine directory you built:

```
sudo install -Dm644 bin/BOOTX64.EFI /usr/local/share/limine-gamepad/BOOTX64.EFI
grep -qxF 'LIMINE_BINARY_PATH=/usr/local/share/limine-gamepad/BOOTX64.EFI' /etc/default/limine || echo 'LIMINE_BINARY_PATH=/usr/local/share/limine-gamepad/BOOTX64.EFI' | sudo tee -a /etc/default/limine
grep -qxF 'SKIP_LIMINE_VERSION_CHECK=yes' /etc/default/limine || echo 'SKIP_LIMINE_VERSION_CHECK=yes' | sudo tee -a /etc/default/limine
sudo limine-install --no-efi-register
```

`SKIP_LIMINE_VERSION_CHECK` is needed because this build carries no version
number that `limine-install` can read, so it would skip the install without
saying so. It also turns off the protection against installing an older
Limine. `--no-efi-register` leaves the UEFI boot entries alone, and the
fallback `/boot/EFI/BOOT/BOOTX64.EFI` is left unchanged.

Check that it worked; the two hashes must be identical:

```
sudo sha256sum /usr/local/share/limine-gamepad/BOOTX64.EFI /boot/EFI/limine/limine_x64.efi
```

After rebuilding the fork, run the first and last commands again. To go back
to the stock Limine:

```
sudo sed -i '/^LIMINE_BINARY_PATH=/d;/^SKIP_LIMINE_VERSION_CHECK=/d' /etc/default/limine
sudo env SKIP_LIMINE_VERSION_CHECK=yes limine-install --no-efi-register
```

## Going back

Restore the backup:

```
sudo cp /boot/EFI/limine/limine_x64.efi.bak /boot/EFI/limine/limine_x64.efi
```

If Limine no longer starts, restore the same file from the UEFI shell, from
Windows or from a live USB. Your firmware boot menu can still start other
entries meanwhile.
