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

Cheers from the Exynos990-Mainline team!
