#ifndef __XMBMP_SYSCALLS_H__
#define __XMBMP_SYSCALLS_H__
#include <ppu-lv2.h>
#include <lv2/sysfs.h>
#include <string>
#include <stdio.h>

using namespace std;

s32 lv2_get_platform_info(struct platform_info *info);
s32 sysFsMount(const char* MOUNT_POINT, const char* TYPE_OF_FILESYSTEM, const char* PATH_TO_MOUNT, int IF_READ_ONLY);
s32 sysFsUnmount(const char* PATH_TO_UNMOUNT);
u32 reboot_sys(string reboottype);
s32 lv2_get_target_type(uint64_t *type);
int is_dev_blind_mounted();
int mount_dev_blind();
int unmount_dev_blind();
string get_firmware_info(string what);

#endif // __XMBMP_SYSCALLS_H__
