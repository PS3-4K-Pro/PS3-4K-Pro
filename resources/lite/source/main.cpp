#include "filesystem.h"
#include "syscalls.h"
#include "graphics.h"
//#include "xmbmp-debug.h"
#include <io/pad.h>
#include <time.h>
#include <zlib.h>

#define MAX_OPTIONS 20

//global vars
string menu1[MAX_OPTIONS];
string menu2[MAX_OPTIONS][MAX_OPTIONS];
string menu2_path[MAX_OPTIONS][MAX_OPTIONS];
string menu3[MAX_OPTIONS];
string jailbreak=get_firmware_info("jailbreak");
string payload=get_firmware_info("payload");
string payload_version=get_firmware_info("payload_version");
string hen_version=get_firmware_info("hen_version");
string gpu_clock_speed=get_firmware_info("gpu_clock_speed");
string console_model=get_firmware_info("console_model");

//headers
const string currentDateTime();
const string buildNumber();
int restore(string appfolder, string foldername);
int install(string appfolder, string firmware_folder, string app_choice);
int delete_all(string appfolder);
int delete_one(string appfolder, string foldername, string type);
int make_menu_to_array(string appfolder, int whatmenu, string vers, string type);
void bitmap_menu(string fw_version, string ttype, int menu_id, int msize, int selected, int choosed, int menu1_pos, int menu1_restore);

#define BUFFER_SIZE 1024


s32 main(s32 argc, char* argv[]);

const string currentDateTime() {
    const char* filePath = ("/dev_hdd0/game/"+ string(APP_TITLEID) + "/USRDIR/data/version").c_str();

    s32 fd;
    int ret = sysFsOpen(filePath, SYS_O_RDONLY, &fd, NULL, 0);
    if (ret == 0) {
        char buffer[BUFFER_SIZE];
        uint64_t bytesRead;
        ret = sysFsRead(fd, buffer, BUFFER_SIZE - 1, &bytesRead);
        if (ret == 0) {
            
			buffer[bytesRead] = '\0';

            char* newlinePos = find(buffer, buffer + bytesRead, '\n');
            if (newlinePos != buffer + bytesRead) {
                *newlinePos = '\0';
            }

			string content(buffer);
            content.erase(remove(content.begin(), content.end(), '\r'), content.end());

            sysFsClose(fd);
            return content;
        } else {
            printf("Couldn't read the source file: %s (error code %d)\n", filePath, ret);
            sysFsClose(fd);
        }
    }

    // If cannot read the file, returns the actual date & time
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return string(buf);
}

const string buildNumber() {
    const char* filePath = ("/dev_hdd0/game/" + string(APP_TITLEID) + "/USRDIR/data/version").c_str();

    s32 fd;
    int ret = sysFsOpen(filePath, SYS_O_RDONLY, &fd, NULL, 0);
    if (ret == 0) {
        char buffer[BUFFER_SIZE];
        uint64_t bytesRead;
        ret = sysFsRead(fd, buffer, BUFFER_SIZE - 1, &bytesRead);
        if (ret == 0) {
           
		   buffer[bytesRead] = '\0';

            char* newlinePos = find(buffer, buffer + bytesRead, '\n');
            if (newlinePos != buffer + bytesRead) {
                *newlinePos = '\0'; 
            }

            string firstLine(buffer);
            firstLine.erase(remove(firstLine.begin(), firstLine.end(), '\r'), firstLine.end());

            char* secondLineStart = newlinePos + 1;

            newlinePos = find(secondLineStart, buffer + bytesRead, '\n');
            if (newlinePos != secondLineStart + bytesRead) {
                *newlinePos = '\0';
            }

            string secondLine(secondLineStart);
            secondLine.erase(remove(secondLine.begin(), secondLine.end(), '\r'), secondLine.end());

            sysFsClose(fd);
            return secondLine;
        } else {
            printf("Couldn't read the source file: %s (error code %d)\n", filePath, ret);
            sysFsClose(fd);
        }
    }

    return "";  
}

void deleteIfZeroBytes(const char* filePath) {
    sysFSStat stat;

    // Check if the file exists and retrieve its information
    if (sysFsStat(filePath, &stat) == 0) {
        // Delete the file if its size is 0 bytes
        if (stat.st_size == 0) {
            sysFsUnlink(filePath);
        }
    }
}

int string_array_size(string *arr)
{
	int size=0;
	while (strcmp(arr[size].c_str(),"") != 0)
	{
		size++;
	}
	return size;
}

