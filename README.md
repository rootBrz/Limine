# Limine [![Matrix Server](https://img.shields.io/matrix/limine:matrix.org?color=000000&label=Matrix&logo=matrix)](https://matrix.to/#/#limine:matrix.org)

<p align="center">
    <img src="https://github.com/Limine-Bootloader/Limine/blob/trunk/logo.png?raw=true" alt="Limine's logo"/>
</p>

### About this fork

This is an unofficial fork of [Limine](https://github.com/Limine-Bootloader/Limine).
It is not supported by the Limine project; please do not report its problems
there. It adds USB gamepad support to the UEFI boot menu, written for the ASUS
ROG Ally. The `uefi-gamepad` branch is the one to use. Details, supported
controllers and limits are in [GAMEPAD.md](GAMEPAD.md).

To build and install it on an Arch based system such as CachyOS with a UEFI
x86-64 machine, run these commands in a terminal, one block at a time.

Build:
```
sudo pacman -Syu --needed base-devel git clang lld llvm nasm autoconf automake
git clone --depth 1 -b uefi-gamepad https://github.com/rootBrz/Limine.git
cd Limine
./bootstrap && ./configure --enable-uefi-x86-64 --disable-uefi-cd && make
```

Install (this backs up the current file once, then replaces it):
```
ls /boot/EFI/limine/limine_x64.efi && sudo cp -n /boot/EFI/limine/limine_x64.efi /boot/EFI/limine/limine_x64.efi.bak && sudo cp bin/BOOTX64.EFI /boot/EFI/limine/limine_x64.efi && sync && echo INSTALLED
```

If it prints `No such file`, nothing was changed: your system keeps Limine
somewhere else (see `efibootmgr -v`). Do not use these commands with Secure
Boot or an enrolled config checksum.

To go back:
```
sudo cp /boot/EFI/limine/limine_x64.efi.bak /boot/EFI/limine/limine_x64.efi
```

A system update of the Limine package may overwrite the installed file; run
the install block again afterwards.

#### Findings behind this fork

Measured on an ASUS ROG Ally (RC71L) with Limine 12.9.4. Upstream issue:
[#644](https://github.com/Limine-Bootloader/Limine/issues/644).

* Limine only uses input the firmware already exposes: text input and the
  pointer protocols. Nothing reads USB gamepads.
* On a cold boot the firmware exposes no gamepad input. The only absolute
  pointer is the console splitter's own aggregate handle, with no device
  behind it.
* The controller is on the USB bus on a cold boot (`045e:028e`, an Xbox 360
  class pad), alongside the keyboard-class `0b05:1abe`, Bluetooth `0489:e0f5`
  and `2808:a753`.
* After entering the BIOS setup once, the firmware adds a second absolute
  pointer with a vendor-defined, non-USB device path. Stock Limine then works
  with the controls. I do not know what that pointer belongs to.
* Connecting all controllers recursively before the menu does not create it.
* So this fork reads the pad directly over the firmware's USB I/O protocol,
  which works on a cold boot without visiting the BIOS.

Not tested: the Ally X, or any other device.

### What is Limine?

Limine (pronounced as demonstrated [here](https://www.merriam-webster.com/dictionary/in%20limine))
is a modern, secure, portable, multiprotocol bootloader and boot manager, also used
as the reference implementation for the [Limine boot protocol](https://github.com/Limine-Bootloader/limine-protocol/blob/trunk/PROTOCOL.md).

### Community and Support

#### Community
We have a Matrix room at [`#limine:matrix.org`](https://matrix.to/#/#limine:matrix.org) if you need support, info, or you just want to hang out with us.

### Limine's boot menu

![Reference screenshot](screenshot.png?raw=true "Reference screenshot")

[Photo by Levent Simsek](https://www.pexels.com/photo/brown-tabby-cat-in-close-up-photography-3617160/)

### Supported architectures
* IA-32 (32-bit x86)
* x86-64
* aarch64 (arm64)
* riscv64
* loongarch64

### Supported boot protocols
* Linux
* [Limine](https://github.com/Limine-Bootloader/limine-protocol/blob/trunk/PROTOCOL.md)
* Multiboot 1
* Multiboot 2
* Chainloading

### Supported partitioning schemes
* MBR
* GPT
* Unpartitioned media

### Supported filesystems
* FAT12/16/32
* ISO9660 (CDs/DVDs)

If your filesystem isn't listed here, please read [the FAQ](FAQ.md) first, especially before
opening issues or pull requests related to this.

### Minimum system requirements
For 32-bit x86 systems, support is only ensured starting with those with
Pentium Pro (i686) class CPUs.

All x86-64, aarch64, riscv64 and loongarch64 (UEFI) systems are supported.

## Packaging status

All Limine releases since 7.x use [Semantic Versioning](https://semver.org/spec/v2.0.0.html) for their naming.

[![Packaging status](https://repology.org/badge/vertical-allrepos/limine.svg?columns=3)](https://repology.org/project/limine/versions)

## Binary releases

For convenience, for point releases, binaries are distributed. These binaries
are shipped as assets as part of the
[Limine GitHub releases](https://github.com/Limine-Bootloader/Limine/releases)
(see the `limine-binary-*` files).

The `limine` host tool is shipped in highly portable source form as part of the
binary release package. For most/all UNIX-like OSes, in order to rebuild it,
simply run `make` in the unpacked binary release directory. Alternatively, it
can be built stand-alone using any C99 compatible compiler.

`limine` host tool binaries for x86 Windows are provided as part of the binary
release package.

## Build and Install Instructions

*The following steps are not necessary if using a binary release.*

See [INSTALL.md](INSTALL.md).

## Usage

See [USAGE.md](USAGE.md).

## 3rd Party Software Acknowledgments

See [3RDPARTY.md](3RDPARTY.md).
