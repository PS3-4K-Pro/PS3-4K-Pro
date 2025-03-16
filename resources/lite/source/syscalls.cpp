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

//Beeps
void single_beep() 
{ 
	lv2syscall3(SC_RING_BUZZER, 0x1004, 0x4, 0x6);
}

void double_beep()
{
	lv2syscall3(SC_RING_BUZZER, 0x1004, 0x7, 0x36);
}

void triple_beep()
{
	lv2syscall3(SC_RING_BUZZER, 0x1004, 0xa, 0x1b6);
}

//Leds
int sys_sm_control_led(uint8_t led_id,uint8_t led_action)
{ 	
	lv2syscall2(386, (uint64_t)led_id,(uint64_t)led_action);
	return_to_user_prog(int);
}

void setLed(const char *mode)
{
	if(strcmp(mode, "off") == 0)
	{
		sys_sm_control_led(2, 0); 
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
	}
	else if(strcmp(mode, "red_default") == 0)
	{
		sys_sm_control_led(2, 0);		
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(2, 1);
	}
	else if(strcmp(mode, "red_blink_slow") == 0)
	{
		sys_sm_control_led(2, 0);		
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(2, 3);
	}
	else if(strcmp(mode, "red_blink_fast") == 0)
	{
		sys_sm_control_led(2, 0);		
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(2, 2);
	}
	else if(strcmp(mode, "yellow_default") == 0)
	{
		sys_sm_control_led(2, 0);
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(1, 1);
		sys_sm_control_led(2, 1);
	}
	else if(strcmp(mode, "yellow_blink_slow") == 0)
	{
		sys_sm_control_led(2, 0);
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(2, 3);
		sys_sm_control_led(1, 3);
	}
	else if(strcmp(mode, "yellow_blink_fast") == 0)
	{
		sys_sm_control_led(2, 0);
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(2, 2);
		sys_sm_control_led(1, 2);
	}
	else if(strcmp(mode, "yellow_green_blink_slow") == 0)
	{
		sys_sm_control_led(2, 0);
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(1, 1);
		sys_sm_control_led(2, 3);
	}
	else if(strcmp(mode, "yellow_green_blink_fast") == 0)
	{
		sys_sm_control_led(2, 0);
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(1, 1);
		sys_sm_control_led(2, 2);
	}
	else if(strcmp(mode, "yellow_red_blink_slow") == 0)
	{
		sys_sm_control_led(2, 0);
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(2, 1);
		sys_sm_control_led(1, 3);
	}
	else if(strcmp(mode, "yellow_red_blink_fast") == 0)
	{
		sys_sm_control_led(2, 0);
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(2, 1);
		sys_sm_control_led(1, 2);
	}
	else if(strcmp(mode, "green_default") == 0)
	{
		sys_sm_control_led(2, 0);
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(1, 1);
	}
	else if(strcmp(mode, "green_blink_slow") == 0)
	{
		sys_sm_control_led(2, 0);
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(1, 3);
	}
	else if(strcmp(mode, "green_blink_fast") == 0)
	{
		sys_sm_control_led(2, 0);
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(1, 2);
	}
	else if(strcmp(mode, "rainbow") == 0)
	{
		sys_sm_control_led(2, 0); 
		sys_sm_control_led(1, 0);
		sys_sm_control_led(0, 0);
		sys_sm_control_led(2, 2);
		usleep(250000);
		sys_sm_control_led(1, 2);
	}
	else if(strcmp(mode, "special_1") == 0)
	{
		sys_sm_control_led(2, 0); 
		sys_sm_control_led(1, 0); 
		sys_sm_control_led(0, 0); 
		sys_sm_control_led(1, 2);
		sys_sm_control_led(2, 3); 
	}
	else if(strcmp(mode, "special_2") == 0)
	{
		sys_sm_control_led(2, 0); 
		sys_sm_control_led(1, 0); 
		sys_sm_control_led(0, 0); 
		sys_sm_control_led(1, 3); 
		sys_sm_control_led(2, 2);
	}
}