int restore(string appfolder, string foldername)
{
	string ret="";
	string variant="";
	string problems="\n\nPlease be advised that this process changes system files so 'do not turn off the system' while the process is running.\n\nIf you have some corruption after copying the files or if the installer quits unexpectly, check all files before restarting and if possible reinstall the firmware from the XMB™ or recovery menu.";
	
	if (foldername == "Uninstall PS3 Pro Lite") variant="PS3™ Pro Lite"; 
	else if (foldername == "Uninstall PS3 4K Pro Lite") variant="PS3™ 4K Pro Lite";
	else variant=foldername;
	
	//Mess.Dialog(MSG_YESNO_DYES,("Are you sure you want to restore '"+foldername+"' backup?"+problems).c_str());
	Mess.Dialog(MSG_YESNO_DYES,("Are you sure you want to uninstall the "+variant+"?"+problems).c_str());
	if (Mess.GetResponse(MSG_DIALOG_BTN_YES)==1)
	{
		//Mount flash if not mounted
		mount_dev_blind();
		
		setLed("rainbow");

		//Unmap mapped paths
		sys_map_path((char*)"/dev_flash/ps1emu/ps1_emu.self", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/ps1emu/ps1_emu.self", NULL);
		sys_map_path((char*)"/dev_flash/ps1emu/ps1_netemu.self", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/ps1emu/ps1_netemu.self", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/custom_render_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/custom_render_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/download_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/download_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/explore_category_psn.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/explore_category_psn.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/explore_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/explore_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/game_ext_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/game_ext_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/newstore_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/newstore_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/sysconf_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/sysconf_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/wboard_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/wboard_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/xmb_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/xmb_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/resource/explore/icon/hen_enable.png", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/resource/explore/icon/hen_enable.png", NULL);
		sys_map_path((char*)"/dev_flash/vsh/resource/software_update_plugin.rco", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/resource/software_update_plugin.rco", NULL);
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/HEN/hen_enable.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/HEN/hen_enable.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/plugin_folder.png", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/plugin_folder.png", NULL);
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/remaps/webman_plugin_features.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/remaps/webman_plugin_features.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/plugins_manager/plugins_manager_xai_HFW_items.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/plugins_manager/plugins_manager_xai_HFW_items.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/switch_features/xai_plugin_features.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/switch_features/xai_plugin_features.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/hen/category_playstation_home.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/hen/category_playstation_home.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/hen/category_restore_system_files.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/hen/category_restore_system_files.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/hen/flash_games.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/hen/flash_games.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/hen/pro_features.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/hen/pro_features.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/hen/unofficial_stores.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/hen/unofficial_stores.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/webman_external_game_data.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/webman_external_game_data.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/remaps/webman_system_features.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/remaps/webman_system_features.xml", NULL);
		sys_map_path(NULL, NULL);
		sys_map_paths(NULL, NULL, 0);

		//Delete mapped files
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/switch_features/webman_plugin_features.xml");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/switch_features/xai_plugin_features.xml");
		sysFsUnlink((char*)"/dev_hdd0/xmlhost/game_plugin/fb-hen.xml");
		
		//Delete some PS3™ 4K Pro files in flash
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/download_plugin.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/explore_category_psn.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/explore_plugin.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/game_ext_plugin.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/hen_enabled.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/newstore_plugin.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/npsignin_plugin.off");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/npsignin_plugin.rco");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/ps1_emu.self");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/ps1_netemu.self");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/software_update_plugin.rco");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/sysconf_plugin.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/system_plugin.off");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/system_plugin.rco");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/wboard_plugin.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/xmb_plugin.off");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/xmb_plugin.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/about.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/blind.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/convert_save.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/delete_package.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/disc_refresh.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/download.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/dump_erk.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/dvd_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/file_manager.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/flash.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_all_homebrews.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_cheats.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_download.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_emulators.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_exdata.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_game.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_launchers.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_list.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_maintenance.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_network.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_package.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_plain.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_play.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_plugin.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_ps2.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_ps3.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_psp.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_psx.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_retro.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_theme.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_tools.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_update.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_video.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_warn.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_xmbm.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/game.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/game_data.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/google_cloud_storage.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/hdd.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/hen_pro.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/homebrews_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/ingame_enable_ss.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/ingame_override_sfo.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/music.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/online_video.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/online_wallpaper.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/package.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/package_delete.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/package_download.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/package_hdd.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/package_package.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/package_psn.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/photo.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/playstation_cloud_storages.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/plus_cloud_storage.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/ps2_classics_launcher.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/ps2_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/ps3_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psn_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psp_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psp_launcher.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psx_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psx_launcher.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/reboot.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/reboot_warning.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/reload_xmb.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/remarry_bd_drive.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/retro_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/retro_launcher.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/screensaver.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/step.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/clear_history.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/no_psn.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/disable_syscalls.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/donate.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/email.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/file_manager.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/flash_games.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/game_library.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/information_board.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/members_chat.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/online_radio.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/online_tv.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/podcast.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psn_access.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psn_disabled.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psn_enabled.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/screen_mirror.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/unofficial_store.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/xmb_screenshot.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_home_catalogue.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/server.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/tools.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/trash.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/update_download.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/usb.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/video.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/warning.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/web.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/web_warning.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/hen_emergency.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/hen_enable.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/hen_full.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/ps3xploit_www.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_game_folders.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_game_folders/ps2_classics_dex_warning.off");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_game_folders/ps2_classics_dex_warning.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_game_trophies.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_package_manager.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_playstation_home.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_restore_hen_exploit.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_restore_system_files.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_service_mode_tools.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_system_features.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_toolbox.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/npsignin_plugin.lck");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/npsignin_plugin.off");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/theme/01.p3t");

		//Delete extra user avatars
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/027.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/028.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/029.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/030.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/031.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/032.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/033.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/034.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/035.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/036.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/037.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/038.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/039.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/040.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/041.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/042.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/043.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/044.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/045.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/046.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/047.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/048.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/049.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/050.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/051.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/052.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/053.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/054.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/055.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/056.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/057.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/058.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/059.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/060.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/061.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/062.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/063.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/064.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/065.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/066.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/067.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/068.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/069.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/070.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/071.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/072.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/073.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/074.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/075.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/076.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/077.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/078.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/079.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/080.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/081.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/082.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/083.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/084.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/085.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/086.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/087.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/088.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/089.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/090.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/091.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/092.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/093.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/094.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/095.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/096.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/097.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/098.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/099.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/100.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/100.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/101.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/102.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/103.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/104.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/105.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/106.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/107.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/108.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/109.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/110.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/111.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/112.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/113.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/114.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/115.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/116.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/117.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/118.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/119.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/120.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/121.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/122.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/123.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/124.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/125.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/126.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/127.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/128.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/129.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/130.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/131.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/132.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/133.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/134.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/135.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/136.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/137.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/138.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/139.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/140.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/user/141.png");

		//Delete files installed in dev_hdd0 that are not deleted by deleting the pkg
		recursiveDelete("/dev_hdd0/PROISO");
		recursiveDelete("/dev_hdd0/download/theme");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 4K Pro Lite/PS3~dev_hdd0/hen");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 Pro Lite/PS3~dev_hdd0/hen");
		recursiveDelete("/dev_hdd0/game/PS3ONLINE/USRDIR/settings/installedplugins");
		recursiveDelete("/dev_hdd0/game_debug");
		recursiveDelete("/dev_hdd0/hen/themes");
		recursiveDelete("/dev_hdd0/tmp/prep_icons");
		recursiveDelete("/dev_hdd0/tmp/wm_ingame");
		recursiveDelete("/dev_hdd0/xmlhost.off");
		sysFsUnlink((char*)"/dev_hdd0/autoexec.bat");
		sysFsUnlink((char*)"/dev_hdd0/boot_init.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_init_swap.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_plugins.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_plugins_firmware_change.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_plugins_kernel_nocobra.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_plugins_kernel_nocobra_dex.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_plugins_nocobra.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_plugins_nocobra_dex.txt");
		sysFsUnlink((char*)"/dev_hdd0/ingame.bat");
		sysFsUnlink((char*)"/dev_hdd0/onxmb.bat");
		sysFsUnlink((char*)"/dev_hdd0/patches.bat");
		sysFsUnlink((char*)"/dev_hdd0/ps3-updatelist.txt");
		sysFsUnlink((char*)"/dev_hdd0/tmp/wm_icons/icon_wm_root.png");
		
		//Reset Vsh Modes
		sysFsUnlink((char*)"/dev_blind/vsh/module/vsh.self.dex");
		sysFsUnlink((char*)"/dev_blind/vsh/module/vsh.self.cex");
		sysFsUnlink((char*)"/dev_blind/vsh/module/vsh.self.cexsp");
		sysFsUnlink((char*)"/dev_blind/vsh/module/vsh.self.dexsp");
		sysFsUnlink((char*)"/dev_blind/vsh/module/vsh.self.swp");
		sysFsUnlink((char*)"/dev_blind/vsh/module/vsh.self.nrm");
		
		ret=copy_prepare(appfolder, "restore", foldername, "", "");
		if (ret == "") //restore success
		{

			//Delete the check file to allow the installer to run again
			sysFsUnlink((char*)"/dev_blind/vsh/etc/lite.txt");
			sysFsUnlink((char*)"/dev_blind/vsh/etc/premium.txt");
			sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/pro.xml");
			
			//Delete HEN Plus files
			recursiveDelete("/dev_hdd0/game/PS34KPROL/HEN");
			recursiveDelete("/dev_hdd0/game/PS34KPROX/HEN");
			sysFsUnlink((char*)"/dev_blind/hen/HENPLUS.BIN");
			sysFsUnlink((char*)"/dev_blind/hen/xml/hen_enabled.xml");
			sysFsUnlink((char*)"/dev_blind/hen/xml/pro_features.xml");		
			sysFsUnlink((char*)"/dev_blind/hen/xml/audio_patch.off");
			sysFsUnlink((char*)"/dev_blind/hen/xml/audio_patch.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/hen_auto_update.off");
			sysFsUnlink((char*)"/dev_blind/hen/xml/hen_auto_update.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/clear_web_auth_cache.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/clear_web_cookies.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/clear_web_history.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/clear_ci.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/clear_mi.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/clear_ptl.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/reload_xmb.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/reload_xmb.off");
			sysFsUnlink((char*)"/dev_blind/hen/xml/beep.off");
			sysFsUnlink((char*)"/dev_blind/hen/xml/beep.on");
			sysFsUnlink((char*)"/dev_hdd0/HENplugin.sprx");

			//Delete the check file to set the initial settings
			sysFsUnlink((char*)"/dev_hdd0/vm/settings.xml");
			sysFsUnlink((char*)"/dev_hdd0/vm/optimize.xml");
			
			//Delete alternative default theme to free up space and revert to ofw look
			sysFsUnlink((char*)"/dev_blind/vsh/resource/theme/01.p3t");
			
			//Delete game folders
			recursiveDelete("/dev_blind/vsh/resource/explore/xmb/category_game_folders");
			
			//Reset Patches
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/game_column_fix");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/current");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/current/D800");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/current/dynamic");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/disabled/D800");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/disabled/dynamic");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/enabled/D800");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/enabled/dynamic");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/swap/D800");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/swap/dynamic");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/swap/dynamic");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/hen_mode");				
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/online/unofficial_stores/sub_items/settings.xml");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/boot_plugins/cfw/boot_init.txt");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/boot_plugins/hfw/boot_init.txt");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/system_update_server/ps3-updatelist");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D001/dev_blind/vsh/module/custom_render_plugin.sprx");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D001/dev_blind/vsh/resource/custom_render_plugin.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D001/dev_blind/vsh/resource/explore/icon/yourchannels.png");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D001/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D001/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D002/dev_blind/vsh/module/custom_render_plugin.sprx");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D002/dev_blind/vsh/resource/explore/icon/yourchannels.png");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D002/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D002/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D003/dev_blind/vsh/module/custom_render_plugin.sprx");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D003/dev_blind/vsh/resource/explore/icon/yourchannels.png");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D003/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D003/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D004/dev_blind/vsh/module/custom_render_plugin.sprx");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D004/dev_blind/vsh/resource/explore/icon/yourchannels.png");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D004/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D004/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D005/dev_blind/vsh/module/custom_render_plugin.sprx");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D005/dev_blind/vsh/resource/explore/icon/yourchannels.png");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D005/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D005/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D006/dev_blind/vsh/module/custom_render_plugin.sprx");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D006/dev_blind/vsh/resource/explore/icon/yourchannels.png");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D006/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D006/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D001/dev_blind/vsh/module/custom_render_plugin.sprx");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D001/dev_blind/vsh/resource/custom_render_plugin.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D001/dev_blind/vsh/resource/explore/icon/yourchannels.png");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D001/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D001/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D002/dev_blind/vsh/module/custom_render_plugin.sprx");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D002/dev_blind/vsh/resource/explore/icon/yourchannels.png");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D002/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D002/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D003/dev_blind/vsh/module/custom_render_plugin.sprx");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D003/dev_blind/vsh/resource/explore/icon/yourchannels.png");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D003/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D003/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D004/dev_blind/vsh/module/custom_render_plugin.sprx");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D004/dev_blind/vsh/resource/explore/icon/yourchannels.png");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D004/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D004/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D005/dev_blind/vsh/module/custom_render_plugin.sprx");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D005/dev_blind/vsh/resource/explore/icon/yourchannels.png");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D005/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D005/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D006/dev_blind/vsh/module/custom_render_plugin.sprx");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D006/dev_blind/vsh/resource/explore/icon/yourchannels.png");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D006/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D006/dev_blind/vsh/resource/xmb_plugin_normal.rco");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/essential_apps/essential_apps.xml");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/hen_features/sub_items/hen_update.xml");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/switch_features/webman_plugin_features.xml");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/switch_features/xai_plugin_features.xml");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/visual_customizations/sub_items/game_column_style/game_column_style.xml");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/visual_customizations/sub_items/transformation_pack/transformation_pack.xml");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/visual_customizations/visual_customizations.xml");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/online/unofficial_stores/sub_items/package_downloader/package_downloader.xml");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/settings/settings.xml");
			

			//Reset Icons
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/layout_fix");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/online_servers");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/playstation_store");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/plugins_icons");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/off.png");
			sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/on.png");
			
			//Delete plugins
			recursiveDelete("/dev_hdd0/plugins/ps3_menu");
			recursiveDelete("/dev_hdd0/plugins/PS3Lock");
			recursiveDelete("/dev_hdd0/plugins/ps3xpad");
			recursiveDelete("/dev_hdd0/plugins/RouLetteVshMenu");
			sysFsUnlink((char*)"/dev_hdd0/plugins/akari_vsh_menu.sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/EBOOTLoader.sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/fps_counter.sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/fps_counter.yaml");
			sysFsUnlink((char*)"/dev_hdd0/plugins/gameboot_cycle.sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/openps3ftp.sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/PS3_PPU_Project2.1.2.sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/raibow_xmb_complete_ccapi.sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/rainbow_xmb.sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/psnpatch.sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/psnpatch.off");
			sysFsUnlink((char*)"/dev_hdd0/plugins/rainbow_xmb.p3t");
			sysFsUnlink((char*)"/dev_hdd0/plugins/roulette_vsh_menu.sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/slaunch.sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/trophy_unlocker.sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/TrophyUnlocker (DPAD LEFT).sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/xmb_plugin.sprx");
			sysFsUnlink((char*)"/dev_hdd0/plugins/xmb_plugin");
			sysFsUnlink((char*)"/dev_hdd0/plugins/akari_vsh_menu.off");
			sysFsUnlink((char*)"/dev_hdd0/plugins/EBOOTLoader.off");
			sysFsUnlink((char*)"/dev_hdd0/plugins/fps_counter.off");
			sysFsUnlink((char*)"/dev_hdd0/plugins/gameboot_cycle.off");
			sysFsUnlink((char*)"/dev_hdd0/plugins/openps3ftp.off");
			sysFsUnlink((char*)"/dev_hdd0/plugins/rainbow_xmb.off");
			sysFsUnlink((char*)"/dev_hdd0/plugins/rainbow_xmb.p3t");
			sysFsUnlink((char*)"/dev_hdd0/plugins/roulette_vsh_menu.off");
			sysFsUnlink((char*)"/dev_hdd0/plugins/slaunch.off");
			sysFsUnlink((char*)"/dev_hdd0/plugins/trophy_unlocker.off");
			sysFsUnlink((char*)"/dev_hdd0/plugins/xmb_plugin.off");

			//Enable Cobra payload if disabled
			sysLv2FsRename("/dev_blind/rebug/cobra/stage2.cex.bak","/dev_blind/rebug/cobra/stage2.cex");
			sysLv2FsRename("/dev_blind/rebug/cobra/stage2.dex.bak","/dev_blind/rebug/cobra/stage2.dex");
			sysLv2FsRename("/dev_blind/sys/stage2.bin.bak","/dev_blind/sys/stage2.bin");
			sysLv2FsRename("/dev_blind/sys/stage2.bin.bak","/dev_blind/sys/stage2.bin");
			sysLv2FsRename("/dev_blind/sys/stage2.cex.bak","/dev_blind/sys/stage2.cex");
			sysLv2FsRename("/dev_blind/sys/stage2.dex.bak","/dev_blind/sys/stage2.dex");
			sysLv2FsRename("/dev_blind/spy/cobra/stage2.bin.bak","/dev_blind/spy/cobra/stage2.bin");

			//Delete webMAN's random XMB™ files
			recursiveDelete("/dev_hdd0/tmp/canyon");
			recursiveDelete("/dev_hdd0/tmp/canyon.off");
			recursiveDelete("/dev_hdd0/tmp/clock");
			recursiveDelete("/dev_hdd0/tmp/clock.off");
			recursiveDelete("/dev_hdd0/tmp/coldboot");
			recursiveDelete("/dev_hdd0/tmp/coldboot.off");
			recursiveDelete("/dev_hdd0/tmp/earth");
			recursiveDelete("/dev_hdd0/tmp/earth.off");
			recursiveDelete("/dev_hdd0/tmp/gameboot");
			recursiveDelete("/dev_hdd0/tmp/gameboot.off");
			recursiveDelete("/dev_hdd0/tmp/gameboot_logo");
			recursiveDelete("/dev_hdd0/tmp/gameboot_logo.off");
			recursiveDelete("/dev_hdd0/tmp/impose");
			recursiveDelete("/dev_hdd0/tmp/impose.off");
			recursiveDelete("/dev_hdd0/tmp/lines");
			recursiveDelete("/dev_hdd0/tmp/lines.off");
			recursiveDelete("/dev_hdd0/tmp/psn_icons");
			recursiveDelete("/dev_hdd0/tmp/psn_icons.off");
			recursiveDelete("/dev_hdd0/tmp/system_music");
			recursiveDelete("/dev_hdd0/tmp/theme");
			recursiveDelete("/dev_hdd0/tmp/theme.off");
			recursiveDelete("/dev_hdd0/tmp/wallpaper");
			recursiveDelete("/dev_hdd0/tmp/wallpaper.off");
			sysFsUnlink((char*)"/dev_hdd0/tmp/wm_res/npsignin_plugin.rco");
			
			//Re-enable webMAN plugins
			sysLv2FsRename("/dev_hdd0/tmp/wm_res/VshFpsCounter.off","/dev_hdd0/tmp/wm_res/VshFpsCounter.sprx");
			sysLv2FsRename("/dev_hdd0/tmp/wm_res/VshFpsCounterM.off","/dev_hdd0/tmp/wm_res/VshFpsCounterM.sprx");
			sysLv2FsRename("/dev_hdd0/tmp/wm_res/slaunch.off","/dev_hdd0/tmp/wm_res/slaunch.sprx");
			sysLv2FsRename("/dev_hdd0/tmp/wm_res/video_rec.off","/dev_hdd0/tmp/wm_res/video_rec.sprx");
			sysLv2FsRename("/dev_hdd0/tmp/wm_res/wm_vsh_menu.off","/dev_hdd0/tmp/wm_res/wm_vsh_menu.sprx");
			sysLv2FsRename("/dev_hdd0/xmlhost.off","/dev_hdd0/xmlhost");
			
			//Delete backup
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/backup");
			
			/* if (is_dev_blind_mounted()==0)
			{

				unmount_dev_blind();
				Mess.Dialog(MSG_YESNO_DYES, ("The "+variant+" has been uninstalled with success.\nThe system files have been modified, do you want to reboot the system?").c_str());
				if (Mess.GetResponse(MSG_DIALOG_BTN_YES)==1) return 2;
				else 
					{
						Graphics->AppExit();
						return 1;
					}
			} */
			
			single_beep();
			setLed("green_default");
			
			Mess.Dialog(MSG_OK, ("The "+variant+" has been uninstalled with success\nPress OK to continue.").c_str());
			Mess.Dialog(MSG_YESNO_DYES, ("Would you like to optimize the system by rebuilding the database?\n\nThis process will delete the following content:\n* Chat rooms and messages.\n* Custom information for Photos/Music/Game/Video and album names.\n* Music playlists and Video playback history, resume information and thumbnails.\n\nThis process is recommended to improve boot times for applications and system performance."));
			if (Mess.GetResponse(MSG_DIALOG_BTN_YES)==1) 
			{
				rebuild_db();
				create_file("/dev_hdd0/vm/optimize.xml");
				recursiveDelete("/dev_hdd0/game/PS3ONLINE");
				return 2;
			}
			else
			{
				return 2;
			}
		}
		else //problem in the restore process so emit a warning
		{
			triple_beep();
			setLed("red_blink_fast");
			
			Mess.Dialog(MSG_ERROR,("The "+variant+" has not been uninstalled!\nAn error occured while restoring the backup!\n\nError: "+ret+"\n\nTry to restore again manually, if the error persists, the system may be corrupted, please check all files and if needed reinstall the firmware from the XMB™ or recovery menu.\nPress OK to exit.").c_str());
			Graphics->AppExit(); //To avoid crash luan
		}
	}

	return 0;
}

