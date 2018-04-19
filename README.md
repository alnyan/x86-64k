x86-64k is a 64-bit kernel for x86-based computers written in C++ (well, actually, with a bit of Assembly).

Building
========

Prerequisites
--------------------------

In order to build x86-64k iso image:

* Cross-compiler toolchain (`gcc`) for both i[3456]86 and x86-64 builds (tested with i686-elf and x86_64-elf)
* grub 2 utility package (for utilities like `grub-mkrescue`)

If you also want to emulate/debug x86-64k, `qemu` with x86-64 emulation support is required.

Make image.iso
--------------------

Once you've obtained all the stuff required from previous step, you can build x86-64k iso image:

```bash
    # Setup paths to toolchains
    $ export TARGET32=i686-elf
    $ export TARGET64=x86_64-elf
    $ export TOOLCHAIN32=/path/to/your/32-bit/toolchain
    $ export TOOLCHAIN64=/path/to/your/64-bit/toolchain
    $ make -j4 all
```

After running this, `build/image.iso` file should appear, and will be bootable by `qemu` or can be written to USB drive.

Features
--------

Here comes list of currently implemented and planned **basic** features:

- [x] It boots (yeah, that already was a hard task to get done)
- [x] 2MiB paging, dynamical mappings and structure allocation/freeing
- [x] Kernel heap
- [ ] Multitasking with processes and threads model
- [ ] Ring 3 a.k.a. userland
- [ ] Usermode ELF loading/execution
- [ ] Userspace library for kernel interaction (via system calls)

Contribution
------------

If you want to add some interesting stuff or have a nice idea to contribute - feel free to create issues with feature-requests or make pull-requests to our project.
You can also contact me directly via my e-mail.

