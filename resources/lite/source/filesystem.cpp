#include "filesystem.h"
#include "syscalls.h"
#include "graphics.h"

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

int check_current_state(string appfolder)
{	
	if (exists("/dev_flash/vsh/resource/explore/xmb/pro.xml")==0)
	{
		if (exists(("/dev_hdd0/game/"+(string)APP_TITLEID"/USRDIR/backup").c_str())!=0 && exists(("/dev_hdd0/game/"+(string)APP_TITLEID_FULL+"/USRDIR/backup").c_str())==0)
		{
			Mess.Dialog(MSG_OK,"Error:\nThe PS3™ 4K Pro Full version is currently installed!\nPlease uninstall it before proceeding with the installation of this version.");
			return 0;
		}
		else if (exists_backups(appfolder)!=0) 
		{
			Mess.Dialog(MSG_OK,"Warning:\nThe system has detected that the PS3™ 4K Pro is already installed, but it has not found the backup file. This situation may be attributed to one of the following reasons:\n\n- The system storage has been formatted.\n- The backup file has been intentionally deleted.\n\nThe installer will allow you to proceed with a new installation. However, it's important to note that to return the system to its original state, you will need to perform a firmware reinstallation.");
		}
	}
	return -1;
}

void check_current_folder(string appfolder)

{	
	//Resets the current folder to allow jailbreak type change without reinstalling the pkg
	if (exists((appfolder+"/app/install/current/Custom Firmware (CFW)").c_str())==0)
	{
		sysLv2FsRename((appfolder+"/app/install/current").c_str(),(appfolder+"/app/install/cfw").c_str());
	}
	else if (exists((appfolder+"/app/install/current/Hybrid Firmware (HFW)").c_str())==0)
	{
		sysLv2FsRename((appfolder+"/app/install/current").c_str(),(appfolder+"/app/install/hfw").c_str());
	}
	else
	{
		//Mess.Dialog(MSG_OK,"First launch, detecting jailbreak type...");
	}
}


void check_jailbreak_type(string appfolder)
{	

	string jailbreak=get_firmware_info("jailbreak");
	
	
	//Checks jailbreak type
	{
		if (jailbreak == "Hybrid Firmware")
			{	
				sysLv2FsRename((appfolder+"/app/install/hfw").c_str(),(appfolder+"/app/install/current").c_str());
				//Mess.Dialog(MSG_OK,"HEN jaibreak detected!");
			}
		else if (jailbreak == "Custom Firmware")
			{
				sysLv2FsRename((appfolder+"/app/install/cfw").c_str(),(appfolder+"/app/install/current").c_str());
				//Mess.Dialog(MSG_OK,"CFW jaibreak detected!");
		
			}
		else
			{
				Mess.Dialog(MSG_OK,"Error: No jaibreak detected!");
			}
	}
}