int install(string appfolder, string firmware_folder, string app_choice)
{
	string ret="";
	string variant="";
	string foldername="";
	string problems="\n\nPlease be advised that, this process changes the system files so 'do not turn off the system' while the process in running.\n\nIf you have some corruption after copying the files or the installer quits unexpectly check all files before restarting and if possible reinstall the firmware from the XMB™ or recovery menu.";
	
	//string foldername="Before Installation of "+app_choice+currentDateTime();
	
	if (firmware_folder == "All-All-1. PS3 Pro Lite") variant="PS3™ Pro Lite", foldername="Uninstall PS3 Pro Lite"; 
	else if (firmware_folder == "All-All-2. PS3 4K Pro Lite") variant="PS3™ 4K Pro Lite", foldername ="Uninstall PS3 4K Pro Lite";
	else variant=firmware_folder, foldername=firmware_folder;
	
	//Mess.Dialog(MSG_YESNO_DNO,("Are you sure you want to install "+app_choice+"?"+problems).c_str());
	Mess.Dialog(MSG_YESNO_DNO,("Are you sure you want to install the "+variant+"?"+problems).c_str());
	if (Mess.GetResponse(MSG_DIALOG_BTN_YES)==1)
	{
		
		if (console_model == "Fat")
		{		
			Mess.Dialog(MSG_YESNO_DYES, "Warning:\nThis console model is the FAT model.\nInstallation on these models is known to be slower and takes several minutes.\n\nDo you want to continue?");
		}
		
		if (Mess.GetResponse(MSG_DIALOG_BTN_YES)==1) ;else 
		{
			return 0;
		}
		
		if (jailbreak == "Custom Firmware")
		{
			if (gpu_clock_speed.find("Overclock") != string::npos)
			{
				Mess.Dialog(MSG_OK,"Notice:\nThe system has detected that an overclocked firmware is installed.\n\nAs a result, 'Boost Mode' will be available!");
			}
			else if (gpu_clock_speed.find("Standard") != string::npos)
			{
				Mess.Dialog(MSG_OK,"Notice:\nThe system has detected that a standard firmware is installed.\n\nAs a result, the 'Boost Mode' will not be available!\nThis feature is only available on overclocked firmwares."); 
			}
			else if (gpu_clock_speed.find("Underclock") != string::npos)
			{
				Mess.Dialog(MSG_OK,"Notice:\nThe system has detected that a underclocked firmware is installed.\nUnderclocked firmwares are not recommended due to its lower performance.\n\nAs a result, the 'Boost Mode' will not be available!\nThis feature is only available on overclocked firmwares."); 
			}
		}
		
		setLed("rainbow");

		//Mount flash if not mounted
		mount_dev_blind();
		//Unmap mapped paths
		sys_map_path((char*)"/dev_flash/ps1emu/ps1_emu.self", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/ps1emu/ps1_emu.self", NULL);
		sys_map_path((char*)"/dev_flash/ps1emu/ps1_netemu.self", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/ps1emu/ps1_netemu.self", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/custom_render_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/custom_render_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/download_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/download_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/explore_category_psn.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/explore_category_psn.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/explore_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/explore_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/game_ext_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/game_ext_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/newstore_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/newstore_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/sysconf_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/sysconf_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/wboard_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/wboard_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/module/xmb_plugin.sprx", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/module/xmb_plugin.sprx", NULL);
		sys_map_path((char*)"/dev_flash/vsh/resource/explore/icon/hen_enable.png", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/resource/explore/icon/hen_enable.png", NULL);
		sys_map_path((char*)"/dev_flash/vsh/resource/software_update_plugin.rco", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_flash/vsh/resource/software_update_plugin.rco", NULL);
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/HEN/hen_enable.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/HEN/hen_enable.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/plugin_folder.png", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/plugin_folder.png", NULL);
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/remaps/webman_plugin_features.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/remaps/webman_plugin_features.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/plugins_manager/plugins_manager_xai_HFW_items.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/plugins_manager/plugins_manager_xai_HFW_items.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/switch_features/xai_plugin_features.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/switch_features/xai_plugin_features.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/hen/category_playstation_home.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/hen/category_playstation_home.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/hen/category_restore_system_files.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/hen/category_restore_system_files.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/hen/flash_games.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/hen/flash_games.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/hen/pro_features.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/hen/pro_features.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/hen/unofficial_stores.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/hen/unofficial_stores.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/webman_external_game_data.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/webman_external_game_data.xml", NULL);
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/remaps/webman_system_features.xml", (char*)"/dev_flash/vsh/resource/explore/xmb/null.xml");
		sys_map_path((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/remaps/webman_system_features.xml", NULL);
		sys_map_path(NULL, NULL);
		sys_map_paths(NULL, NULL, 0);

		//Delete the check file to set the initial settings
		sysFsUnlink((char*)"/dev_hdd0/vm/settings.xml");
		sysFsUnlink((char*)"/dev_hdd0/vm/optimize.xml");
		
		//Delete mapped files
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/switch_features/webman_plugin_features.xml");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/switch_features/xai_plugin_features.xml");
		
		//Delete and create game folders
		recursiveDelete("/dev_blind/vsh/resource/explore/xmb/category_game_folders");
		sysFsMkdir((char*)"/dev_blind/vsh/resource/explore/xmb/category_game_folders", 0777);

		//Delete HEN enablers
		recursiveDelete("/dev_hdd0/game/PS34KPROL/HEN");
		recursiveDelete("/dev_hdd0/game/PS34KPROX/HEN");
			
		if (jailbreak == "Hybrid Firmware")
		{	
			sysFsUnlink((char*)"/dev_blind/hen/HENPLUS.BIN");
			sysFsUnlink((char*)"/dev_blind/hen/xml/hen_enabled.xml");
			sysFsUnlink((char*)"/dev_blind/hen/xml/pro_features.xml");		
			sysFsUnlink((char*)"/dev_blind/hen/xml/audio_patch.off");
			sysFsUnlink((char*)"/dev_blind/hen/xml/audio_patch.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/hen_auto_update.off");
			sysFsUnlink((char*)"/dev_blind/hen/xml/hen_auto_update.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/clear_web_auth_cache.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/clear_web_cookies.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/clear_web_history.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/clear_ci.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/clear_mi.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/clear_ptl.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/reload_xmb.on");
			sysFsUnlink((char*)"/dev_blind/hen/xml/reload_xmb.off");
			sysFsUnlink((char*)"/dev_blind/hen/xml/beep.off");
			sysFsUnlink((char*)"/dev_blind/hen/xml/beep.on");
			sysFsUnlink((char*)"/dev_hdd0/HENplugin.sprx");
		
			//Create necessary folders on HEN, lmn7's autohen renames them
			sysFsChmod((char*)"/dev_blind/hen", 0777);
			sysFsMkdir((char*)"/dev_blind/hen", 0777);
			sysFsMkdir((char*)"/dev_blind/hen/xml", 0777);
			sysFsMkdir((char*)"/dev_blind/vsh/resource/AAA", 0777);
		}

		//Create default jailbreak folders
		sysFsMkdir((char*)"/dev_hdd0/BDISO", 0777);
		sysFsMkdir((char*)"/dev_hdd0/DVDISO", 0777);
		sysFsMkdir((char*)"/dev_hdd0/GAMES", 0777);
		sysFsMkdir((char*)"/dev_hdd0/PS2ISO", 0777);
		sysFsMkdir((char*)"/dev_hdd0/PS3ISO", 0777);
		sysFsMkdir((char*)"/dev_hdd0/PSPISO", 0777);
		sysFsMkdir((char*)"/dev_hdd0/PSXISO", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS", 0777);
		sysFsMkdir((char*)"/dev_hdd0/exdata", 0777);
		sysFsMkdir((char*)"/dev_hdd0/music", 0777);
		sysFsMkdir((char*)"/dev_hdd0/packages", 0777);
		sysFsMkdir((char*)"/dev_hdd0/photo", 0777);
		sysFsMkdir((char*)"/dev_hdd0/plugins", 0777);
		sysFsMkdir((char*)"/dev_hdd0/theme", 0777);
		sysFsMkdir((char*)"/dev_hdd0/updater/01", 0777);
		sysFsMkdir((char*)"/dev_hdd0/video", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/SNES", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/SNES9X", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/SNES9X2005", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/SNES9X2010", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/SNES9X_NEXT", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/MSNES", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/NES", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/FCEUMM", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/NESTOPIA", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/QNES", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/GENESIS", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/MEGADRIVE", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/SEGACD", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/PICO", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/GG", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/GGEAR", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/SG1000", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/GBA", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/VBA", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/MGBA", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/GB", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/GBC", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/GAMBATTE", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/VBOY", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/FBA", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/FBA2012", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/FBNEO", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/NEOCD", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/NEO", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/NEOGEO", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/NGP", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/NGPC", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/MAME", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/MAME078", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/MAME2000", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/MAME2003", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/MAMEPLUS", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/MIDWAY", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/QUAKE", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/DOOM", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/PCE", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/PCECD", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/PCFX", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/SGX", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/ATARI", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/STELLA", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/ATARI2600", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/ATARI800", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/ATARI5200", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/ATARI7800", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/HATARI", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/LYNX", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/JAGUAR", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/HANDY", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/AMIGA", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/CD32", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/VICE", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/X64", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/X64SC", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/XSCPU64", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/X64DTV", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/X128", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/XCBM2", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/XCBM25X0", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/XCBM5", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/XPET", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/XPLUS4", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/XVIC", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/FMSX", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/BMSX", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/MSX", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/MSX2", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/BLUEMSX", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/COLECO", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/O2EM", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/INTV", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/ZX81", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/FUSE", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/2048", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/CAP32", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/CANNONBALL", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/NXENGINE", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/DOS", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/DOSBOX", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/GME", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/GW", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/JAVAME", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/JME", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/SQUIRREL", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/JUMP", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/MU", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/PALM", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/POKEMINI", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/SCUMMVM", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/THEODORE", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/VECX", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/WSWAM", 0777);
		sysFsMkdir((char*)"/dev_hdd0/ROMS/WSWAMC", 0777);
		
		//Delete alternative default theme to free up space
		sysFsUnlink((char*)"/dev_blind/vsh/resource/theme/01.p3t");
		
		//Delete packages on flash to free up space
		recursiveDelete("/dev_blind/ferrox/pkg"); //If the ferrox folder is deleted, ps2 playback will be broken
		recursiveDelete("/dev_blind/rebug/packages");
		recursiveDelete("/dev_blind/vsh/resource/explore/netflix");
		sysFsMkdir((char*)"/dev_blind/vsh/resource/explore/netflix", 0777);

		//Delete old installation files if the user has not properly uninstalled, for example installing a firmware:
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/download_plugin.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/explore_category_psn.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/newstore_plugin.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/npsignin_plugin.off");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/npsignin_plugin.rco");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/ps1_emu.self");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/ps1_netemu.self");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/software_update_plugin.rco");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/sysconf_plugin.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/system_plugin.off");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/system_plugin.rco");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/wboard_plugin.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/xmb_plugin.off");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/AAA/xmb_plugin.sprx");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/blind.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/convert_save.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/delete_package.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/disc_refresh.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/download.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/dump_erk.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/dvd_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/file_manager.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/flash.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_all_homebrews.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_cheats.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_download.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_emulators.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_exdata.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_game.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_launchers.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_list.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_maintenance.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_network.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_package.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_plain.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_play.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_plugin.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_ps2.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_ps3.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_psp.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_psx.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_retro.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_theme.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_tools.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_update.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_video.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_warn.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_xmbm.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/game.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/game_data.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/google_cloud_storage.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/hdd.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/hen_pro.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/homebrews_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/ingame_enable_ss.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/ingame_override_sfo.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/music.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/online_video.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/online_wallpaper.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/package.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/package_delete.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/package_download.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/package_hdd.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/package_package.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/package_psn.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/photo.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/playstation_cloud_storages.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/plus_cloud_storage.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/ps2_classics_launcher.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/ps2_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/ps3_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psn_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psp_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psp_launcher.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psx_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psx_launcher.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/reboot.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/reboot_warning.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/reload_xmb.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/remarry_bd_drive.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/retro_folder.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/retro_launcher.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/screensaver.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/step.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/clear_history.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/no_psn.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/disable_syscalls.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/donate.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/email.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/file_manager.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/flash_games.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/game_library.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/information_board.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/members_chat.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/online_radio.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/online_tv.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/podcast.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psn_access.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psn_disabled.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/psn_enabled.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/screen_mirror.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/unofficial_store.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/xmb_screenshot.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/folder_home_catalogue.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/server.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/tools.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/trash.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/update_download.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/usb.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/video.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/warning.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/web.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/icon/web_warning.png");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_game_folders.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_game_trophies.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_package_manager.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_playstation_home.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_restore_hen_exploit.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_restore_system_files.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_service_mode_tools.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_system_features.xml");
		sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_toolbox.xml");
		
		//Reset Icons
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/online_servers");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/playstation_store");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/plugins_icons");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/off.png");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/icons/on.png");
		
		//Delete per-game script
		recursiveDelete("/dev_hdd0/tmp/wm_ingame");
		
		//Reset Patches
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/game_column_fix");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/current");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/current/D800");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/current/dynamic");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/disabled/D800");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/disabled/dynamic");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/enabled/D800");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/enabled/dynamic");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/swap/D800");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/swap/dynamic");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/gameboot/swap/dynamic");
		recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/hen_mode");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/online/unofficial_stores/sub_items/settings.xml");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/boot_plugins/cfw/boot_init.txt");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/boot_plugins/hfw/boot_init.txt");		
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/system_update_server/ps3-updatelist");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D001/dev_blind/vsh/module/custom_render_plugin.sprx");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D001/dev_blind/vsh/resource/custom_render_plugin.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D001/dev_blind/vsh/resource/explore/icon/yourchannels.png");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D001/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D001/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D002/dev_blind/vsh/module/custom_render_plugin.sprx");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D002/dev_blind/vsh/resource/explore/icon/yourchannels.png");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D002/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D002/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D003/dev_blind/vsh/module/custom_render_plugin.sprx");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D003/dev_blind/vsh/resource/explore/icon/yourchannels.png");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D003/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D003/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D004/dev_blind/vsh/module/custom_render_plugin.sprx");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D004/dev_blind/vsh/resource/explore/icon/yourchannels.png");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D004/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D004/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D005/dev_blind/vsh/module/custom_render_plugin.sprx");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D005/dev_blind/vsh/resource/explore/icon/yourchannels.png");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D005/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D005/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D006/dev_blind/vsh/module/custom_render_plugin.sprx");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D006/dev_blind/vsh/resource/explore/icon/yourchannels.png");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D006/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/4K/D006/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D001/dev_blind/vsh/module/custom_render_plugin.sprx");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D001/dev_blind/vsh/resource/custom_render_plugin.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D001/dev_blind/vsh/resource/explore/icon/yourchannels.png");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D001/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D001/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D002/dev_blind/vsh/module/custom_render_plugin.sprx");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D002/dev_blind/vsh/resource/explore/icon/yourchannels.png");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D002/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D002/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D003/dev_blind/vsh/module/custom_render_plugin.sprx");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D003/dev_blind/vsh/resource/explore/icon/yourchannels.png");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D003/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D003/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D004/dev_blind/vsh/module/custom_render_plugin.sprx");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D004/dev_blind/vsh/resource/explore/icon/yourchannels.png");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D004/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D004/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D005/dev_blind/vsh/module/custom_render_plugin.sprx");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D005/dev_blind/vsh/resource/explore/icon/yourchannels.png");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D005/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D005/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D006/dev_blind/vsh/module/custom_render_plugin.sprx");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D006/dev_blind/vsh/resource/explore/icon/yourchannels.png");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D006/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/transformation_pack/normal/Pro/D006/dev_blind/vsh/resource/xmb_plugin_normal.rco");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/essential_apps/essential_apps.xml");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/hen_features/sub_items/hen_update.xml");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/switch_features/webman_plugin_features.xml");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/switch_features/xai_plugin_features.xml");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/visual_customizations/sub_items/game_column_style/game_column_style.xml");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/visual_customizations/sub_items/transformation_pack/transformation_pack.xml");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/visual_customizations/visual_customizations.xml");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/online/unofficial_stores/sub_items/package_downloader/package_downloader.xml");
		sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/settings/settings.xml");

		//Delete plugins
		recursiveDelete("/dev_hdd0/plugins/ps3_menu");
		recursiveDelete("/dev_hdd0/plugins/PS3Lock");
		recursiveDelete("/dev_hdd0/plugins/ps3xpad");
		recursiveDelete("/dev_hdd0/plugins/RouLetteVshMenu");
		sysFsUnlink((char*)"/dev_hdd0/plugins/akari_vsh_menu.sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/EBOOTLoader.sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/fps_counter.sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/fps_counter.yaml");
		sysFsUnlink((char*)"/dev_hdd0/plugins/gameboot_cycle.sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/openps3ftp.sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/PS3_PPU_Project2.1.2.sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/raibow_xmb_complete_ccapi.sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/rainbow_xmb.sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/psnpatch.sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/psnpatch.off");
		sysFsUnlink((char*)"/dev_hdd0/plugins/rainbow_xmb.p3t");
		sysFsUnlink((char*)"/dev_hdd0/plugins/roulette_vsh_menu.sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/slaunch.sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/trophy_unlocker.sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/TrophyUnlocker (DPAD LEFT).sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/xmb_plugin.sprx");
		sysFsUnlink((char*)"/dev_hdd0/plugins/xmb_plugin");
		sysFsUnlink((char*)"/dev_hdd0/plugins/akari_vsh_menu.off");
		sysFsUnlink((char*)"/dev_hdd0/plugins/EBOOTLoader.off");
		sysFsUnlink((char*)"/dev_hdd0/plugins/fps_counter.off");
		sysFsUnlink((char*)"/dev_hdd0/plugins/gameboot_cycle.off");
		sysFsUnlink((char*)"/dev_hdd0/plugins/openps3ftp.off");
		sysFsUnlink((char*)"/dev_hdd0/plugins/rainbow_xmb.off");
		sysFsUnlink((char*)"/dev_hdd0/plugins/rainbow_xmb.p3t");
		sysFsUnlink((char*)"/dev_hdd0/plugins/roulette_vsh_menu.off");
		sysFsUnlink((char*)"/dev_hdd0/plugins/slaunch.off");
		sysFsUnlink((char*)"/dev_hdd0/plugins/trophy_unlocker.off");
		sysFsUnlink((char*)"/dev_hdd0/plugins/xmb_plugin.off");
		
		//Deletes files installed in dev_hdd0 that are not deleted by deleting the pkg
		recursiveDelete("/dev_hdd0/PROISO");
		recursiveDelete("/dev_hdd0/download/theme");
		recursiveDelete("/dev_hdd0/game/PS3ONLINE/USRDIR/settings/installedplugins");
		recursiveDelete("/dev_hdd0/game_debug");
		recursiveDelete("/dev_hdd0/hen/themes");
		recursiveDelete("/dev_hdd0/tmp/prep_icons");
		recursiveDelete("/dev_hdd0/xmlhost.off");
		sysFsUnlink((char*)"/dev_hdd0/autoexec.bat");
		sysFsUnlink((char*)"/dev_hdd0/boot_init.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_init_swap.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_plugins.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_plugins_firmware_change.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_plugins_kernel_nocobra.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_plugins_kernel_nocobra_dex.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_plugins_nocobra.txt");
		sysFsUnlink((char*)"/dev_hdd0/boot_plugins_nocobra_dex.txt");
		sysFsUnlink((char*)"/dev_hdd0/ingame.bat");
		sysFsUnlink((char*)"/dev_hdd0/onxmb.bat");
		sysFsUnlink((char*)"/dev_hdd0/patches.bat");
		sysFsUnlink((char*)"/dev_hdd0/ps3-updatelist.txt");
		sysFsUnlink((char*)"/dev_hdd0/tmp/wm_icons/icon_wm_root.png");
		sysFsUnlink((char*)"/dev_hdd0/tmp/wm_res/npsignin_plugin.rco");

		//Delete webMAN's random XMB™ files
		recursiveDelete("/dev_hdd0/tmp/canyon");
		recursiveDelete("/dev_hdd0/tmp/canyon.off");
		recursiveDelete("/dev_hdd0/tmp/clock");
		recursiveDelete("/dev_hdd0/tmp/clock.off");
		recursiveDelete("/dev_hdd0/tmp/coldboot");
		recursiveDelete("/dev_hdd0/tmp/coldboot.off");
		recursiveDelete("/dev_hdd0/tmp/earth");
		recursiveDelete("/dev_hdd0/tmp/earth.off");
		recursiveDelete("/dev_hdd0/tmp/gameboot");
		recursiveDelete("/dev_hdd0/tmp/gameboot.off");
		recursiveDelete("/dev_hdd0/tmp/gameboot_logo");
		recursiveDelete("/dev_hdd0/tmp/gameboot_logo.off");
		recursiveDelete("/dev_hdd0/tmp/impose");
		recursiveDelete("/dev_hdd0/tmp/impose.off");
		recursiveDelete("/dev_hdd0/tmp/lines");
		recursiveDelete("/dev_hdd0/tmp/lines.off");
		recursiveDelete("/dev_hdd0/tmp/psn_icons");
		recursiveDelete("/dev_hdd0/tmp/psn_icons.off");
		recursiveDelete("/dev_hdd0/tmp/system_music");
		recursiveDelete("/dev_hdd0/tmp/theme");
		recursiveDelete("/dev_hdd0/tmp/theme.off");
		recursiveDelete("/dev_hdd0/tmp/wallpaper");
		recursiveDelete("/dev_hdd0/tmp/wallpaper.off");
		
		//Disable webMAN plugins
		recursiveDelete("/dev_hdd0/tmp/wm_res");
		sysFsMkdir((char*)"/dev_hdd0/tmp/wm_res", 0777);

		//Enable Cobra payload if disabled
		sysLv2FsRename("/dev_blind/rebug/cobra/stage2.cex.bak","/dev_blind/rebug/cobra/stage2.cex");
		sysLv2FsRename("/dev_blind/rebug/cobra/stage2.dex.bak","/dev_blind/rebug/cobra/stage2.dex");
		sysLv2FsRename("/dev_blind/sys/stage2.bin.bak","/dev_blind/sys/stage2.bin");
		sysLv2FsRename("/dev_blind/sys/stage2.bin.bak","/dev_blind/sys/stage2.bin");
		sysLv2FsRename("/dev_blind/sys/stage2.cex.bak","/dev_blind/sys/stage2.cex");
		sysLv2FsRename("/dev_blind/sys/stage2.dex.bak","/dev_blind/sys/stage2.dex");
		sysLv2FsRename("/dev_blind/spy/cobra/stage2.bin.bak","/dev_blind/spy/cobra/stage2.bin");
		
		//Revert default off features
		sysLv2FsRename("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/online_server/warning.xml","/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/online_server/warning.off");
		sysLv2FsRename("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/webman/sub_items/rebug_tools_items.xml","/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/webman/sub_items/rebug_tools_items.off");
		
		//Rename to avoid HEN copying the wrong folder
		sysLv2FsRename("/dev_hdd0/game/PS34KPROX/USRDIR/toolbox/patches/boot_plugins","/dev_hdd0/game/PS34KPROX/USRDIR/toolbox/patches/boot_plugins.off");
		sysLv2FsRename("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/boot_plugins.off","/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/patches/boot_plugins");
		
		//Checks for enabling/disabling specific features
		if (exists("/dev_flash/rebug")==0)
		{	
			sysLv2FsRename("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/webman/sub_items/rebug_tools_items.off","/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/webman/sub_items/rebug_tools_items.xml");
		}
		else if (exists("/dev_flash/vsh/module/vsh.self.dex") == 0 || exists("/dev_flash/vsh/module/vsh.self.cex") == 0)
		{	
			sysLv2FsRename("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/webman/sub_items/rebug_tools_items.xml","/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/webman/sub_items/rebug_tools_items.off");
		}
		else
		{
			sysLv2FsRename("/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/webman/sub_items/rebug_tools_items.xml","/dev_hdd0/game/PS34KPROL/USRDIR/toolbox/xmls/webman/sub_items/rebug_tools_items.off");
		}

		//Delete what's new items
		recursiveDelete("/dev_hdd0/tmp/explore/");
	
		//Avoid issues with this file
		sysFsUnlink((char*)"/dev_hdd0/tmp/wm_combo/wm_custom_r2_square");
		sysFsMkdir((char*)"/dev_hdd0/tmp/wm_combo/", 0777);
		create_file("/dev_hdd0/tmp/wm_combo/wm_custom_r2_square");
		sysFsChmod((char*)"/dev_hdd0/tmp/wm_combo/wm_custom_r2_square", 0777);
		sysFsUnlink((char*)"/dev_hdd0/tmp/wm_combo/wm_custom_r2_square");
				
		ret=copy_prepare(appfolder, "backup", foldername, firmware_folder, app_choice);
		if (ret == "") //backup success
		{
		
			//Delete unnecessary backups
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 4K Pro Lite/PS3~dev_hdd0/game/PS34KPROL");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 4K Pro Lite/PS3~dev_hdd0~game~PS34KPROL~USRDIR~toolbox~patches~welcome_screen");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 Pro Lite/PS3~dev_hdd0/game/PS34KPROL");
			recursiveDelete("/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 Pro Lite/PS3~dev_hdd0~game~PS34KPROL~USRDIR~toolbox~patches~welcome_screen");
			
			//Free up space and backup default users
			recursiveDelete("/dev_blind/vsh/resource/explore/user");
			sysFsMkdir((char*)"/dev_blind/vsh/resource/explore/user", 0777);
			
			// Reset Patches
			sysFsUnlink((char*)"/dev_blind/vsh/resource/npsignin_plugin.off");
			sysFsUnlink((char*)"/dev_blind/vsh/resource/npsignin_plugin.lck");
			
			//Delete old installer files
			sysFsUnlink((char*)"/dev_hdd0/home/settings.xml");
			sysFsUnlink((char*)"/dev_hdd0/home/optimize.xml");
			
			ret=copy_prepare(appfolder, "install", "", firmware_folder, app_choice);
			if (ret == "") //copy success
			{	
				create_file("/dev_blind/vsh/etc/lite.txt");

				//Delete unused xmls
				sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_friend_shop.xml");
				sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_game_tool2.xml");
				sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_network_tool2.xml");
				sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_sysconf_shop.xml");
				sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_user_shop.xml");
				sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_video_bdponly.xml");
				sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/category_widget.xml");
				sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/upload_list.xml");
				sysFsUnlink((char*)"/dev_blind/vsh/resource/explore/xmb/videodownloader_list.xml");
				
				string firmware_type=get_firmware_info("type");
				
				//Delete cobra backup to avoid issues with plugins that requires newer versions
				sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 4K Pro Lite/PS3~dev_blind~rebug~cobra/stage2.cex");
				sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 4K Pro Lite/PS3~dev_blind~rebug~cobra/stage2.dex");
				sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 4K Pro Lite/PS3~dev_blind~sys/stage2.bin");
				sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 4K Pro Lite/PS3~dev_blind~sys/stage2.cex");
				sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 4K Pro Lite/PS3~dev_blind~sys/stage2.dex");
				sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 4K Pro Lite/PS3~dev_blind~spy/cobra/stage2.bin");
				
				sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 Pro Lite/PS3~dev_blind~rebug~cobra/stage2.cex");
				sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 Pro Lite/PS3~dev_blind~rebug~cobra/stage2.dex");
				sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 Pro Lite/PS3~dev_blind~sys/stage2.bin");
				sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 Pro Lite/PS3~dev_blind~sys/stage2.cex");
				sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 Pro Lite/PS3~dev_blind~sys/stage2.dex");
				sysFsUnlink((char*)"/dev_hdd0/game/PS34KPROL/USRDIR/backup/Uninstall PS3 Pro Lite/PS3~dev_blind~spy/cobra/stage2.bin");
				
				//Delete dummy vsh files
				deleteIfZeroBytes("/dev_blind/vsh/module/vsh.self.cex");
				deleteIfZeroBytes("/dev_blind/vsh/module/vsh.self.cexsp");
				deleteIfZeroBytes("/dev_blind/vsh/module/vsh.self.dex");
				deleteIfZeroBytes("/dev_blind/vsh/module/vsh.self.dexsp");
				deleteIfZeroBytes("/dev_blind/vsh/module/vsh.self.nrm");
				deleteIfZeroBytes("/dev_blind/vsh/module/vsh.self.swp");
				
				//Allow vsh mode change
				sysLv2FsRename("/dev_blind/vsh/module/vsh.self.dexsp","/dev_blind/vsh/module/vsh.self.dex");
				sysLv2FsRename("/dev_blind/vsh/module/vsh.self.cexsp","/dev_blind/vsh/module/vsh.self.cex");
				
				if (firmware_type == "CEX")
				{
					sysLv2FsRename("/dev_blind/vsh/resource/explore/xmb/category_game_folders/ps2_classics_dex_warning.xml","/dev_blind/vsh/resource/explore/xmb/category_game_folders/ps2_classics_dex_warning.off");
				}
				else if (firmware_type == "DEX")
				{
					sysLv2FsRename("/dev_blind/vsh/resource/explore/xmb/category_game_folders/ps2_classics_dex_warning.off","/dev_blind/vsh/resource/explore/xmb/category_game_folders/ps2_classics_dex_warning.xml");
				}
				
				//Delete webMAN on flash
				sysFsUnlink((char*)"/dev_blind/vsh/module/webftp_server.sprx");
				
				/* if (is_dev_blind_mounted()==0)
				{
					unmount_dev_blind();
					//Mess.Dialog(MSG_YESNO_DYES, ("The "+app_choice+" has been installed with success.\nThe system files have been modified, do you want to reboot the system?").c_str());
					Mess.Dialog(MSG_YESNO_DYES, ("The "+variant+" has been installed with success.\nThe system files have been modified, do you want to reboot the system?").c_str());
					
					if (Mess.GetResponse(MSG_DIALOG_BTN_YES)==1) return 2;else 
					{
						Graphics->AppExit();
						return 1;
					}
				} */
				
				single_beep();
				setLed("green_default");
				
				Mess.Dialog(MSG_OK, ("The "+variant+" has been installed with success\nPress OK to continue.").c_str());
				setLed("green_default");
				Mess.Dialog(MSG_YESNO_DYES, ("Would you like to optimize the system by rebuilding the database?\n\nThis process will delete the following content:\n* Chat rooms and messages.\n* Custom information for Photos/Music/Game/Video and album names.\n* Music playlists and Video playback history, resume information and thumbnails.\n\nThis process is recommended to improve boot times for applications and system performance."));
				if (Mess.GetResponse(MSG_DIALOG_BTN_YES)==1) 
				{
					rebuild_db();
					create_file("/dev_hdd0/vm/optimize.xml");
					return 2;
				}
				else
				{
					return 2;
				}
			}
			else //problem in the copy process so rollback by restoring the backup
			{
				//Mess.Dialog(MSG_ERROR,("The "+app_choice+" has not been installed!\nAn error occured while copying files!\n\nError: "+ret+"\n\nThe backup will be restored.").c_str());
				
				triple_beep();
				setLed("red_blink_fast");
				
				if (exists_backups(appfolder)==0)
				{
					Mess.Dialog(MSG_ERROR,("The "+variant+" has not been installed!\nAn error occured while copying files!\n\nError: "+ret+"\n\nThe backup will be restored.\nPress OK to continue.").c_str());
					return restore(appfolder, foldername);
				}
				else
				{
					Mess.Dialog(MSG_ERROR,("The "+variant+" has not been installed!\nAn error occured while copying files!\n\nError: "+ret+"\n\nPlease reinstall the system firmware.\nPress OK to exit.").c_str());
					Graphics->AppExit();
				}
			}
		}
		else //problem in the backup process so rollback by deleting the backup
		{
			triple_beep();
			setLed("red_blink_fast");
			
			//Mess.Dialog(MSG_ERROR,("The "+app_choice+" has not been installed!\nAn error occurred while backing up the files.!\n\nError: "+ret+"\n\nThe incomplete backup will be deleted.").c_str());
			Mess.Dialog(MSG_ERROR,("The "+variant+" has not been installed!\nAn error occurred while backing up the files.!\n\nError: "+ret+"\n\nThe incomplete backup will be deleted.").c_str());
			if (recursiveDelete(appfolder+"/backup/"+foldername) != "") Mess.Dialog(MSG_ERROR,("There was a problem deleting the backup!\n\nError: "+ret+"\n\nTry deleting with a file manager.").c_str());
		}
	}

	return 0;
}

