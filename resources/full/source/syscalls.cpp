#include "syscalls.h"
#include "filesystem.h"
#include <cstring>
#include <sstream>

struct platform_info
{
	uint32_t firmare_version; //Ex: 0x03055000
	uint8_t res1[4];
	uint64_t platform_id;
	uint32_t unknown;
	uint8_t res2[4];
};

s32 lv2_get_platform_info(struct platform_info *info)
{
	lv2syscall1(387, (uint64_t) info);
	return_to_user_prog(s32);
}

__attribute__((noinline)) uint64_t lv1_peek(uint64_t addr)
{
   lv2syscall1(8, (uint64_t)addr);
   return_to_user_prog(uint64_t);
}

uint8_t lv1_peek8(uint64_t addr) 
{
	return (lv1_peek(addr) >> 56) & 0xFFUL;
}

uint32_t lv1_peek32(uint64_t addr) 
{
	return (lv1_peek(addr) >> 32) & 0xFFFFFFFFUL;
}

s32 sysFsMount(const char* MOUNT_POINT, const char* TYPE_OF_FILESYSTEM, const char* PATH_TO_MOUNT, int IF_READ_ONLY)
{
	lv2syscall8(837, (u64)MOUNT_POINT, (u64)TYPE_OF_FILESYSTEM, (u64)PATH_TO_MOUNT, 0, IF_READ_ONLY, 0, 0, 0);
	return_to_user_prog(s32);
}

s32 sysFsUnmount(const char* PATH_TO_UNMOUNT)
{
	lv2syscall1(838, (u64)PATH_TO_UNMOUNT);
	return_to_user_prog(s32);
}

u32 reboot_sys(string reboottype)
{
	uint16_t op;

	if (reboottype=="soft") op=0x200;
	else op=0x1200;
	lv2syscall4(379,op,0,0,0);
	return_to_user_prog(u32);
}

s32 lv2_get_target_type(uint64_t *type)
{
	lv2syscall1(985, (uint64_t) type);
	return_to_user_prog(s32);
}

int is_dev_blind_mounted()
{
	const char* MOUNT_POINT = "/dev_blind"; //our mount point
	sysFSStat dir;

	return sysFsStat(MOUNT_POINT, &dir);
}

int mount_dev_blind()
{
	const char* DEV_BLIND = "CELL_FS_IOS:BUILTIN_FLSH1";	// dev_flash
	const char* FAT = "CELL_FS_FAT"; //it's also for fat32
	const char* MOUNT_POINT = "/dev_blind"; //our mount point

	sysFsMount(DEV_BLIND, FAT, MOUNT_POINT, 0);

	return 0;
}

int unmount_dev_blind()
{
	const char* MOUNT_POINT = "/dev_blind"; //our mount point

	sysFsUnmount(MOUNT_POINT);

	return 0;
}

int sys_map_path(char *oldpath, char *newpath)
{
#if 1
	lv2syscall2(35, (u64)oldpath, (u64)newpath);
#else
	char *paths[1]={NULL}; char *new_paths[1]={NULL};
	paths[0]=oldpath;new_paths[0]=newpath;
	lv2syscall4(8, SYSCALL8_OPCODE_MAP_PATHS, (u64)paths, (u64)new_paths, 1);
#endif
	return_to_user_prog(s32);
}

int sys_map_paths(char *paths[], char *new_paths[], unsigned int num)
{
	lv2syscall4(8, SYSCALL8_OPCODE_MAP_PATHS, (u64)paths, (u64)new_paths, num);
	return_to_user_prog(s32);
}

bool is_hen(void)
{
	lv2syscall1(8, 0x1337); 
	return (int)0x1337 == (int)p1;
}


bool is_mamba(void)
{
	lv2syscall1(8, 0x7fff); 
	return (int)0x666 == (int)p1;
}

int get_payload_version(uint16_t *version)
{
	if(version==0)
	return 0x8001000D;
	lv2syscall2(8, 0x7001, (uint64_t)version);
	return (int)p1;
}

bool is_cobra(void){
	uint16_t ver=0;
	return get_payload_version(&ver)==0 ? ver>0 ? true : false : false;
}

bool is_cfw(void)
{
	lv2syscall1(6, 0x8000000000000000ULL); 
	return (int)0 == (int)p1;
}

s32 get_hen_version()
{
	lv2syscall1(8, (uint64_t) 0x1339);
	return_to_user_prog(s32);
}