string find_clock_speeds()
{
    char rsxData[120];
	uint32_t core_val = 0;
	uint32_t rsx_val = 0;
    uint8_t hex[8];
    int mode = 0;
    char core[5];
    char memory[5];
    for (uint64_t offset = 0x600000; offset < 0x700000; offset++)
    {
        if (lv1_peek32(offset) == 0x7670653A && (lv1_peek32(offset + 7) == 0x7368643A || lv1_peek32(offset + 8) == 0x7368643A))
        {
            if (lv1_peek8(offset - 5) == 0x2F || lv1_peek8(offset - 6) == 0x2F)
            {
                if (lv1_peek8(offset - 10) == 0x20 && lv1_peek8(offset - 6) == 0x2F && lv1_peek8(offset - 1) == 0x20)
                {
                    mode = 2;
                    core_val = lv1_peek32(offset - 9);
                    rsx_val = lv1_peek32(offset - 5);
                    break;
                }
                else if (lv1_peek8(offset - 10) == 0x20 && lv1_peek8(offset - 5) == 0x2F && lv1_peek8(offset - 1) == 0x20)
                {
                    mode = 4;
                    core_val = lv1_peek32(offset - 9);
                    rsx_val = lv1_peek32(offset - 4);
                    break;
                }
                else if (lv1_peek8(offset - 11) == 0x20 && lv1_peek8(offset - 1) == 0x20 && lv1_peek8(offset - 6) == 0x2F)
                {
                    mode = 3;
                    core_val = lv1_peek32(offset - 10);
                    rsx_val = lv1_peek32(offset - 5);
                    break;
                }
                else
                {
                    mode = 1;
                    core_val = lv1_peek32(offset - 8);
                    rsx_val = lv1_peek32(offset - 4);
                    break;
                }
            }
        }
    }
    char buf[100];
    if (core_val > 0 && rsx_val > 0) 
	{
        memcpy(hex, &core_val, 4);
        memcpy(hex + 4, &rsx_val, 4);
        for (int i = 0; i < 8; i++)
        {
            sprintf(&rsxData[i], "%c", hex[i]);
        }
        strncpy(core, rsxData, (mode == 3 ? 4 : mode == 4 ? 4 : 3));
        strncpy(memory, rsxData + 4, (mode == 2 ? 4 : mode == 3 ? 4 : 3));
        int gpuClock = atoi(core);
        int memoryClock = atoi(memory);
        if (gpuClock > 500 || memoryClock > 650)
        {
            snprintf(buf, sizeof(buf), "Overclock %d/%d MHz", gpuClock, memoryClock);
        }
        else if (gpuClock == 500 && memoryClock == 650)
        {
            snprintf(buf, sizeof(buf), "Standard %d/%d MHz", gpuClock, memoryClock);
        }
       else if (gpuClock  < 500 || memoryClock < 650)
		{
			snprintf(buf, sizeof(buf), "Underclock %d/%d MHz", gpuClock, memoryClock);
		}
    }
	else 
	{
		snprintf(buf, sizeof(buf), "Error: Unable to get clock speeds");
	}
	return (string)buf;
}

static inline s32 sys_storage_read(int fd, uint64_t start_sector, uint64_t nb_sector, const void* buffer, uint32_t *sectors_read, uint64_t flags )
{
	lv2syscall7( 602, fd, 0, start_sector, nb_sector, (uint64_t)buffer, (uint64_t)sectors_read, flags );
    return_to_user_prog(int);
}

string get_metldr_version(const char* item_name, int flash_id, uint64_t metldr_sec, void* read_buf) {
	string result = "Unknown";
    uint32_t sectors_read;
    
    sys_storage_read(flash_id, 0, metldr_sec, read_buf, &sectors_read, FLASH_FLAGS);
    
    if (strcmp(item_name, "metldr") == 0)
	{
        result = "metldr";
    }
	else if (strcmp(item_name, "metldr.2") == 0)
	{
        result = "metldr.2";
    }
	else
	{
        result = "Wrong metldr version";
    }

    return result;
}

int lv2_ss_get_cache_of_flash_ext_flag(uint8_t *flag)
{
	lv2syscall1(874, (uint64_t) flag);
	return_to_user_prog(int);
}

bool check_flash_type()
{
	uint8_t flag;
	lv2_ss_get_cache_of_flash_ext_flag(&flag);
	return !(flag & 0x1);
}

static uint32_t GetApplicableVersion(void * data)
{
	lv2syscall8(863, 0x6011, 1, (uint64_t)data, 0, 0, 0, 0, 0);
	return_to_user_prog(uint32_t);
}
// Not working for some reason
const char* applicable_version()
{
    static char result[256];
    uint8_t data[0x20];
    memset(data, 0, sizeof(data));

    int ret = GetApplicableVersion(data);
    if (ret != CELL_OK)
    {
        snprintf(result, sizeof(result), "Error: Applicable version failed, return code: %d", ret);
        return result;
    }

    snprintf(result, sizeof(result), "%x.%02x", data[1], data[3]);
    return result;
}

int sys_ss_appliance_info_manager_get_ps_code(uint8_t *pscode)
{
	lv2syscall2(867, (uint64_t)0x19004, (uint64_t)pscode);
	return_to_user_prog(int);
}

