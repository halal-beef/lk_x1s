![lk3rd](https://github.com/user-attachments/assets/32f57e56-811c-4590-a7f4-27b40d057dcb)

# lk3rd - A (3rd stage?!) bootloader for Exynos 990 devices.
And it gets even better...

## What?!
It's a lot of things.
* a bootloader (duh?)
* a hardware testing playground
* a fastboot interface
* a learning experience :)

## Let me install it!
NO. The current builds are not only unstable - they might hard brick your device easily. You are expected to know how to compile and package lk3rd to test it.
An user build will come in the near future.

## What devices does it run on?
* the canvas family
* the hubble family

## What is this commit history?!
We based lk3rd off of Linaro's source code for their Exynos 850 board, which contained code for Exynos 990 as well. Thus, the commit history is a mix of Linaro/Samsung/upstream.

## What works?
Well, not much. Booting, display (look ma, decon is up!!!), and USB. Regulators seem to initialize, and the MAX77705 seems to be charging the device. However,
we can't be too sure due to the lack of debug info. This should be fixed soon. The main breakage is UFS. Seems like it's UFS Protector that is not really working,
so even if the link is up (you can briefly see the storage device and capacity on boot!) the device outputs unfortunate garbage.

## Boot Sequence
You may be wondering, how the hell are you booting this? We can't replace our bootloaders!
Well, you're right, but we dont replace our bootloaders ;).
This flowchart of what happens will show you how we actually boot lk3rd.

```mermaid
flowchart LR
    A["Pre S-LK<br> (BootROM, BL1, BL2, etc)"] --> B["S-LK"]
    B --> C["Initialise Peripherals, Display and UFS"]
    C --> D["Boot from boot partition, which has had a boot image with a payload as kernel (uniLoader)"]
    D --> E["uniLoader sets regs to the kernel (lk3rd) address, sets the decon hardware registers and copies our kernel to 0x90000000 (our base)"]
    E --> F["uniLoader branches out to the base address, launching lk3rd"]
    F --> G["lk3rd sets up Peripherals, USB and UFS, but doesn't use the display, instead using the framebuffer"]
    G --> H["Framebuffer is cleared once again, logs and then the fastboot text appears, when fastboot is launched, ready to take commands from the user"]
```

Cheers from the Exynos990-Mainline team!