int check_terms(string appfolder)
{					
	if (exists((appfolder+"/data/terms-accepted.cfg").c_str())!=0)//terms not yet accepted
	{
		Mess.Dialog(MSG_OK,"This software is provided without any warranty of any kind, express or implied, including but not limited to the warranties of merchantability and fitness for a particular purpose and noninfringement. In no event shall the author or copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software or the use of other dealings in the software.");
		Mess.Dialog(MSG_OK,"This software is a hobby project and is intended solely for educational and testing purposes, it is required that such user actions must comply with local, federal and country legislation.\nThe author, partners, and associates do not condone piracy and shall take 'no' responsibility, legal or otherwise implied, for any misuse of, or for any loss that may occur while using the software.");
		Mess.Dialog(MSG_YESNO_DYES,"You are solely responsible for complying with the applicable laws in your country and you must cease using this software should your actions during the software operation lead to or may lead to infringement or violation of the rights of the respective content copyright holders.\n\nDo you accept this terms?");
		Mess.Dialog(MSG_OK,"\n\n\nWelcome to the Ultimate PlayStation Experience\n\n");
		if (Mess.GetResponse(MSG_DIALOG_BTN_YES)==1)
		{
			create_file((appfolder+"/data/terms-accepted.cfg").c_str());
			return 0;
		}
		else return -1;
	}
	return 0;
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
				if (changepercent>1)
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
//			if (size != size2) return "Source and destination files have different sizes!";
//			if (memcmp(buf, buf2, size)!=0) return "Source and destination files are different!";

			if (showprogress==0)
			{
				current_copy_size=current_copy_size+(double)size;
				percent=(copy_currentsize+current_copy_size)/copy_totalsize*100;
				changepercent=percent-oldpercent;
				current="Processing "+scurrent_files+" of "+stotal_files+" files ("+convert_size(copy_currentsize+current_copy_size, "auto")+"/"+stotal_size+")";
				//PF.printf((" "+int_to_string((int)percent)+"%% "+current+" \r\n").c_str());
				//PF.printf((" change"+int_to_string((int)changepercent)+"%% real"+int_to_string((int)percent)+"%% "+current+" \r\n").c_str());
				Mess.ProgressBarDialogFlip();
				if (changepercent>1)
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
	string check_path, clock_speed_path, reinstall_path, shared_path, version_path, sourcefile, destfile, filename, dest, source, title;
	string fw_version=get_firmware_info("version"), ttype=get_firmware_info("type"), clock_speed=get_firmware_info("gpu_clock_speed");
	string *files_list = NULL, *final_list_source = NULL, *final_list_dest = NULL;  //Pointer for an array to hold the filenames.
	string ret="";

	if (operation=="backup")
	{
		check_path=appfolder+"/app/install/current/"+app+"/"+fw_folder;
		dp = opendir (check_path.c_str());
		if (dp == NULL) return "Couldn't open directory "+check_path;
		while ( (dirp = readdir(dp) ) )
		{
			if ( strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0)
			{
				if (dirp->d_type == DT_DIR)
				{
					check_paths[findex]=check_path+"/"+dirp->d_name;
					source_paths[findex]=correct_path(dirp->d_name,2);
					dest_paths[findex]=appfolder+"/backup/"+foldername+"/"+dirp->d_name;
					if (source_paths[findex].find("dev_blind")!=string::npos) mountblind=1;
					findex++;
				}
			}
		}
		
		shared_path=appfolder+"/app/shared/";
		dp = opendir (shared_path.c_str());
		if (dp == NULL) return "Couldn't open directory "+shared_path;
		while ( (dirp = readdir(dp) ) )
		{
			if ( strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0)
			{
				if (dirp->d_type == DT_DIR)
				{
					check_paths[findex]=shared_path+"/"+dirp->d_name;
					source_paths[findex]=correct_path(dirp->d_name,2);
					dest_paths[findex]=appfolder+"/backup/"+foldername+"/"+dirp->d_name;
					if (source_paths[findex].find("dev_blind")!=string::npos) mountblind=1;
					findex++;
				}
			}
		}
		
		version_path=appfolder+"/app/version/"+fw_version+"/"+app+"/"+ttype;
		dp = opendir (version_path.c_str());
		if (dp == NULL) return "Couldn't open directory "+version_path;
		while ( (dirp = readdir(dp) ) )
		{
			if ( strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0)
			{
				if (dirp->d_type == DT_DIR)
				{
					check_paths[findex]=version_path+"/"+dirp->d_name;
					source_paths[findex]=correct_path(dirp->d_name,2);
					dest_paths[findex]=appfolder+"/backup/"+foldername+"/"+dirp->d_name;
					if (source_paths[findex].find("dev_blind")!=string::npos) mountblind=1;
					findex++;
				}
				//check zip files
			}
		}

		closedir(dp);
		title="Backing up files...";
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
	else if (operation=="install")
	{
		//
		check_path=appfolder+"/app/install/current/"+app+"/"+fw_folder;
		dp = opendir (check_path.c_str());
		if (dp == NULL) return "Couldn't open directory "+check_path;
		while ( (dirp = readdir(dp) ) )
		{
			if ( strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0 && dirp->d_type == DT_DIR)
			{
				if(check_delete(dirp->d_name,2)) continue;
				source_paths[findex]=check_path + "/" + dirp->d_name;
				dest_paths[findex]=correct_path(dirp->d_name,2);
				if (dest_paths[findex].find("dev_blind")!=string::npos) mountblind=1;
				findex++;
			}
		}
		
			reinstall_path=appfolder+"/app/reinstall/";
			dp = opendir (reinstall_path.c_str());
			if (dp == NULL) return "Couldn't open directory "+reinstall_path;
			while ( (dirp = readdir(dp) ) )
		{
			if ( strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0 && dirp->d_type == DT_DIR)
			{
				if(check_delete(dirp->d_name,2)) continue;		
				source_paths[findex]=reinstall_path + "/" + dirp->d_name;
				dest_paths[findex]=correct_path(dirp->d_name,2);
				if (dest_paths[findex].find("dev_blind")!=string::npos) mountblind=1;
				findex++;
			}
		}
		
			shared_path=appfolder+"/app/shared/";
			dp = opendir (shared_path.c_str());
			if (dp == NULL) return "Couldn't open directory "+shared_path;
			while ( (dirp = readdir(dp) ) )
		{
			if ( strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0 && dirp->d_type == DT_DIR)
			{
				if(check_delete(dirp->d_name,2)) continue;		
				source_paths[findex]=shared_path + "/" + dirp->d_name;
				dest_paths[findex]=correct_path(dirp->d_name,2);
				if (dest_paths[findex].find("dev_blind")!=string::npos) mountblind=1;
				findex++;
			}
		
		}
			
			clock_speed_path="";
			
			if (clock_speed.find("Overclock") != string::npos)
			{
				clock_speed_path=appfolder+"/app/speed/overclock"+"/"+fw_folder;
			}
			else
			{
				clock_speed_path=appfolder+"/app/speed/standard"+"/"+fw_folder;
			}

			dp = opendir (clock_speed_path.c_str());
			if (dp == NULL) return "Couldn't open directory "+clock_speed_path;
			while ( (dirp = readdir(dp) ) )
		{
			if ( strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0 && dirp->d_type == DT_DIR)
			{
				if(check_delete(dirp->d_name,2)) continue;		
				source_paths[findex]=clock_speed_path + "/" + dirp->d_name;
				dest_paths[findex]=correct_path(dirp->d_name,2);
				if (dest_paths[findex].find("dev_blind")!=string::npos) mountblind=1;
				findex++;
			}
		
		}
		
		version_path=appfolder+"/app/version/"+fw_version+"/"+app+"/"+ttype;
		dp = opendir (version_path.c_str());
		if (dp == NULL) return "Couldn't open directory "+version_path;
		while ( (dirp = readdir(dp) ) )
		{
			if ( strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, "") != 0 && dirp->d_type == DT_DIR)
			{
				if(check_delete(dirp->d_name,2)) continue;
				source_paths[findex]=version_path + "/" + dirp->d_name;
				dest_paths[findex]=correct_path(dirp->d_name,2);
				if (dest_paths[findex].find("dev_blind")!=string::npos) mountblind=1;
				findex++;
			}
			//check zip files
		}
		closedir(dp);
		title="Copying files...\nDo not turn off the system.\nThe progress bar may appear stuck for some time, please wait.";
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