string get_console_model() {
    string result = "Unknown";
    uint8_t pscode[8] = {0};

    int ret = sys_ss_appliance_info_manager_get_ps_code(pscode);
    if (ret != CELL_OK) {
        return "Error: AIM syscall failed";
    }

    uint8_t sub_code = pscode[5];

    switch (sub_code) {
        // FAT Models
		case 0x01: // Model CECHAxx - Motherboard COK-001
		case 0x02: // Model CECHBxx - Motherboard COK-002MC
		case 0x03: // Model CECHCxx - Motherboard COK-002
		case 0x04: // Model CECHExx - Motherboard COK-002W
		case 0x05: // Model CECHGxx - Motherboard SEM-001
		case 0x06: // Model CECHHxx - Motherboard DIA-001
		case 0x07: // Model CECHJ/Kxx - Motherboard DIA-002
		case 0x08: // Model CECHL/M/P/Qxx - Motherboard VER-001
		
			result = "Fat";
			break;
		
		// SLIM Models
		case 0x09: // Model CECH-20xxA/B - Motherboard DYN-001
		case 0x0A: // Model CECH-21xxA/B - Motherboard SUR-001
		case 0x0B: // Model CECH-25xxA/B - Motherboard JTP-001 / JSP-001
		case 0x0C: // Model CECH-30xxA/B - Motherboard KTE-001
		
			result = "Slim";
			break;
		
		// Super Slim Models
		case 0x0D: // Model CECH-40xxB/C v1 - Motherboard MSX-001
		case 0x0E: // Model CECH-40xxA v1 - Motherboard MPX-001
		case 0x0F: // Model CECH-40xxB/C v2 - Motherboard Unknown
		case 0x10: // Model CECH-40xxA v2 - Motherboard Unknown
		case 0x11: // Model CECH-42xxB/C - Motherboard NPX-001
		case 0x12: // Model CECH-42xxA - Motherboard PPX-001 / PQX-001
		case 0x13: // Model CECH-43xxB/C - Motherboard RTX-001
		case 0x14: // Model CECH-43xxA  - Motherboard REX-001
        
            result = "Super Slim";
            break;

        default:
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "Unknown (0x%02X)", sub_code);
            result = buffer;
            break;
    }

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
		if (targettype==1 && exists("/dev_flash/rebug")==0 && (exists("/dev_flash/vsh/module/vsh.self.cexsp")==0 || exists("/dev_flash/vsh/module/vsh.self.dexsp")==0 || exists("/dev_flash/vsh/module/vsh.self.swp")==0 || exists("/dev_flash/vsh/module/vsh.self.nrm")==0 || exists("/dev_flash/vsh/module/vsh.self.cex")==0 || exists("/dev_flash/vsh/module/vsh.self.dex")==0)) result="REX";
		else if (targettype==1 && (exists("/dev_flash/vsh/module/vsh.self.cex")==0 || exists("/dev_flash/vsh/module/vsh.self.dex")==0)) result="PEX";
		else if (targettype==1) result="CEX";
		else if (targettype==2 && exists("/dev_flash/rebug")==0 && (exists("/dev_flash/vsh/module/vsh.self.cexsp")==0 || exists("/dev_flash/vsh/module/vsh.self.dexsp")==0 || exists("/dev_flash/vsh/module/vsh.self.swp")==0 || exists("/dev_flash/vsh/module/vsh.self.nrm")==0 || exists("/dev_flash/vsh/module/vsh.self.cex")==0 || exists("/dev_flash/vsh/module/vsh.self.dex")==0)) result="D-REX";
		else if (targettype==2 && (exists("/dev_flash/vsh/module/vsh.self.cex")==0 || exists("/dev_flash/vsh/module/vsh.self.dex")==0)) result="D-PEX";
		else if (targettype==2) result="DEX";
		else if (targettype==3) result="DECR";
		else result="Unknown";
	}
	else if (what == "applicable_version")
	{
		result = applicable_version();
	}
	else if (what=="console_model")
	{
		result = get_console_model();
	}
    else if (what == "flash_type")
	{	
		if (check_flash_type()) 
			result = "NOR";
		else 
			result = "NAND";
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
	else if (what == "metldr_version") 
	{
		int flash_id = 0;
		uint64_t metldr_sec = 0;
		void* read_buf = malloc(512);
		
		if (read_buf) {
			result = get_metldr_version("metldr", flash_id, metldr_sec, read_buf);
			free(read_buf);
		} else {
			result = "Error: Memory allocation failed";
		}
	}
	else if (what == "gpu_clock_speed")
    {
        const char* clockSpeeds = find_clock_speeds().c_str(); // Convert to const char*
        result = clockSpeeds;
    }
	return result;
}