int delete_all(string appfolder)
{
	string ret="";
	string problems="\n\nPlease be advised that by deleting the backup files, it will not be possible to restore the original files. To return the system to its original state, you will need to perform a firmware reinstallation.";

	Mess.Dialog(MSG_YESNO_DNO,("Are you sure you want to delete all backups?"+problems).c_str());
	if (Mess.GetResponse(MSG_DIALOG_BTN_YES)==1)
	{
		ret=recursiveDelete(appfolder+"/backup");
		if (ret == "") //delete sucess
		{
			Mess.Dialog(MSG_OK,"All backups have been deleted!\nPress OK to continue.");
			return 1;
		}
		else //problem in the delete process so emit a warning
		{
			Mess.Dialog(MSG_ERROR,("The backup folders were not deleted!\nAn error occured while deleting the folders!\n\nError: "+ret+"\n\nTry to delete again manually, if the error persists, try other software to delete this folders.").c_str());
		}
	}
	return 0;
}

int delete_one(string appfolder, string foldername, string type)
{
	string ret="";
	string problems="\n\nPlease be advised that by deleting the backup file, it will not be possible to restore the original files. To return the system to its original state, you will need to perform a firmware reinstallation.";
	//Mess.Dialog(MSG_YESNO_DNO,("Are you sure you want to delete the "+type+" '"+foldername+"'?"+problems).c_str());
	Mess.Dialog(MSG_YESNO_DNO,("Are you sure you want to delete the backup of the system?"+problems).c_str());
	if (Mess.GetResponse(MSG_DIALOG_BTN_YES)==1)
	{
		if (strcmp(type.c_str(), "backup") == 0) ret = recursiveDelete(appfolder + "/backup/" + foldername);
		else if (strcmp(type.c_str(), "app") == 0) 
		{
			if (jailbreak == "Custom Firmware") ret = recursiveDelete(appfolder + "/app/install/cfw/" + foldername);
			else if (jailbreak == "Hybrid Firmware") ret = recursiveDelete(appfolder + "/app/install/hfw/" + foldername);
		}

		if (ret == "") //delete sucess
		{
			//Mess.Dialog(MSG_OK,("The "+type+" of "+foldername+" has been deleted!\nPress OK to continue.").c_str());
			Mess.Dialog(MSG_OK, "The backup of the system has been deleted!\nPress OK to continue.");
			return 1;
		}
		else //problem in the delete process so emit a warning
		{
			//Mess.Dialog(MSG_ERROR,("An error occured while deleting the "+type+" of "+foldername+"!\n\nError: "+ret+"\n\nTry to delete again manually, if the error persists, try other software to delete this folders.").c_str());
			Mess.Dialog(MSG_ERROR,("An error occured while deleting the backup of the system!\n\nError: "+ret+"\n\nTry to delete again manually, if the error persists, try other software to delete this folders.").c_str());
		}
	}
	return 0;
}