string find_clock_speeds()
{
    string result = "";
    char rsxData[120];
    char core[25], memory[25];
    u8 hex[8];
    u64 rsx_values = 0;

    for (u64 offset = 0x600000; offset < 0x700000; offset++)
    {
        if (lv1_peek32(offset) == 0x7670653A &&
            (lv1_peek32(offset + 7) == 0x7368643A || lv1_peek32(offset + 8) == 0x7368643A) &&
            lv1_peek8(offset - 5) == 0x2F)
        {
            rsx_values = lv1_peek(offset - 8);
            break;
        }
    }

    memcpy(hex, &rsx_values, 8);

    for (int i = 0; i < 8; i++)
        snprintf(&rsxData[i], sizeof(rsxData), "%c", hex[i]);

    strncpy(core, rsxData, 3);
    strncpy(memory, rsxData + 4, 3);
	
	int gpuClock = atoi(core);
    int memoryClock = atoi(memory);

    char buf[100];
    if (gpuClock > 500 || memoryClock > 650)
    {
        snprintf(buf, sizeof(buf), "Overclock %d/%d MHz", gpuClock, memoryClock);
    }
    else if (gpuClock == 500 && memoryClock == 650)
    {
        snprintf(buf, sizeof(buf), "Standard %d/%d MHz", gpuClock, memoryClock);
    }

     result = (string)buf;

    return result;
}


string get_firmware_info(string what)
{
	string result="";

	if (what=="version")
	{
		char buf[50];
		struct platform_info info;
		lv2_get_platform_info(&info);
		sprintf(buf, "%02X", info.firmare_version);
		result=(string)buf;
		result.replace(result.find("0"),1,".");
		result.erase(result.find("000"),3);
	}
	else if (what=="type")
	{
		uint64_t targettype;
		lv2_get_target_type(&targettype);
		if (targettype==1 && (exists("/dev_flash/vsh/module/vsh.self.cexsp")==0 || exists("/dev_flash/vsh/module/vsh.self.dexsp")==0 || exists("/dev_flash/vsh/module/vsh.self.swp")==0 || exists("/dev_flash/vsh/module/vsh.self.nrm")==0)) result="REX";
		else if (targettype==1 && (exists("/dev_flash/vsh/module/vsh.self.cex")==0 || exists("/dev_flash/vsh/module/vsh.self.dex")==0)) result="PEX";
		else if (targettype==1) result="CEX";
		else if (targettype==2 && (exists("/dev_flash/vsh/module/vsh.self.cexsp")==0 || exists("/dev_flash/vsh/module/vsh.self.dexsp")==0 || exists("/dev_flash/vsh/module/vsh.self.swp")==0 || exists("/dev_flash/vsh/module/vsh.self.nrm")==0)) result="D-REX";
		else if (targettype==2 && (exists("/dev_flash/vsh/module/vsh.self.cex")==0 || exists("/dev_flash/vsh/module/vsh.self.dex")==0)) result="D-PEX";
		else if (targettype==2) result="DEX";
		else if (targettype==3) result="DECR";
		else result="Unknown";
	}
	else if (what=="jailbreak")
	{
		bool  HEN=is_hen();
        bool  CFW =is_cfw();
		if (HEN) result = "Hybrid Firmware";
        else if (CFW) result = "Custom Firmware";
        else result = "Official Firmware";
    }
	else if (what=="payload")
	{
		bool  MAMBA=is_mamba();
        bool  COBRA= is_cobra();
        if (MAMBA) result = "Mamba";
        else if (COBRA) result = "Cobra";
		else result = "no";
    }	
	else if (what == "payload_version")
	{   
		char buf[50];
		uint16_t ver = 0;
		get_payload_version(&ver);
    
		if (ver > 0) 
		{
			int digit1 = (ver >> 8) & 0xF;
			int digit2 = (ver >> 4) & 0xF;
			sprintf(buf, " v%X.%X", digit1, digit2);
			result = (string)buf;
		}
    else
		{
			result = "";
		}
	}
	else if (what == "gpu_clock_speed")
    {
        const char* clockSpeeds = find_clock_speeds().c_str(); // Convert to const char*
        result = clockSpeeds;
    }
	else if (what=="hen_version")
	{	
		char buf[50];
		s32 hen_version = get_hen_version();
		int digit1 = (hen_version >> 8) & 0xF;
		int digit2 = (hen_version >> 4) & 0xF;
		int digit3 = hen_version & 0xF;
		sprintf(buf, "%X.%X.%X", digit1, digit2, digit3);
		result = (string)buf;
	}
	
	return result;
}
