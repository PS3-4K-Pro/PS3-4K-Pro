#ifndef __XMBMP_SYSCALLS_H__
#define __XMBMP_SYSCALLS_H__
#include <ppu-lv2.h>
#include <lv2/sysfs.h>
#include <string>
#include <stdio.h>

#define SYSCALL8_OPCODE_MAP_PATHS			0x7964
#define SC_RING_BUZZER						(392)
#define CELL_OK 							0
#define FLASH_FLAGS							0x22ULL

using namespace std;

s32 lv2_get_platform_info(struct platform_info *info);
s32 sysFsMount(const char* MOUNT_POINT, const char* TYPE_OF_FILESYSTEM, const char* PATH_TO_MOUNT, int IF_READ_ONLY);
s32 sysFsUnmount(const char* PATH_TO_UNMOUNT);
u32 reboot_sys(string reboottype);
s32 lv2_get_target_type(uint64_t *type);
int is_dev_blind_mounted();
int mount_dev_blind();
int unmount_dev_blind();
int sys_map_path(char *oldpath, char *newpath);
int sys_map_paths(char *paths[], char *new_paths[], unsigned int num);
uint64_t lv1_peek(uint64_t addr);
uint8_t lv1_peek8(uint64_t addr);
uint32_t lv1_peek32(uint64_t addr);
string get_firmware_info(string what);
bool check_flash_type();
int lv2_ss_get_cache_of_flash_ext_flag(uint8_t *flag);
void setLed(const char *mode);
void single_beep();
void double_beep();
void triple_beep();

#endif // __XMBMP_SYSCALLS_H__
