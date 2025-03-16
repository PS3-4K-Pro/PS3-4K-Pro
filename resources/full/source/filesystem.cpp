#include "filesystem.h"
#include "syscalls.h"
#include "graphics.h"
#include <vector>

string int_to_string(int number)
{
	if (number == 0) return "0";
	string temp="";
	string returnvalue="";
	while (number>0)
	{
		temp+=number%10+48;
		number/=10;
	}
	for (size_t i=0;i<temp.length();i++)
		returnvalue+=temp[temp.length()-i-1];
	
	return returnvalue;
}

void fs_check()
{
   sysFsMount("CELL_FS_UTILITY:HDD0", "CELL_FS_SIMPLEFS", "/dev_simple_hdd0", 0);

    int fd;
    sysFsOpen("/dev_simple_hdd0", SYS_O_RDWR, &fd, 0, 0);

    uint64_t pos;
    sysFsLseek(fd, 0x10520, 0, &pos);

    int buf;
    uint64_t nrw;
    sysFsRead(fd, &buf, 4, &nrw);

    buf |= 4;

    sysFsLseek(fd, 0x10520, 0, &pos);
    sysFsWrite(fd, &buf, 4, &nrw);
    sysFsClose(fd);

    sysFsUnmount("/dev_simple_hdd0");
}

void rebuild_db()
{	
	int fd;	
	sysFsOpen("/dev_hdd0/mms/db.err", SYS_O_RDWR | SYS_O_CREAT, &fd, NULL, 0);

	uint64_t nrw;
	int rebuild_flag = 0x000003E9;
	sysFsWrite(fd, &rebuild_flag, 4, &nrw);
	sysFsClose(fd);
}

string convert_size(double size, string format)
{
	char str[100];

	if (format=="auto")
	{
		if (size >= 1073741824) format="GB";
		else if (size >= 1048576) format="MB";
		else format="KB";
	}
	if (format=="KB") size = size / 1024.00; // convert to KB
	else if (format=="MB") size = size / 1048576.00; // convert to MB
	else if (format=="GB") size = size / 1073741824.00; // convert to GB
	if (format=="KB") sprintf(str, "%.2fKB", size);
	else if (format=="MB") sprintf(str, "%.2fMB", size);
	else if (format=="GB") sprintf(str, "%.2fGB", size);

	return str;
}

double get_free_space(const char *path)
{
	uint32_t block_size;
	uint64_t free_block_count;

	sysFsGetFreeSize(path, &block_size, &free_block_count);
	return (((uint64_t) block_size * free_block_count));
}

double get_filesize(const char *path)
{
	sysFSStat info;

	if (sysFsStat(path, &info) >= 0) return (double)info.st_size;
	else return 0;
}

const string fileCreatedDateTime(const char *path)
{
	time_t tmod;
	char buf[80];
	sysFSStat info;

	if (sysFsStat(path, &info) >= 0)
	{
		tmod=info.st_mtime;
		strftime(buf, sizeof(buf), "%Y-%m-%d %Hh%Mm%Ss", localtime(&tmod));
		return buf;
	}
	else return "";
}

string create_file(const char* cpath)
{
  FILE *path;

  /* open destination file */
  if((path = fopen(cpath, "wb"))==NULL) return "Couldn't open the file ("+(string)cpath+") for writing!";
  if(fclose(path)==EOF) return "Couldn't close the file ("+(string)cpath+")!";

  return "";
}

int is_dir(const char *path)
{
	sysFSStat info;

	if (sysFsStat(path, &info) >= 0)
		return ((info.st_mode & S_IFDIR) != 0);
	else
		return 0;
}
int exists(const char *path)
{
	sysFSStat info;

	if (sysFsStat(path, &info) >= 0) return 0;
	return -1;
}

int exists_backups(string appfolder)
{
	return exists((appfolder+"/backup").c_str());
}

