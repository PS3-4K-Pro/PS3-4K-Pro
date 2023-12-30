#ifndef __XMBMP_FILE_H__
#define __XMBMP_FILE_H__
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <dirent.h>
#include <string>
#include <algorithm>

#define CHUNK 65536
#define APP_TITLEID "PS34KPROX"
#define DEV_TITLEID "PS34KPROX"

#define APP_TITLEID_LITE "PS34KPROL"
#define APP_TITLEID_FULL "PS34KPROX"

using namespace std;

string int_to_string(int number);
string convert_size(double size, string format);
double get_free_space(const char *path);
double get_filesize(const char *path);
const string fileCreatedDateTime(const char *path);
string create_file(const char* cpath);
int exists(const char *path);
int exists_backups(string appfolder);
int mkdir_one(string fullpath);
int mkdir_full(string fullpath);
string recursiveDelete(string direct);
string *recursiveListing(string direct);
string correct_path(string dpath, int what);
string get_app_folder(char* path);
int check_current_state(string appfolder);
void check_current_folder(string appfolder);
void check_jailbreak_type(string appfolder);
void check_firmware_changes(string appfolder);
int check_terms(string appfolder);
int check_firmware_warning(string appfolder);
string copy_file(string title, const char *dirfrom, const char *dirto, const char *filename, double filesize, double copy_currentsize, double copy_totalsize, int numfiles_current, int numfiles_total, int check_flag, int showprogress);
string copy_prepare(string appfolder, string operation, string foldername, string fw_folder, string app);

#endif