int make_menu_to_array(string appfolder, int whatmenu, string vers, string type)
{
	int ifw=0, iapp=0, ibackup=0;
	DIR *dp, *dp2;
	struct dirent *dirp, *dirp2;
	string direct, direct2;

	if (whatmenu==1 || whatmenu==2 || whatmenu==0)
	{
		iapp=0;
		if (jailbreak == "Custom Firmware")  direct=appfolder+"/app/install/cfw";
		else if (jailbreak == "Hybrid Firmware") direct=appfolder+"/app/install/hfw";

		dp = opendir (direct.c_str());
		if (dp == NULL) return -1;
		while ( (dirp = readdir(dp) ) )
		{
			if ( strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0 && dirp->d_type == DT_DIR)
			{
				//second menu
				ifw=0;
				direct2=direct+"/"+dirp->d_name;
				dp2 = opendir (direct2.c_str());
				if (dp2 == NULL) return -1;
				while ( (dirp2 = readdir(dp2) ) )
				{
					if ( strcmp(dirp2->d_name, ".") != 0 && strcmp(dirp2->d_name, "..") != 0 && strcmp(dirp2->d_name, "") != 0 && dirp2->d_type == DT_DIR)
					{
						string fwfolder=(string)dirp2->d_name;
						string app_fwv=fwfolder.substr(0,fwfolder.find("-"));
						string app_fwt=fwfolder.substr(app_fwv.size()+1,fwfolder.rfind("-")-app_fwv.size()-1);
						string app_fwc=fwfolder.substr(app_fwv.size()+1+app_fwt.size()+1);
						if ((strcmp(app_fwv.c_str(), vers.c_str())==0 || strcmp(app_fwv.c_str(), "All")==0) && (strcmp(app_fwt.c_str(), type.c_str())==0 || strcmp(app_fwt.c_str(), "All")==0))
						{
							//Mess.Dialog(MSG_OK,(app_fwv+"-"+vers+"||"+app_fwt+"-"+type+"||"+app_fwc).c_str());
							menu2[iapp][ifw]=app_fwc;
							menu2_path[iapp][ifw]=dirp2->d_name;
							ifw++;
						}
					}
				}
				closedir(dp2);
				if (ifw>0) //has apps for the current firmware version
				{
					menu2[iapp][ifw]="Main Menu";
					ifw++;
					menu2[iapp][ifw]="\0";
					menu1[iapp]=dirp->d_name;
					iapp++;
				}
			}
		}
		closedir(dp);
		//print(("iapp:"+int_to_string(iapp)+"\n").c_str());
		if (iapp>0)
		{
			// menu1[iapp]="Backup";
			menu1[iapp]=" ";
			iapp++;
			menu1[iapp]="Exit";
			iapp++;
			menu1[iapp]="\0";
		}
	}
	if (whatmenu==3 || whatmenu==0)
	{
		ibackup=0;
		direct=appfolder+"/backup";
		if (exists_backups(appfolder)==0)
		{
			dp = opendir(direct.c_str());
			if (dp == NULL) return -1;
			while ( (dirp = readdir(dp) ) )
			{
				if ( strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0 && dirp->d_type == DT_DIR)
				{
					menu3[ibackup]=dirp->d_name;
					ibackup++;
				}
			}
			closedir(dp);
			if (ibackup>0)
			{
				//menu3[ibackup]="Main Menu";
				menu3[ibackup]="Exit";
				ibackup++;
				menu3[ibackup]="\0";
			}
			else recursiveDelete(direct);
		}
	}
	return 0;
}