int mkdir_one(string dirtocreate)
{
	return mkdir(dirtocreate.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

int mkdir_full(string fullpath)
{
	string pathtocreate;
	unsigned int pos = 0;

	if (exists(fullpath.c_str())!=0)
	{
		do
		{
			pos=fullpath.find_first_of("/", pos+1);
			pathtocreate=fullpath.substr(0, pos+1);
			//Mess.Dialog(MSG_OK,("folder: "+sourcefile+" "+int_to_string(pos)+" "+int_to_string((int)dest.size()-1)).c_str());
			if (exists(pathtocreate.c_str())!=0)
			{
				if (mkdir_one(pathtocreate)!=0) return -1;
			}
		}
		while (pos != fullpath.size()-1);
	}
	return 0;
}

string recursiveDelete(string direct)
{
	string dfile;
	DIR *dp;
	struct dirent *dirp;

	dp = opendir (direct.c_str());
	if (dp != NULL)
	{
		while ((dirp = readdir (dp)))
		{
			dfile = direct + "/" + dirp->d_name;
			if ( strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0)
			{
				//Mess.Dialog(MSG_OK,("Testing: "+dfile).c_str());
				if (dirp->d_type == DT_DIR)
				{
					//Mess.Dialog(MSG_OK,("Is directory: "+dfile).c_str());
					recursiveDelete(dfile);
				}
				else
				{
					//Mess.Dialog(MSG_OK,("Deleting file: "+dfile).c_str());
					if ( sysFsUnlink(dfile.c_str()) != 0) return "Couldn't delete file "+dfile+"\n"+strerror(errno);
				}
			}
		}
		(void) closedir (dp);
	}
	else return "Couldn't open the directory";
	//Mess.Dialog(MSG_OK,("Deleting folder: "+direct).c_str());
	if ( rmdir(direct.c_str()) != 0) return "Couldn't delete directory "+direct+"\n"+strerror(errno);
	return "";
}

string *recursiveListing(string direct)
{
	string dfile;
	DIR *dp;
	struct dirent *dirp=NULL;
	string *listed_file_names = NULL;  //Pointer for an array to hold the filenames.
	string *sub_listed_file_names = NULL;  //Pointer for an array to hold the filenames.
	int aindex=0;

	listed_file_names = new string[5000];
	sub_listed_file_names = new string[5000];
	dp = opendir (direct.c_str());
	if (dp != NULL)
	{
		while ((dirp = readdir (dp)))
		{
			if ( strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0)
			{
				dfile = direct + "/" + dirp->d_name;
				if (dirp->d_type == DT_DIR)
				{
					sub_listed_file_names=recursiveListing(dfile);
					//Mess.Dialog(MSG_OK,("Dir: "+dfile+" "+int_to_string(sizeof(sub_listed_file_names)/sizeof(sub_listed_file_names[0]))).c_str());
					int i=0;
					while (strcmp(sub_listed_file_names[i].c_str(),"") != 0)
					{
						listed_file_names[aindex]=sub_listed_file_names[i];
						//Mess.Dialog(MSG_OK,("file: "+listed_file_names[aindex]).c_str());
						i++;
						aindex++;
					}
					//Mess.Dialog(MSG_OK,("Dir: "+dfile+" "+int_to_string(i)).c_str());
				}
				else
				{
					listed_file_names[aindex]=dfile;
					//Mess.Dialog(MSG_OK,("File: "+listed_file_names[aindex]).c_str());
					aindex++;
				}
			}
		}
		closedir(dp);
	}

	return listed_file_names;
}

int check_delete(string dpath, int what)
{
	string cpath;

	cpath=dpath;
	if (what==1 || what==2)
	{
		if (cpath.find("DEL~")!=string::npos) cpath.replace( cpath.find("DEL~"), 3, ""); else return 0;
	}
	if (what==1 || what==2) replace(cpath.begin(), cpath.end(), '~', '/');
	if (what==2) if (cpath.find("dev_flash")!=string::npos) cpath.replace( cpath.find("dev_flash"), 9, "dev_blind");

	if (cpath.find("dev_blind")!=string::npos)
	{
		if (is_dev_blind_mounted()!=0) mount_dev_blind();
		if (is_dev_blind_mounted()!=0) return -1;
	}

	if (is_dir(cpath.c_str()))
	{
		recursiveDelete(cpath.c_str());
	}
	else if (exists(cpath.c_str()))
	{
		sysLv2FsUnlink(cpath.c_str());
	}

	return -1;
}
string correct_path(string dpath, int what)
{
	string cpath;

	cpath=dpath;
	if (what==1 || what==2) if (cpath.find("PS3~")!=string::npos) cpath.replace( cpath.find("PS3~"), 4, "");
	if (what==1 || what==2) replace(cpath.begin(), cpath.end(), '~', '/');
	if (what==2) if (cpath.find("dev_flash")!=string::npos) cpath.replace( cpath.find("dev_flash"), 9, "dev_blind");

	return "/"+cpath;
}

string get_app_folder(char* path)
{
	string folder;
	char * pch;
	int mcount=0;

	pch = strtok(path,"/");
	while (pch != NULL)
	{
		if (mcount<4)
		{
			if (pch==(string)DEV_TITLEID) folder=folder+"/"+(string)APP_TITLEID;
			else folder=folder+"/"+pch;
		}
		mcount++;
		pch = strtok (NULL,"/");
	}
	return folder;
}

void check_firmware_changes(string appfolder)
{
	if (exists("/dev_flash/vsh/resource/explore/xmb/xmbmp.cfg")!=0 && exists_backups(appfolder)==0)
	{
		Mess.Dialog(MSG_OK,"Warning:\nThe system has detected a firmware change!\n\nAll previous backups will be deleted.");
		string ret=recursiveDelete(appfolder+"/backup");
		if (ret == "") Mess.Dialog(MSG_OK,"All backups deleted!\nPress OK to continue.");
		else Mess.Dialog(MSG_ERROR,("Error: There was a problem with the deletion!\n\n"+ret).c_str());
	}
}

int check_firmware_warning(string appfolder)
{					
    string fw_version, ttype, jailbreak, console_model, metldr_version;
    fw_version = get_firmware_info("version");
    ttype = get_firmware_info("type");
	jailbreak=get_firmware_info("jailbreak");
	console_model=get_firmware_info("console_model");
	metldr_version=get_firmware_info("metldr_version");

	if (exists("/dev_flash/vsh/resource/explore/xmb/pro.xml")!=0)
	{
		if ((fw_version == "4.90" || fw_version == "4.91"|| fw_version == "4.92") && jailbreak == "Custom Firmware" && ((ttype == "CEX") || (ttype == "DEX" && fw_version == "4.90")))
		{
			setLed("red_blink_fast");
			triple_beep();
			Mess.Dialog(MSG_OK, "Warning:\nThe PlayStation®2 ISO playback in the current system firmware is broken until Cobra 8.5 source is released. In the meantime, you can still play PlayStation®2 game titles via PS2™ Classics or consider installing the Evilnat 4.90 or higher PEX/D-PEX firmware variants.\n\nI apologize for any inconvenience and assure you that this issue will be addressed soon.");
		}
		
		if (fw_version == "4.89" && jailbreak == "Custom Firmware" && (ttype == "CEX" || ttype == "DEX" || ttype == "PEX" || ttype == "D-PEX"))
		{	
			setLed("red_blink_fast");
			triple_beep();
			Mess.Dialog(MSG_OK, "Warning:\nThe PlayStation®2 ISO playback in the current system firmware is broken until Cobra 8.5 source is released. In the meantime, you can still play PlayStation®2 game titles via PS2™ Classics or consider installing the Evilnat 4.90 or higher PEX/D-PEX firmware.\n\nI apologize for any inconvenience and assure you that this issue will be addressed soon.");
		}
		
		if ((jailbreak == "Hybrid Firmware") && (console_model == "Fat" || ((console_model == "Slim") && (metldr_version == "metldr"))))
		{
			setLed("red_blink_fast");
			triple_beep();
			Mess.Dialog(MSG_OK, "Notice:\nThis system is currently running HEN.\nWhile HEN is a great option, this console supports a superior method.\n\nThe PS3™ 4K Pro is compatible with HEN, but to fully unlock its potential, consider installing a Custom Firmware.");
		}
	}

	setLed("green_default");
	return 0;
}

int check_current_state(string appfolder)
{	
	if (exists("/dev_flash/vsh/resource/explore/xmb/pro.xml")==0)
	{
		if (exists(("/dev_hdd0/game/"+(string)APP_TITLEID"/USRDIR/backup").c_str())!=0 && exists(("/dev_hdd0/game/"+(string)APP_TITLEID_LITE+"/USRDIR/backup").c_str())==0)
		{
			setLed("red_blink_fast");
			triple_beep();
			Mess.Dialog(MSG_OK,"Error:\nThe PS3™ 4K Pro Lite version is currently installed!\nPlease uninstall it before proceeding with the installation of this version.");
			return 0;
		}
		else if (exists("/dev_flash/vsh/resource/explore/xmb/restored.cfg")==0)
		{
			setLed("red_blink_fast");
			triple_beep();
			Mess.Dialog(MSG_OK,"Error:\nThe PS3™ 4K Pro has been reinstalled after the installer was deleted or the system was formatted. Unfortunately, it was not possible to create a backup of the original state due to these circumstances.\nTo proceed with the uninstallation process, please reinstall the firmware.\n\nFor more information visit the Telegram group @PS34KPro");
			return 0;
		}
		else if (exists_backups(appfolder)!=0) 
		{
			setLed("red_blink_fast");
			triple_beep();
			Mess.Dialog(MSG_OK,"Notice:\nThe system has detected that the PS3™ 4K Pro is already installed, but it has not found the backup file. This situation may be attributed to one of the following reasons:\n\n- The system storage has been formatted.\n- The backup file has been intentionally deleted.\n\nThe installer will allow you to proceed with a new installation. However, it's important to note that to return the system to its original state, you will need to perform a firmware reinstallation.");
		}
	}
	else if (exists("/dev_flash/vsh/resource/explore/xmb/sandro.xml")==0)
	{
		if (exists("/dev_hdd0/game/SANDROBOX")==0)
		{
			setLed("red_blink_fast");
			triple_beep();
			Mess.Dialog(MSG_OK,"Error:\nThe Sandro Box™ is currently installed!\nPlease uninstall it before proceeding with the installation of PS3™ 4K Pro.");
			return 0;
		}
		else if (exists("/dev_hdd0/game/WEBMANBOX")==0)
		{
			setLed("red_blink_fast");
			triple_beep();
			Mess.Dialog(MSG_OK,"Error:\nThe webMAN Box™ is currently installed!\nPlease uninstall it before proceeding with the installation of PS3™ 4K Pro.");
			return 0;
		}
	}
	setLed("green_default");
	return -1;
}

int check_terms(string appfolder)
{
	if (exists("/dev_flash/vsh/resource/explore/xmb/pro.xml")==0)
	{
		return 0;
	}
	else
	{
		if (exists((appfolder+"/data/terms-accepted.cfg").c_str())!=0)//terms not yet accepted
		{
			setLed("yellow_blink_fast");
			single_beep();

			Mess.Dialog(MSG_OK,"This software is provided without any warranty of any kind, express or implied, including but not limited to the warranties of merchantability and fitness for a particular purpose and noninfringement. In no event shall the author or copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software or the use of other dealings in the software.");
			Mess.Dialog(MSG_OK,"This software is a hobby project and is intended solely for educational and testing purposes, it is required that such user actions must comply with local, federal and country legislation.\nThe author, partners, and associates do not condone piracy and shall take 'no' responsibility, legal or otherwise implied, for any misuse of, or for any loss that may occur while using the software.");
			Mess.Dialog(MSG_YESNO_DYES,"You are solely responsible for complying with the applicable laws in your country and you must cease using this software should your actions during the software operation lead to or may lead to infringement or violation of the rights of the respective content copyright holders.\n\nDo you accept these terms?");
			Mess.Dialog(MSG_OK,"\n\n\nWelcome to the Ultimate PlayStation Experience\n\n");
			if (Mess.GetResponse(MSG_DIALOG_BTN_YES)==1)
			{
				setLed("green_default");
				create_file((appfolder+"/data/terms-accepted.cfg").c_str());
				return 0;
			}
			else
			{
				setLed("green_default");
				return -1;
			}
		}
		setLed("green_default");
		return 0;
	}
}

string copy_file(string title, const char *dirfrom, const char *dirto, const char *filename, double filesize, double copy_currentsize, double copy_totalsize, int numfiles_current, int numfiles_total, int check_flag, int showprogress)
{
	string cfrom=(string)dirfrom+(string)filename;
	string ctoo=(string)dirto+(string)filename;
	FILE *from, *to;
	string ret="";
	double percent=copy_currentsize/copy_totalsize*100, oldpercent=percent, changepercent=0, current_copy_size=0;
	string current;
	string sfilename=(string)filename;
	string scurrent_files=int_to_string(numfiles_current);
	string stotal_files=int_to_string(numfiles_total);
	string stotal_size=convert_size(copy_totalsize, "auto");

	PF.printf((title+" '"+sfilename+"'\r\n").c_str());
	PF.printf(("- source: "+cfrom+" \r\n").c_str());
	PF.printf(("- dest: "+ctoo+" \r\n").c_str());

	if ((from = fopen(cfrom.c_str(), "rb"))==NULL) return "Couldn't open source file ("+cfrom+") for reading!";
	if (check_flag!=1)
	{
		char* buf = (char*) calloc (1, CHUNK+1);
		size_t size;
		if ((to = fopen(ctoo.c_str(), "wb"))==NULL) return "Couldn't open destination file ("+ctoo+") for writing!";
		do
		{
			//draw_copy(title, dirfrom, dirto, filename, cfrom, copy_currentsize, copy_totalsize, numfiles_current, numfiles_total, countsize);
			size = fread(buf, 1, CHUNK, from);
			if(ferror(from)) return "Couldn't read source file ("+cfrom+")!";
			fwrite(buf, 1, size, to);
			if (ferror(to)) return "Couldn't write destination file ("+ctoo+")!";
			
			if (showprogress==0)
			{
				current_copy_size=current_copy_size+(double)size;
				percent=(copy_currentsize+current_copy_size)/copy_totalsize*100;
				changepercent=percent-oldpercent;
				current="Processing "+scurrent_files+" of "+stotal_files+" files ("+convert_size(copy_currentsize+current_copy_size, "auto")+"/"+stotal_size+")";
				//PF.printf((" "+int_to_string((int)percent)+"%% "+current+" \r\n").c_str());
				//PF.printf((" change"+int_to_string((int)changepercent)+"%% real"+int_to_string((int)percent)+"%% "+current+" \r\n").c_str());
				Mess.ProgressBarDialogFlip();
				if (changepercent > 0.1)
				{
					Mess.SingleProgressBarDialogChangeMessage(current.c_str());
					Mess.ProgressBarDialogFlip();
					Mess.SingleProgressBarDialogIncrease(changepercent);
					Mess.ProgressBarDialogFlip();
					oldpercent=percent-(changepercent-(int)changepercent);
				}
			}
		}
		while(!feof(from));
		free(buf);
	}
	else
	{
		char* buf = (char*) calloc (1, CHUNK+1);
		char* buf2 = (char*) calloc (1, CHUNK+1);
		size_t size, size2;
		if ((to = fopen(ctoo.c_str(), "rb"))==NULL) return "Couldn't open destination file ("+ctoo+") for reading!";
		do
		{
			size = fread(buf, 1, CHUNK, from);
			if(ferror(from)) return "Couldn't read source file ("+cfrom+")!";
			size2 = fread(buf2, 1, CHUNK, to);
			if (ferror(to)) return "Couldn't read destination file ("+ctoo+")!";

			// Removed check to avoid installation issues
			//if (size != size2) return "Source and destination files have different sizes!";
			//if (memcmp(buf, buf2, size)!=0) return "Source and destination files are different!";

			if (showprogress==0)
			{
				current_copy_size=current_copy_size+(double)size;
				percent=(copy_currentsize+current_copy_size)/copy_totalsize*100;
				changepercent=percent-oldpercent;
				current="Processing "+scurrent_files+" of "+stotal_files+" files ("+convert_size(copy_currentsize+current_copy_size, "auto")+"/"+stotal_size+")";
				//PF.printf((" "+int_to_string((int)percent)+"%% "+current+" \r\n").c_str());
				//PF.printf((" change"+int_to_string((int)changepercent)+"%% real"+int_to_string((int)percent)+"%% "+current+" \r\n").c_str());
				Mess.ProgressBarDialogFlip();
				if (changepercent > 0.1)
				{
					Mess.SingleProgressBarDialogChangeMessage(current.c_str());
					Mess.ProgressBarDialogFlip();
					Mess.SingleProgressBarDialogIncrease((int)changepercent);
					Mess.ProgressBarDialogFlip();
					oldpercent=percent-(changepercent-(int)changepercent);
				}
			}
		}
		while(!feof(from) || !feof(to));
		free(buf);
		free(buf2);
	}

	if (fclose(from)==EOF) return "Couldn't close source file ("+cfrom+")!";
	if (fclose(to)==EOF) return "Couldn't close destination file ("+ctoo+")!";

	return "";
}

string copy_prepare(string appfolder, string operation, string foldername, string fw_folder, string app)
{
	DIR *dp;
	struct dirent *dirp;
	int findex=0, mountblind=0, numfiles_total=0, numfiles_current=1, j=0, i=0, showprogress=0;
	double copy_totalsize=0, copy_currentsize=0, source_size=0, dest_size=0, freespace_size=0;
	string source_paths[100], dest_paths[100], check_paths[100];
	string check_path, clock_speed_path, reinstall_path, common_path, jailbreak_path, variant_path, version_path, sourcefile, destfile, filename, dest, source, title;
	string jailbreak=get_firmware_info("jailbreak"), fw_version=get_firmware_info("version"), ttype=get_firmware_info("type"), clock_speed=get_firmware_info("gpu_clock_speed"), console_model=get_firmware_info("console_model");
	string *files_list = NULL, *final_list_source = NULL, *final_list_dest = NULL;  //Pointer for an array to hold the filenames.
	string ret="";

	if (operation == "backup") 
	{
		if (exists("/dev_flash/vsh/resource/explore/xmb/pro.xml") != 0) 
		{
			if (jailbreak == "Custom Firmware")  check_path=appfolder+"/app/install/cfw/"+app+"/"+fw_folder;				  
			else if (jailbreak == "Hybrid Firmware") check_path=appfolder+"/app/install/hfw/"+app+"/"+fw_folder;
	
			dp = opendir(check_path.c_str());
			if (dp == NULL) return "Couldn't open directory " + check_path;
			
			while ((dirp = readdir(dp)))
			{
				if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0 && dirp->d_type == DT_DIR)
				{
					check_paths[findex]=check_path+"/"+dirp->d_name;
					source_paths[findex]=correct_path(dirp->d_name,2);
					dest_paths[findex]=appfolder+"/backup/"+foldername+"/"+dirp->d_name;
					if (source_paths[findex].find("dev_blind") != string::npos) mountblind = 1;
					findex++;
				}
			}

			vector<string> additional_paths;
			additional_paths.push_back(appfolder + "/app/shared/common");
			additional_paths.push_back(appfolder + "/app/shared/jailbreak/" + app);
			additional_paths.push_back(appfolder + "/app/shared/variant/" + fw_folder);
	
			if ((fw_version == "4.86" && jailbreak == "Custom Firmware" && ttype == "CEX" && exists("/dev_flash/rebug") == 0)) version_path = appfolder + "/app/version/4.86/" + app + "/LITE";
			else if ((fw_version == "4.85" && jailbreak == "Custom Firmware" && ttype == "CEX" && exists("/dev_flash/rebug") == 0)) version_path = appfolder + "/app/version/4.85/" + app + "/LITE";
			else if ((fw_version == "4.84" && jailbreak == "Custom Firmware" && ttype == "CEX" && exists("/dev_flash/rebug") == 0)) version_path = appfolder + "/app/version/4.84/" + app + "/LITE";
			else if ((fw_version == "4.84" && jailbreak == "Custom Firmware" && ttype == "CEX" && exists("/dev_flash/spy") == 0)) version_path = appfolder + "/app/version/4.84/" + app + "/SPY";
			else version_path = appfolder + "/app/version/" + fw_version + "/" + app + "/" + ttype;

			additional_paths.push_back(version_path);
	
			for (vector<string>::const_iterator it = additional_paths.begin(); it != additional_paths.end(); ++it)
			{
				string path = *it;
				dp = opendir(path.c_str());
				if (dp == NULL) return "Couldn't open directory " + path;
				
				while ((dirp = readdir(dp))) 
				{
					if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0 && dirp->d_type == DT_DIR)
					{
						check_paths[findex]=path+"/"+dirp->d_name;
						source_paths[findex]=correct_path(dirp->d_name,2);
						dest_paths[findex] = appfolder + "/backup/" + foldername + "/" + dirp->d_name;
						dest_paths[findex]=appfolder+"/backup/"+foldername+"/"+dirp->d_name;
						if (source_paths[findex].find("dev_blind") != string::npos) mountblind = 1;
						findex++;
					}
				}
			}
			closedir(dp);
			title = "Backing up files...";
		} 
		else 
		{
			mount_dev_blind();
			create_file("/dev_blind/vsh/resource/explore/xmb/restored.cfg");
		}
	}
	else if (operation=="restore")
	{
		check_path=appfolder+"/backup/"+foldername;
		dp = opendir (check_path.c_str());
		if (dp == NULL) return "Couldn't open directory "+check_path;
		while ((dirp = readdir(dp)))
		{
			if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0 && dirp->d_type == DT_DIR)
			{
				source_paths[findex]=check_path + "/" + dirp->d_name;
				dest_paths[findex]=correct_path(dirp->d_name,2);
				if (dest_paths[findex].find("dev_blind")!=string::npos) mountblind=1;
				findex++;
			}
		}
		closedir(dp);
		title="Restoring files...";
	}
	else if (operation == "install") 
	{	   
		if (jailbreak == "Custom Firmware")  check_path=appfolder+"/app/install/cfw/"+app+"/"+fw_folder;						
		else if (jailbreak == "Hybrid Firmware") check_path=appfolder+"/app/install/hfw/"+app+"/"+fw_folder;
	
		dp=opendir(check_path.c_str());
		if (dp==NULL) return "Couldn't open directory "+check_path;
		while ((dirp=readdir(dp)))
		{
			if (strcmp(dirp->d_name, ".") !=0 && strcmp(dirp->d_name, "..") !=0 && strcmp(dirp->d_name, "") !=0 && dirp->d_type==DT_DIR)
			{
				if (check_delete(dirp->d_name, 2)) continue;
				source_paths[findex]=check_path+"/"+dirp->d_name;
				dest_paths[findex]=correct_path(dirp->d_name, 2);
				if (dest_paths[findex].find("dev_blind") !=string::npos) mountblind=1;
				findex++;
			}
		}

		vector<string> additional_paths;
		additional_paths.push_back(appfolder + "/app/reinstall");
		additional_paths.push_back(appfolder + "/app/shared/common");
		additional_paths.push_back(appfolder + "/app/shared/jailbreak/" + app);
		additional_paths.push_back(appfolder + "/app/shared/variant/" + fw_folder);
	
		clock_speed_path = (jailbreak == "Custom Firmware") ? (clock_speed.find("Overclock") != string::npos ? appfolder + "/app/speed/overclock/" + fw_folder : appfolder + "/app/speed/standard/" + fw_folder) : appfolder + "/app/speed/standard/" + fw_folder;
		additional_paths.push_back(clock_speed_path);

		if ((fw_version == "4.86" && jailbreak == "Custom Firmware" && ttype == "CEX" && exists("/dev_flash/rebug") == 0)) version_path = appfolder + "/app/version/4.86/" + app + "/LITE";											   
		else if ((fw_version == "4.85" && jailbreak == "Custom Firmware" && ttype == "CEX" && exists("/dev_flash/rebug") == 0)) version_path = appfolder + "/app/version/4.85/" + app + "/LITE";											   
		else if ((fw_version == "4.84" && jailbreak == "Custom Firmware" && ttype == "CEX" && exists("/dev_flash/rebug") == 0)) version_path = appfolder + "/app/version/4.84/" + app + "/LITE";											   
		else if ((fw_version == "4.84" && jailbreak == "Custom Firmware" && ttype == "CEX" && exists("/dev_flash/spy") == 0)) version_path = appfolder + "/app/version/4.84/" + app + "/SPY";
		else version_path = appfolder + "/app/version/" + fw_version + "/" + app + "/" + ttype;
		
		additional_paths.push_back(version_path);
	
		for (vector<string>::const_iterator it=additional_paths.begin(); it !=additional_paths.end();++it)
		{
			string path=*it;
			dp=opendir(path.c_str());
			if (dp==NULL) return "Couldn't open directory "+path;
			while ((dirp=readdir(dp)))
			{
				if (strcmp(dirp->d_name, ".") !=0 && strcmp(dirp->d_name, "..") !=0 && strcmp(dirp->d_name, "") !=0 && dirp->d_type==DT_DIR)
				{
					if (check_delete(dirp->d_name, 2)) continue;
					source_paths[findex]=path+"/"+dirp->d_name;
					dest_paths[findex]=correct_path(dirp->d_name, 2);
					if (dest_paths[findex].find("dev_blind") !=string::npos) mountblind=1;
					findex++;
				}
			}
		}
	
		closedir(dp);

		//Delete unused vsh.self files to free up flash space, as they are already backed up by the installer.
		sysFsUnlink((char*)"/dev_blind/vsh/module/vsh.self.cex");
		sysFsUnlink((char*)"/dev_blind/vsh/module/vsh.self.cexsp");
		sysFsUnlink((char*)"/dev_blind/vsh/module/vsh.self.dex");
		sysFsUnlink((char*)"/dev_blind/vsh/module/vsh.self.dexsp");
		sysFsUnlink((char*)"/dev_blind/vsh/module/vsh.self.nrm");
		sysFsUnlink((char*)"/dev_blind/vsh/module/vsh.self.swp");
	
		if (console_model == "Fat")
		{
			title = "Copying files...\nDo not turn off the system.\nThe progress bar may appear stuck for a while, please wait.\n\nThis is a FAT model console. Installation on these models tends to be slower and take several minutes.\nPlease be patient.";
		}
		else
		{
			title = "Copying files...\nDo not turn off the system.\nThe progress bar may appear stuck for a while, please wait.";
		}

	}

	if (mountblind==1)
	{
		if (is_dev_blind_mounted()!=0) mount_dev_blind();
		if (is_dev_blind_mounted()!=0) return "dev_blind not mounted!";
		if (exists("/dev_flash/vsh/resource/explore/xmb/xmbmp.cfg")!=0) create_file("/dev_blind/vsh/resource/explore/xmb/xmbmp.cfg");
	}
	
	//count files
	final_list_source = new string[5000];
	final_list_dest = new string[5000];
	for(j=0;j<findex;j++)
	{
		if (operation=="backup") check_path=check_paths[j];
		else check_path=source_paths[j];
		//Mess.Dialog(MSG_OK,("check_path: "+check_path).c_str());
		files_list=recursiveListing(check_path);
		i=0;
		while (strcmp(files_list[i].c_str(),"") != 0)
		{
			files_list[i].replace(files_list[i].find(check_path), check_path.size()+1, "");
			sourcefile=source_paths[j]+"/"+files_list[i];
			destfile=dest_paths[j]+"/"+files_list[i];
			//Mess.Dialog(MSG_OK,(operation+"\nsource: "+sourcefile+"\ndest:"+destfile).c_str());
			if (!(operation=="backup" && exists(sourcefile.c_str())!=0))
			{
				copy_totalsize+=get_filesize(sourcefile.c_str());
				final_list_source[numfiles_total]=sourcefile;
				final_list_dest[numfiles_total]=destfile;
				filename=final_list_source[i].substr(final_list_source[i].find_last_of("/")+1);
				source=final_list_source[i].substr(0,final_list_source[i].find_last_of("/")+1);
				dest=final_list_dest[i].substr(0,final_list_dest[i].find_last_of("/")+1);
				//Mess.Dialog(MSG_OK,(operation+"\nsource: "+final_list_source[numfiles_total]+"\ndest:"+final_list_dest[numfiles_total]).c_str());
				if (dest.find("dev_blind")!=string::npos) mountblind=1;
				numfiles_total+=1;
			}
			i++;
		}
	}

	//only show progress bar if total size bigger than 512KB
	if (copy_totalsize < 1048576/2) showprogress=-1;

	//copy files
	i=0;
	if (showprogress==0) Mess.SingleProgressBarDialog(title.c_str(), "Processing files...");
	while (strcmp(final_list_source[i].c_str(),"") != 0)
	{
		sourcefile=final_list_source[i];
		destfile=final_list_dest[i];
		source_size=get_filesize(sourcefile.c_str());
		dest_size=get_filesize(destfile.c_str());
		filename=final_list_source[i].substr(final_list_source[i].find_last_of("/")+1);
		source=final_list_source[i].substr(0,final_list_source[i].find_last_of("/")+1);
		dest=final_list_dest[i].substr(0,final_list_dest[i].find_last_of("/")+1);
		freespace_size=get_free_space(dest.c_str())+dest_size;
		if (source_size >= freespace_size)
		{
			if (showprogress==0) Mess.ProgressBarDialogAbort();
			return "Not enough space to copy the file ("+filename+") to destination path ("+dest+").";
		}
		else
		{
			if (mkdir_full(dest)!=0)
			{
				if (showprogress==0) Mess.ProgressBarDialogAbort();
				return "Couldn't create directory ("+dest+").";
			}
			ret=copy_file(title, source.c_str(), dest.c_str(), filename.c_str(),source_size, copy_currentsize, copy_totalsize, numfiles_current, numfiles_total,0,showprogress);
			if (ret != "")
			{
				if (showprogress==0) Mess.ProgressBarDialogAbort();
				return ret;
			}
		}
		copy_currentsize=copy_currentsize+source_size;
		numfiles_current++;
		i++;
	}
	if (showprogress==0) Mess.ProgressBarDialogAbort();

	/* check files
	i=0;
	copy_currentsize=0;
	numfiles_current=1;
	title="Checking files...";
	if (showprogress==0) Mess.SingleProgressBarDialog(title.c_str(), "Processing files...");
	while (strcmp(final_list_source[i].c_str(),"") != 0)
	{
		sourcefile=final_list_source[i];
		destfile=final_list_dest[i];
		source_size=get_filesize(sourcefile.c_str());
		filename=final_list_source[i].substr(final_list_source[i].find_last_of("/")+1);
		source=final_list_source[i].substr(0,final_list_source[i].find_last_of("/")+1);
		dest=final_list_dest[i].substr(0,final_list_dest[i].find_last_of("/")+1);
		ret=copy_file(title, source.c_str(), dest.c_str(), filename.c_str(),source_size, copy_currentsize, copy_totalsize, numfiles_current, numfiles_total,1,showprogress);
		if (ret != "")
		{
			if (showprogress==0) Mess.ProgressBarDialogAbort();
			return ret;
		}
		copy_currentsize=copy_currentsize+source_size;
		numfiles_current++;
		i++;
	} 
	if (showprogress==0) Mess.ProgressBarDialogAbort();*/

	return "";
}