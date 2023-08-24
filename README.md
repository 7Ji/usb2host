# usb2host
A tiny program to put dual-role USB port to host mode

## Build
```
make
```
The output binary would be `usb2host`

## Usage
```
./usb2host [node 1] [node 2] ...
```
`[node]` is the canoncical device-tree node name of a USB dual-role controller used by kernel under sysfs

The program needs write permission to files under `/sys/kernel/debug/usb`, so you need to run it with root permission.

e.g. `fc000000.usb` is the node for RK3588(S), so for SBCs using RK3588(S) e.g. OrangePi 5/5B/5+, you should use the following command:
```
./usb2host fc000000.usb
```

If no nodes are given, it would search through all available usb debug folders to put all dual-roal usb controllers (those with `mode` file under their debug folder) into host mode. Usually you don't want this as there would be only one dual-role ports for a single device, which can be named explicitly.

## Why
The current ways used in distros to force a dual-role usb controller into host mode uses shell:
```
echo host > /sys/kernel/debug/usb/fc000000.usb/mode
```
As `>` is pure shell syntax, if you want to integrate the above command into a udev rule, or a systemd `.service`, you'll need to write it like this:
```
ACTION=="remove", SUBSYSTEM=="typec", RUN+="/usr/bin/sh -c 'echo host > /sys/kernel/debug/usb/fc000000.usb/mode'"
```
```
ExecStart=/usr/bin/sh -c 'echo host > /sys/kernel/debug/usb/fc000000.usb/mode'
```
Starting a full-blown shell just for the sake of writing 4 bytes is very heavy and slows down the bootup speed.

The biggest problem is that it **also opens up vulnerability to shell injection and coupliing with shell**. This is no different from using `system()` in C or `os.system()` or `subprocess.call([], shell=True)` in Python.

Instead, using this tool, the above examples would become:
```
ACTION=="remove", SUBSYSTEM=="typec", RUN+="/usr/bin/usb2host fc000000.usb"
```

```
ExecStart=/usr/bin/usb2host fc000000.usb
```