void bitmap_menu(string fw_version, string ttype, int menu_id, int msize, int selected, int choosed, int menu1_pos, int menu1_restore)
{
	bitmap_background(jailbreak, fw_version, ttype, currentDateTime(), buildNumber());
	int j, tposy=png_logo.height+ypos(30)+ypos(30), posy=tposy+ypos(30), sizeTitleFont = ypos(40), sizeFont = ypos(30), spacing=ypos(4);
	int start_at=0, end_at=0, roll_at=0, dynamic_menu_end=0;
	string menu1_text;

	//dynamic menu calculations
	if (menu_id==1) dynamic_menu_end=msize-3;
	else if (menu_id==2) dynamic_menu_end=msize-2;
	else if (menu_id==3) dynamic_menu_end=msize-2;
	if (menu_id==1) roll_at=MENU_ROLL_OPTIONS;
	else roll_at=SUBMENU_ROLL_OPTIONS;

	if (dynamic_menu_end+1>roll_at)
	{
		if (selected<=1) start_at=0; //two first options
		else if (selected>dynamic_menu_end-roll_at+1) start_at=dynamic_menu_end-roll_at+1; //two last options
		else start_at=selected-1;
		end_at=start_at+roll_at-1;
		if (end_at>=dynamic_menu_end) end_at=dynamic_menu_end;
		if (start_at!=0) ISUp.AlphaDrawIMGtoBitmap(xpos(950),tposy+sizeFont+spacing,&png_scroll_up,&Menu_Layer);
		if (end_at!=dynamic_menu_end) ISDown.AlphaDrawIMGtoBitmap(xpos(950),tposy+roll_at*(sizeFont+spacing),&png_scroll_down,&Menu_Layer);
	}
	else end_at=dynamic_menu_end;

	if (menu_id==1)
	{
		F1.PrintfToBitmap(center_text_x(sizeTitleFont, " System_Jailbreak_Type"),tposy,&Menu_Layer, 0xff0000, sizeTitleFont, "System Jailbreak Type");
		//dynamic menu
		for(j=start_at;j<=end_at;j++)
		{
			posy=posy+sizeFont+spacing;
			F2.PrintfToBitmap(center_text_x(sizeFont, menu1[j].c_str()),posy,&Menu_Layer,menu_text_color(j, selected, choosed,-1,-1),sizeFont, "%s",menu1[j].c_str());
		}
		//static menu
		F2.PrintfToBitmap(center_text_x(sizeFont, menu1[msize-2].c_str()),ypos(550)-(2*(sizeFont+spacing)),&Menu_Layer,menu_text_color(msize-2, selected, choosed,0,menu1_restore),sizeFont, "%s",menu1[msize-2].c_str());
		F2.PrintfToBitmap(center_text_x(sizeFont, menu1[msize-1].c_str()),ypos(550),&Menu_Layer,menu_text_color(msize-1, selected, choosed,-1,-1),sizeFont, "%s",menu1[msize-1].c_str());
		//buttons
		if (selected<msize-2)
		{
			
		//Main menu buttons
		IBCross.AlphaDrawIMGtoBitmap(xpos(90),ypos(570),&png_button_cross,&Menu_Layer);
		if (selected<msize-2) F2.PrintfToBitmap(xpos(90)+png_button_cross.width+xpos(10),ypos(570)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Select");
		else F2.PrintfToBitmap(xpos(90)+png_button_cross.width+xpos(10),ypos(570)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Select");
		
		IBTriangle.AlphaDrawIMGtoBitmap(xpos(90),ypos(610),&png_button_triangle,&Menu_Layer);
		F2.PrintfToBitmap(xpos(90)+png_button_triangle.width+xpos(10),ypos(610)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Information");
		}
		else
		{
			IBCross.AlphaDrawIMGtoBitmap(xpos(90),ypos(610),&png_button_cross,&Menu_Layer);
			F2.PrintfToBitmap(xpos(90)+png_button_cross.width+xpos(10),ypos(610)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Select");
		}
	}
	else if (menu_id==2)
	{
		F1.PrintfToBitmap(center_text_x(sizeTitleFont, "Install_Version"),tposy,&Menu_Layer, 0x2828ca, sizeTitleFont, "Install Version");
		//dynamic menu
		for(j=start_at;j<=end_at;j++)
		{
			posy=posy+sizeFont+spacing;
			F2.PrintfToBitmap(center_text_x(sizeFont, menu2[menu1_pos][j].c_str()),posy,&Menu_Layer,menu_text_color(j, selected, choosed,-1,-1),sizeFont, "%s",menu2[menu1_pos][j].c_str());
		}
		//static menu
		F2.PrintfToBitmap(center_text_x(sizeFont, menu2[menu1_pos][msize-1].c_str()),ypos(550),&Menu_Layer,menu_text_color(msize-1, selected, choosed,-1,-1),sizeFont, "%s",menu2[menu1_pos][msize-1].c_str());
		//buttons	
		if (selected<msize-1)
		{
			IBCross.AlphaDrawIMGtoBitmap(xpos(90),ypos(490),&png_button_cross,&Menu_Layer);
			F2.PrintfToBitmap(xpos(90)+png_button_cross.width+xpos(10),ypos(490)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Install");
			IBSquare.AlphaDrawIMGtoBitmap(xpos(90),ypos(530),&png_button_square,&Menu_Layer);
			F2.PrintfToBitmap(xpos(90)+png_button_square.width+xpos(10),ypos(530)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "About");
			IBTriangle.AlphaDrawIMGtoBitmap(xpos(90),ypos(570),&png_button_triangle,&Menu_Layer);
			F2.PrintfToBitmap(xpos(90)+png_button_triangle.width+xpos(10),ypos(570)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Credits");
			IBCircle.AlphaDrawIMGtoBitmap(xpos(90),ypos(610),&png_button_circle,&Menu_Layer);
			F2.PrintfToBitmap(xpos(90)+png_button_circle.width+xpos(10),ypos(610)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Back");
		}
		else
		{
			IBCross.AlphaDrawIMGtoBitmap(xpos(90),ypos(570),&png_button_cross,&Menu_Layer);
			F2.PrintfToBitmap(xpos(90)+png_button_cross.width+xpos(10),ypos(570)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Select");
			IBCircle.AlphaDrawIMGtoBitmap(xpos(90),ypos(610),&png_button_circle,&Menu_Layer);
			F2.PrintfToBitmap(xpos(90)+png_button_circle.width+xpos(10),ypos(610)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Back");
		}
	}
	else if (menu_id==3)
	{
		F1.PrintfToBitmap(center_text_x(sizeTitleFont, "Restore___Backup"),tposy,&Menu_Layer, 0x3d85c6, sizeTitleFont, "Restore Backup");
		//dynamic menu
		for(j=start_at;j<=end_at;j++)
		{
			posy=posy+sizeFont+spacing;
			F2.PrintfToBitmap(center_text_x(sizeFont, menu3[j].c_str()),posy,&Menu_Layer,menu_text_color(j, selected, choosed,-1,-1),sizeFont, "%s",menu3[j].c_str());
		}
		//static menu
		F2.PrintfToBitmap(center_text_x(sizeFont, menu3[msize-1].c_str()),ypos(550),&Menu_Layer,menu_text_color(msize-1, selected, choosed,-1,-1),sizeFont, "%s",menu3[msize-1].c_str());
		//buttons
		if (selected<msize-1)
		{
			IBCross.AlphaDrawIMGtoBitmap(xpos(90),ypos(530),&png_button_cross,&Menu_Layer);
			F2.PrintfToBitmap(xpos(90)+png_button_cross.width+xpos(10),ypos(530)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Select");
			IBSquare.AlphaDrawIMGtoBitmap(xpos(90),ypos(570),&png_button_square,&Menu_Layer);
			// F2.PrintfToBitmap(xpos(90)+png_button_square.width+xpos(10),ypos(570)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Delete");
			F2.PrintfToBitmap(xpos(90)+png_button_square.width+xpos(10),ypos(570)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "About");
			IBTriangle.AlphaDrawIMGtoBitmap(xpos(90),ypos(610),&png_button_triangle,&Menu_Layer);
			F2.PrintfToBitmap(xpos(90)+png_button_triangle.width+xpos(10),ypos(610)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Credits");
		}
		else
		{
			IBCross.AlphaDrawIMGtoBitmap(xpos(90),ypos(610),&png_button_cross,&Menu_Layer);
			F2.PrintfToBitmap(xpos(90)+png_button_cross.width+xpos(10),ypos(610)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Select");
		}
	}
	//Fixed Buttons
	IBUp.AlphaDrawIMGtoBitmap(xpos(1050),ypos(570),&png_button_up,&Menu_Layer);
	F2.PrintfToBitmap(xpos(1050)+png_button_up.width+xpos(10),ypos(570)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Up");
	IBDown.AlphaDrawIMGtoBitmap(xpos(1050),ypos(610),&png_button_down,&Menu_Layer);
	F2.PrintfToBitmap(xpos(1050)+png_button_down.width+xpos(10),ypos(610)+sizeFont-ypos(5),&Menu_Layer, COLOR_WHITE, sizeFont, "Down");
}

s32 main(s32 argc, char* argv[])
{
	padInfo2 padinfo2;
	padData paddata;
	
	setLed("green_default");
	
	int menu_restore=-1, menu1_position=0, menu2_position=0, menu3_position=0, mpos=0, reboot=0, temp=0, current_menu=1, msize=0, choosed=0;
	string fw_version, ttype, mainfolder, rtype="hard";
	int oldmsize=msize, oldcurrentmenu=current_menu, oldmpos=mpos;
	PF.printf("Starting program\r\n");
	PF.printf("- Intializing pad control\r\n");
	ioPadInit(MAX_PORT_NUM); //this will initialize the controller (7= seven controllers)
	mainfolder=get_app_folder(argv[0]);
	fw_version=get_firmware_info("version");
	ttype=get_firmware_info("type");

	if (jailbreak == "Hybrid Firmware")
	{	
		if (fw_version >= "4.93") 
		{	
			setLed("red_blink_fast");
			triple_beep();

			Mess.Dialog(MSG_ERROR, ("Error:\nThe system firmware version "+fw_version+" is not yet supported.\nCheck for updates on the Telegram group @PS34KPro regarding the compatibility of the PS3™ 4K Pro with the latest firmware.").c_str());
			goto end; 
		}
		
		if (fw_version < "4.86") 
		{
			setLed("red_blink_fast");
			triple_beep();

			Mess.Dialog(MSG_ERROR, ("Error:\nThe system firmware version "+fw_version+" is outdated and not supported.\nPlease consider updating the firmware to version 4.86 or later, then try again.").c_str());
			goto end; 
		}
		
		if (ttype == "DEX")
		{
			setLed("red_blink_fast");
			triple_beep();

			Mess.Dialog(MSG_ERROR, ("Error:\nThe use of Hybrid Firmware in DEX mode is uncommon and, therefore, not fully compatible with the HEN exploit yet. For updates on the compatibility of HEN with hybrid firmware in DEX mode, check PSX-PLACE. For updates on the compatibility of the PS3™ 4K Pro with the firmware, check the Telegram group @PS34KPro."));
			goto end; 
		}
	}
	else if (jailbreak == "Custom Firmware")
	{
		if (fw_version >= "4.93") 
		{
			setLed("red_blink_fast");
			triple_beep();

			Mess.Dialog(MSG_ERROR, ("Error:\nThe system firmware version "+fw_version+" is not yet supported.\nPlease consider downgrading the firmware to version 4.91 or lower, then try again.\n\nCheck for updates on the Telegram group @PS34KPro regarding the compatibility of the PS3™ 4K Pro with the latest firmware.").c_str());
			goto end; 
		}
		
		if (fw_version < "4.84") 
		{
			setLed("red_blink_fast");
			triple_beep();
			
			Mess.Dialog(MSG_ERROR, ("Error:\nThe system firmware version "+fw_version+" is outdated and not supported.\nPlease consider updating the firmware to version 4.84 or later, then try again.").c_str());
			goto end; 
		}
	}

	if (check_current_state(mainfolder)==0) goto end;
	menu_restore=exists_backups(mainfolder);
	PF.printf(("- Getting main app folder: "+mainfolder+"\r\n").c_str());
	PF.printf("- Showing terms\r\n");
	PF.printf("- Detecting firmware changes\r\n");
	check_firmware_changes(mainfolder);
	PF.printf(("- Getting firmware info "+fw_version+" ("+ttype+")\r\n").c_str());
	PF.printf("- Constructing menu\r\n");
	if (make_menu_to_array(mainfolder, 0,fw_version, ttype)!=0) { Mess.Dialog(MSG_ERROR,"Error: There was a problem reading the folder!"); goto end; }
	PF.printf("- Testing  if firmware is supported\r\n");
	if (string_array_size(menu1)==0) { Mess.Dialog(MSG_ERROR, ("Error: The system firmware version "+fw_version+" is not supported.").c_str()); goto end; }
	if (check_terms(mainfolder)!=0) goto end;
	if (check_firmware_warning(mainfolder)!=0) goto end;
	PF.printf("Drawing menu\r\n");
	bitmap_inititalize(int_to_string(Graphics->height)+"p",mainfolder);
	bitmap_menu(fw_version, ttype, current_menu, string_array_size(menu1), mpos, 0, menu1_position, menu_restore);
	Graphics->AppStart();
	while (Graphics->GetAppStatus())
	{
		oldmsize=msize;
		oldcurrentmenu=current_menu;
		oldmpos=mpos;
		
		if (exists("/dev_flash/vsh/resource/explore/xmb/pro.xml")==0)
		{
			//If it's intalled and a backup exists, switch to uninstall mode
			if (exists_backups(mainfolder)==0) 
			{
				current_menu=3;
				msize=string_array_size(menu3);
				mpos=menu3_position;
			}
		}
		
		if (current_menu==1)
		{
			msize=string_array_size(menu1);
			mpos=menu1_position;
		}
		else if (current_menu==2)
		{
			msize=string_array_size(menu2[menu1_position]);
			mpos=menu2_position;
		}
		else if (current_menu==3)
		{
			msize=string_array_size(menu3);
			mpos=menu3_position;
		}
		if (msize!=oldmsize || current_menu!=oldcurrentmenu || mpos!=oldmpos || choosed==1)
		{
			bitmap_menu(fw_version, ttype, current_menu, msize, mpos, 0, menu1_position, menu_restore);
			choosed=0;
		}
		draw_menu(choosed); usleep(1666);
		if (ioPadGetInfo2(&padinfo2)==0)
		{
			for(int i=0;i<MAX_PORT_NUM;i++)
			{
				if (padinfo2.port_status[i])
				{
					//Main Menu
					ioPadGetData(i, &paddata);
					if (current_menu==1)
					{
						unmount_dev_blind();
						
						if (paddata.BTN_DOWN || paddata.ANA_L_V == 0x00FF || paddata.ANA_R_V == 0x00FF)
						{
							if (menu1_position<msize-1)
							{
								menu1_position++;
/* Removed backups menu */	if (menu1_position==msize-2 /*&& menu_restore!=0*/) { menu1_position++; }
							}
							else menu1_position=0;
						}
						else if (paddata.BTN_UP || paddata.ANA_L_V == 0x0000 || paddata.ANA_R_V == 0x0000)
						{
							if (menu1_position>0)
							{
								menu1_position--;
/* Removed backups menu */	if (menu1_position==msize-2 /*&& menu_restore!=0*/) { menu1_position--; }
							}
							else menu1_position=msize-1;
						}
						else if (paddata.BTN_CROSS) //Install an app
						{
							choosed=1;
							bitmap_menu(fw_version, ttype, current_menu, msize, mpos, choosed, menu1_position, menu_restore);
							draw_menu(choosed);
							if (menu1_position<msize-2)
							{
								if (menu2[menu1_position][0]=="All" && string_array_size(menu2[menu1_position])==2)
								{
									draw_menu(choosed);
									temp=install(mainfolder,menu2_path[menu1_position][0], menu1[menu1_position]);
									if (temp==2)
									{
										reboot=1;
										Graphics->AppExit();
									}
									else if (temp==1)
									{
										if (make_menu_to_array(mainfolder, 3,fw_version, ttype)!=0)
										{
											Mess.Dialog(MSG_ERROR,"Error: There was a problem reading the folder!");
											Graphics->AppExit();
										}
										menu_restore=exists_backups(mainfolder);
									}
								}
								else current_menu=2;
							}
							else if (menu1_position<msize-1) current_menu=3;
							else if (menu1_position<msize) Graphics->AppExit();
						}
						
						else if (paddata.BTN_TRIANGLE)
						{
							if (menu1_position < msize - 2)
							{
								string compatibility = "";
								string jailbreak_type = "";
								string compatibility_information = "";
								
								if (jailbreak == "Hybrid Firmware")
								{
									jailbreak_type = jailbreak+": "+fw_version+" "+ttype+" with HEN "+hen_version+" + "+payload+payload_version+" payload.";
									
									if (fw_version > "4.85" && fw_version < "4.89")
									{
										compatibility = "completely";
										compatibility_information = "\n\nIf you encounter any issues or have suggestions, please feel free to contact @LuanTeles at PSX-Place or via the @PS34KPro group on Telegram.";
									}
									else if (fw_version >= "4.89")
									{
										compatibility = "partially";
										compatibility_information = "\n\nPlease be advised that there may be minor visual bugs in your firmware version due to the changes made by Sony, these should not significantly affect your user experience. If you encounter any issues or have suggestions, please feel free to contact @LuanTeles at PSX-Place or via the @PS34KPro group on Telegram.";
									}
								}
								else if (jailbreak == "Custom Firmware")
								{
									jailbreak_type = jailbreak+" ("+gpu_clock_speed+"): "+fw_version+" "+ttype+" with "+payload+payload_version+" payload.";
									compatibility = "completely";
									compatibility_information = "\n\nIf you encounter any issues or have suggestions, please feel free to contact @LuanTeles at PSX-Place or via the @PS34KPro group on Telegram.";
								}

								string information = "Compatibility information:\nThe PS3™ 4K Pro is "+compatibility+" compatible with your system, which is currently running:\n\n• ";
								Mess.Dialog(MSG_OK, (information+jailbreak_type+compatibility_information).c_str());
							}
						}
							 
					//Disabled to Avoid the Deletion of the App
							/* else if (paddata.BTN_SQUARE) //Delete an app
						 {
							if (menu1_position<msize-2)
							{
								choosed=1;
											 
								bitmap_menu(fw_version, ttype, current_menu, msize, mpos, choosed, menu1_position, menu_restore);
								draw_menu(choosed);
								temp=delete_one(mainfolder, menu1[menu1_position], "app");
								if (temp==1)
								{
									if (make_menu_to_array(mainfolder, 1,fw_version, ttype)!=0)
									{
										Mess.Dialog(MSG_ERROR,"Error: There were a problem reading the folder!");
										Graphics->AppExit();
									}
								}
							}
						} */
					}
					else if (current_menu==2)
					{
						unmount_dev_blind();
						
						if (paddata.BTN_CIRCLE) { current_menu=1; }
						else if (paddata.BTN_DOWN || paddata.ANA_L_V == 0x00FF || paddata.ANA_R_V == 0x00FF)
						{
							if (menu2_position<msize-1) { menu2_position++; }
							else menu2_position=0;
						}
						else if (paddata.BTN_UP || paddata.ANA_L_V == 0x0000 || paddata.ANA_R_V == 0x0000)
						{
							if (menu2_position>0) { menu2_position--; }
							else menu2_position=msize-1;
						}
						else if (paddata.BTN_CROSS) //Install an app
						{
							choosed=1;
							bitmap_menu(fw_version, ttype, current_menu, msize, mpos, choosed, menu1_position, menu_restore);
							draw_menu(choosed);
							if (menu2_position<msize-1)
							{
								draw_menu(choosed);
								temp=install(mainfolder, menu2_path[menu1_position][menu2_position], menu1[menu1_position]);
								if (temp==2)
								{
									reboot=1;
									Graphics->AppExit();
								}
								else if (temp==1)
								{
									if (make_menu_to_array(mainfolder, 3,fw_version, ttype)!=0)
									{
										Mess.Dialog(MSG_ERROR,"Error: There was a problem reading the folder!");
										Graphics->AppExit();
									}
									menu_restore=exists_backups(mainfolder);
									current_menu=1;
								}
							}
							else current_menu=1;
						}
						else if (paddata.BTN_SQUARE)
						{
							if (menu2_position<msize-1)
							{
								Mess.Dialog(MSG_OK, "About:\nThe PS3™ 4K Pro is a project that aims to be an all-in-one solution, covering everything from the basics to the most advanced features. It's available in two variants:\n\n• PS3™ Pro, which is the original version.\n• PS3™ 4K Pro, which was introduced later and quickly became the most popular one.\n\nThey share the same features, differing only in names and logos. It's important to note that, despite the second variant having '4K' in its name, it does not output in that resolution.");
							}
						}
							else if (paddata.BTN_TRIANGLE)
						{
							if (menu2_position<msize-1)
							{
								 {
									Mess.Dialog(MSG_OK, "Credits and acknowledgment:\nThe PS3™ 4K Pro couldn't exist without the help of the PSX-Place community, where I met the some of most amazing people in the scene, whom you may have already heard of, such as:\n\nAldostools, Bguerville, DeviL303, Esc0rtd3w, Pinky, Sandungas, and many others.\n\nAll of them contributed to the project in some way, and their invaluable contributions transformed it from what initially started with just a few visual modifications into what it is today—an all-in-one solution.");
								 }
							}
						}
					}
					else if (current_menu==3)
					{
						unmount_dev_blind();
						
						if (paddata.BTN_CIRCLE) { current_menu=1; }
						else if (paddata.BTN_DOWN || paddata.ANA_L_V == 0x00FF || paddata.ANA_R_V == 0x00FF)
						{
							if (menu3_position<msize-1) { menu3_position++; }
							else menu3_position=0;
						}
						else if (paddata.BTN_UP || paddata.ANA_L_V == 0x0000 || paddata.ANA_R_V == 0x0000)
						{
							if (menu3_position>0) { menu3_position--; }
							else menu3_position=msize-1;
						}
						else if (paddata.BTN_CROSS)
						{
							choosed=1;
							bitmap_menu(fw_version, ttype, current_menu, msize, mpos, choosed, menu1_position, menu_restore);
							draw_menu(choosed);
							if (menu3_position<msize-1) //Restore a backup
							{
								draw_menu(choosed);
								temp=restore(mainfolder, menu3[menu3_position]);
								if (temp==2)
								{
									reboot=1;
									Graphics->AppExit();
								}
							}
							//else current_menu=1;
							else Graphics->AppExit();
						}

/* Avoid user deleting the backup
						else if (paddata.BTN_SQUARE)
						{
							if (menu3_position<msize-1) //Delete a backup
							{
								choosed=1;
								bitmap_menu(fw_version, ttype, current_menu, msize, mpos, choosed, menu1_position, menu_restore);
								draw_menu(choosed);
								temp=delete_one(mainfolder, menu3[menu3_position], "backup");
								if (temp==1)
								{
									if (make_menu_to_array(mainfolder, 3,fw_version, ttype)!=0)
									{
										Mess.Dialog(MSG_ERROR,"Error: There was a problem reading the folder!");
										Graphics->AppExit();
									}
									menu_restore=exists_backups(mainfolder);
									if (menu_restore!=0)
									{
										current_menu=1;
										menu1_position++;
									}
								}
							}
						} */
						
						else if (paddata.BTN_SQUARE)
						{
							if (menu3_position<msize-1)
							{
								Mess.Dialog(MSG_OK, "About:\nThe PS3™ 4K Pro is a project that aims to be an all-in-one solution, covering everything from the basics to the most advanced features. It's available in two variants:\n\n• PS3™ Pro, which is the original version.\n• PS3™ 4K Pro, which was introduced later and quickly became the most popular one.\n\nThey share the same features, differing only in names and logos. It's important to note that, despite the second variant having '4K' in its name, it does not output in that resolution.");
							}
						}

						else if (paddata.BTN_TRIANGLE)
						{
							if (menu3_position<msize-1)
							{
								 {
									Mess.Dialog(MSG_OK, "Credits and acknowledgment:\nThe PS3™ 4K Pro couldn't exist without the help of the PSX-Place community, where I met the some of most amazing people in the scene, whom you may have already heard of, such as:\n\nAldostools, Bguerville, DeviL303, Esc0rtd3w, Pinky, Sandungas, and many others.\n\nAll of them contributed to the project in some way, and their invaluable contributions transformed it from what initially started with just a few visual modifications into what it is today—an all-in-one solution.");
								 }
							}
						}
					
						/*
						else if (paddata.BTN_TRIANGLE)
						{
							if (menu3_position<msize-1) //Delete all backups
							{
								if (delete_all(mainfolder)==1)
								{
									make_menu_to_array(mainfolder, 3,fw_version, ttype);
									menu_restore=-1;
									current_menu=1;
								}
							}
						}	*/
					}
				}
			}
		}
	}
	goto end;

	end:
	{
		setLed("green_default");
		
		PF.printf("Ending program\r\n");
		PF.printf("- Unintializing graphics\r\n");
		if (choosed==1 && reboot!=1)
		{
			bitmap_menu(fw_version, ttype, current_menu, msize, mpos, 0, menu1_position, menu_restore);
			draw_menu(0);
		}
		BMap.ClearBitmap(&Menu_Layer);
		PF.printf("- Unintializing pad control\r\n");
		ioPadEnd(); //this will uninitialize the controllers
		if (is_dev_blind_mounted()==0)
		{
			PF.printf("- Unmounting dev_blind\r\n");
			unmount_dev_blind();
		}
		if (reboot==1)
		{
			//Mess.Dialog(MSG_YESNO_DYES, "How do you want to reboot the system?\n\n- Select 'Yes' to reboot the system completely.\n- Select 'No to reboot the system quickly.");
			//if (Mess.GetResponse(MSG_DIALOG_BTN_NO)==1) rtype="soft";
			PF.printf("- Deleting turnoff file\r\n");
			sysFsUnlink((char*)"/dev_hdd0/tmp/turnoff");
			PF.printf("- Rebooting system\r\n");
			Graphics->NoRSX_Exit(); //This will uninit the NoRSX lib
			reboot_sys(rtype); //reboot
		}
		else Graphics->NoRSX_Exit(); //This will uninit the NoRSX lib
	}
	return 0;
}
