#using <System.dll>
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<fstream>
#include<iomanip>
#include<io.h>
#include<time.h>
#include<sstream>
#include<string>
#include <windows.h>

using namespace System;
using namespace System::IO;
using namespace System::Security::Permissions;
using namespace std;
using namespace System::Runtime::InteropServices;


//Function Name
string GetFileNameFromFullPath(string FullPath);
int ScreenFileType(FileSystemEventArgs^ e);
int PositionOfSysList(string FileNameFullPath,int pstart);
DWORD WINAPI BackUpFile(LPVOID lpParam);
string GetTarFileName();
string GetUserName();//NOT
string GetBackUpPath();//NOT
bool JumpWatherOnCreated(FileSystemEventArgs^ e);
bool JumpWatherOnChanged(FileSystemEventArgs^ e);
bool JumpWatherOnDeleted(FileSystemEventArgs^ e);
bool JumpWatherOnRenamed(FileSystemEventArgs^ e);


//class name
class SysFile;



//Function & class 定义

/*
  该类主要是将文件信息整合当到一个地方
*/
class SysFile{

public:
	string FileName;
	string BoxName;//box名称
	string FullPath;//文件在磁盘的路径
	string BoxFullPath;//box中的虚拟文件夹路径,构成  BoxName/虚拟文件夹名/文件名
	int BranchNumber;
	int threadhandle;//储存当前调用该文件的线程号，如果没有则为0

	SysFile(){

		FileName = "";
		BoxName = "";
		FullPath = "";
		BoxFullPath = "";
		threadhandle = 0;
		BranchNumber = 0;
	}

	//默认文件储存在box的根目录下
	SysFile(string FullPath_input,string BoxName_input){

		FullPath = FullPath_input;
		BoxName = BoxName_input;
		FileName = GetFileNameFromFullPath(FullPath);
		BoxFullPath = BoxName+"::"+FileName;
		BranchNumber = 0;
	}

	SysFile(string FullPath_input,string BoxName_input,string BoxFullPath){

		FullPath = FullPath_input;
		FileName = GetFileNameFromFullPath(FullPath);
		BoxName = BoxName_input;
		BoxFullPath = BoxFullPath;
		BranchNumber = 0;
	}

	SysFile operator=(SysFile file_in){
		this->FileName = file_in.FileName;
		this->BoxName = file_in.BoxName;
		this->FullPath = file_in.FullPath;
		this->BoxFullPath = file_in.BoxFullPath;
		this->BranchNumber = file_in.BranchNumber;

		return *this;
	}

	void addSysFile(string FullPath_input,string BoxName_input){
		FullPath = FullPath_input;
		BoxName = BoxName_input;
		FileName = GetFileNameFromFullPath(FullPath);
		BoxFullPath = BoxName+"/"+FileName;
	}
};
	
/*
  该结构主要是传入backupfile函数使用
  //tap 为文件更改类型 created 1 changed 2 deleted 3 renamed 4
*/

struct Backupinfile{

	int tap;//created 1 changed 2 created 3 renamed 4
	string FullPath;
	string OldFullPath;
	//created 1 changed 2 created 3 renamed 4
	Backupinfile(int tap_in,string FullPath_in,string OldFullPath_in){
		tap = tap_in;
		FullPath = FullPath_in;
		OldFullPath = OldFullPath_in;
	}
	//created 1 changed 2 created 3 renamed 4
	Backupinfile(int tap_in,string FullPath_in){
		tap = tap_in;
		FullPath = FullPath_in;
		OldFullPath = "";
	}

};

//global var
//同步列表
int len_SysFileList = 1000;
SysFile *SysFileList = new SysFile [len_SysFileList];
int numSaved_SysFileList = 0;


/*
   从全路径截取出文件名
   例如:想从全路径"folder1/folder2/file.txt"得到"file.txt"
        则GetFileNameFromFullPath("folder1/folder2/file.txt");
   ============================================================
   in:
	FullPath 要截取的全路径
   ------------------------------------------------------------
   out:(none)
   ------------------------------------------------------------
   return:
	文件名
*/
string GetFileNameFromFullPath(string FullPath){


	string FileName;
	FileName = FullPath.substr(FullPath.find_last_of('\\')+1);


	return FileName;
}


/*
  返回传入文件的文件类型:文件或文件夹
  ========================================
  in:
	e 要检查的文件
  ----------------------------------------
  out:(none)
  ----------------------------------------
  return:
	返回文件类型
*/
int ScreenFileType(FileSystemEventArgs^ e){
	//跳过$RECYCAL


	//跳过文件夹changed

	//文件或文件夹creat需要查看有没有delete，有就更改同步路径

	//文件changged可能需要备份

	_finddata_t file;
	long lf;
	if ((lf = _findfirst( (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(e->FullPath), &file)) == -1l) {
		cout << "该文件或目录不存在！\n";
		return -1;
	}else{
		return file.attrib ;
	}
	_findclose(lf);

}

/*
   返回FileNameFullPath在同步列表中的位置(返回值从1开始)
   ==========================================================
   in:
	FileNameFullPath 要查找的文件在本地的全地址
	pstart 为从同步列表的第几位开始
   ----------------------------------------------------------
   out:
	在SysList中的位置,在找到第一个时停止
	如果文件不在同步列表则返回-1
*/
int PositionOfSysList(string FileNameFullPath,int pstart = 1){

	int position = pstart-1;


	for(;position<numSaved_SysFileList;position++){
		if(FileNameFullPath == SysFileList[position].FullPath){
			return position+1;
		}

	}

	return -1;
}

//获得用户名
string GetUserName(){
	string username="";

	//NOT
	username = "bsplu";

	return username;
}

//得到备份路径
string GetBackUpPath(){
	string backuppath = "";

	//NOT
	backuppath = "E:\\";

	return backuppath;
}

/*
   当文件需要备份时，调用该函数可以生成一个唯一的文件名
   构成格式为: <时间组成的数字>_UserName
   in:(none)
   out:(none)
   return:
	文件名
*/
string GetTarFileName(){
	string tarfilename;

		stringstream stime;
		stime << int(time(NULL));

		tarfilename = (stime.str() + "_" + GetUserName() + ".zip").c_str();

		return tarfilename;
}


/*
   调用该函数可以备份（压缩）在监控列表中的指定文件
   in:
	lpParam FileNameFullPath
   out:
	0 不是备份文件
	-1 备份失败
*/

DWORD WINAPI BackUpFile(LPVOID lpParam){
	Backupinfile filetap = *((Backupinfile*)lpParam);
	string FileNameFullPath = filetap.FullPath;
	int position = 1;
	string backupfilename = GetTarFileName();

	position = PositionOfSysList(FileNameFullPath);
	SysFileList[position].threadhandle = GetCurrentThreadId();

	

	if( position>0 ){

		Sleep(1000);
		if(SysFileList[position].threadhandle != GetCurrentThreadId())
			return 0;

		//调用压缩备份函数
		cout<<"备份"<<endl;
		system(("makecab /d compressiontype=mszip " + FileNameFullPath + " " + GetBackUpPath() + backupfilename + ">" + " " + GetBackUpPath() + "log").c_str());



	}else{
		return 0;
	}

	do{

		//找到box的备份文档，将更改信息添加进去
		//NOT
			/*
			|将备份信息压入主文件log中
			|将备份信息压入box文件log中
			|
			*/
		break;
		(position = PositionOfSysList(FileNameFullPath,position));
	}while(position>=0);

	return 1;
}

/*
  当检测到文件被创建时，在一些情况，需要跳过不做出反应
  in:
	e 为动作句柄
  return:
	true 需要跳过
	false 不跳过
*/
bool JumpWatherOnCreated(FileSystemEventArgs^ e){

	string FileName = GetFileNameFromFullPath((char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath));


	if(e->FullPath->IndexOf("$RECYCLE.BIN")>=0){
		return true;
	}else if(FileName.find_first_of("$") == 0){
		return true;
	}else if(FileName.find_first_of("~$") == 0){
		return true;
	}else if(FileName.find_first_of('~') == 0){
		return true;
	}

	return false;
}

/*
  当检测到文件发生改变时，在一些情况，需要跳过不做出反应
  in:
	e 为动作句柄
  return:
	true 需要跳过
	false 不跳过
*/
bool JumpWatherOnChanged(FileSystemEventArgs^ e){
	string FileName = GetFileNameFromFullPath((char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath));

	if(e->FullPath->IndexOf("$RECYCLE.BIN")>=0){
		return true;
	}else if(FileName.find_first_of("$") == 0){
		return true;
	}else if(FileName.find_first_of("~$") == 0){
		return true;
	}else if(FileName.find_first_of("~") == 0){
		return true;
	}else if((ScreenFileType(e) & _A_SUBDIR) != 0 ){//文件为文件夹
		return true;
	}

	return false;
}

/*
  当检测到文件被删除时，在一些情况，需要跳过不做出反应
  in:
	e 为动作句柄
  return:
	true 需要跳过
	false 不跳过
*/
bool JumpWatherOndeleted(FileSystemEventArgs^ e){
	string FileName = GetFileNameFromFullPath((char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath));

	if(e->FullPath->IndexOf("$RECYCLE.BIN")>=0){
		return true;
	}else if(FileName.find_first_of("$") == 0){
		return true;
	}else if(FileName.find_first_of("~$") == 0){
		return true;
	}else if(FileName.find_first_of("~") == 0){
		return true;
	}

	return false;
}


/*
  当检测到文件被重命名时，在一些情况，需要跳过不做出反应
  in:
	e 为动作句柄
  return:
	true 需要跳过
	false 不跳过

	*注意:像office的文档文件，当保存时，触发的不是changed而是renamed
*/
bool JumpWatherOnRenamed(RenamedEventArgs^ e){
	string FileNameNew = GetFileNameFromFullPath((char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath));
	string FileNameOld = GetFileNameFromFullPath((char*)(void*)Marshal::StringToHGlobalAnsi(e->OldFullPath));

	if(e->FullPath->IndexOf("$RECYCLE.BIN")>=0){
		return true;
	}else if(FileNameNew.find_first_of("$") == 0){
		return true;
	}else if(FileNameNew.find_first_of("~$") == 0){
		return true;
	}else if(FileNameNew.find_first_of("~") == 0){
		return true;
	}

	return false;

}

/*
   Watcher类，负责监控文件目录，并根据目录下文件不同反应做出相应改变

*/
public ref class Watcher
{
private:
   /*
      当检测到文件创建改变时调用的函数
   */
   static void OnCreated( Object^ /*source*/, FileSystemEventArgs^ e )
   {
      // Specify what is done when a file is created.
		if(JumpWatherOnCreated(e)){
				return ;
		}
      Console::WriteLine( "File: {0} {1}", e->FullPath, e->ChangeType );
	  
   }

   /*
      当检测到文件被删除时调用的函数
   */
     static void OnDeleted( Object^ /*source*/, FileSystemEventArgs^ e )
   {
      // Specify what is done when a file is deleted.
	   if(JumpWatherOndeleted(e)){
				return ;
		}
		Console::WriteLine( "File: {0} {1}", e->FullPath, e->ChangeType );
	  
   }

   /*
      当检测到文件发生改变时调用的函数
   */
      static void OnChanged( Object^ /*source*/, FileSystemEventArgs^ e )
   {
      // Specify what is done when a file is deleted.
	   if(JumpWatherOnChanged(e)){
				return ;
		}

		
		//PositionOfSysList((char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath));
		

      Console::WriteLine( "File: {0} {1}", e->FullPath, e->ChangeType );

	  Backupinfile filetap(2,(char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath),(char*)(void*)Marshal::StringToHGlobalAnsi(e->OldFullPath));
	  HANDLE hThread = CreateThread(NULL, 0, BackUpFile, &filetap, 0, NULL);
	  
   }

   /*
      当检测到文件重命名时调用的函数
   */
   static void OnRenamed( Object^ /*source*/, RenamedEventArgs^ e )
   {
      // Specify what is done when a file is renamed.
	   if(JumpWatherOnRenamed(e)){
			return ;
	   }
      Console::WriteLine( "File: {0} renamed to {1}", e->OldFullPath, e->FullPath );

	  Backupinfile filetap(4,(char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath),(char*)(void*)Marshal::StringToHGlobalAnsi(e->OldFullPath));
	  HANDLE hThread = CreateThread(NULL, 0, BackUpFile, &filetap, 0, NULL);

   }

public:
   [PermissionSet(SecurityAction::Demand, Name="FullTrust")]

   /*
     监控文件的主要函数，调用该函数监控某一文件夹下的文件变动
	 in:
		MonitorPath 需要监控的文件目录
   */
   int static monitor(String^ MonitorPath)
   {
	   /*
      array<String^>^args = System::Environment::GetCommandLineArgs();
	  int i = 0;

      // If a directory is not specified, exit program.
      if ( args->Length != 2 )
      {
         // Display the proper way to call the program.
         Console::WriteLine( "Usage: Watcher.exe (directory)" );
         return 0;
      }
	  */

      // Create a new FileSystemWatcher and set its properties.
      FileSystemWatcher^ watcher = gcnew FileSystemWatcher;

	  //对路径的设定
      watcher->Path = MonitorPath;


      /* Watch for changes in LastAccess and LastWrite times, and 
          the renaming of files or directories. */
      watcher->NotifyFilter = static_cast<NotifyFilters>(NotifyFilters::LastAccess |
            NotifyFilters::LastWrite | NotifyFilters::FileName | NotifyFilters::DirectoryName);//属性

      // Only watch text files.
      watcher->Filter = "*.*";//查看类型

	  //监视子目录
	  watcher->IncludeSubdirectories = true;
	  
      // Add event handlers.
      watcher->Changed += gcnew FileSystemEventHandler( Watcher::OnChanged );
      watcher->Created += gcnew FileSystemEventHandler( Watcher::OnCreated );
      watcher->Deleted += gcnew FileSystemEventHandler( Watcher::OnDeleted );
      watcher->Renamed += gcnew RenamedEventHandler( Watcher::OnRenamed );

	  
      // Begin watching.
      watcher->EnableRaisingEvents = true;
	  
      // Wait for the user to quit the program.
      Console::WriteLine( "Press \'q\' to quit the sample." );
	  
	  while ( Console::Read() != 'q');//当检测的文件或文件夹被删除时也应结束掉任务

      return 0;
   }
};

int main() {

	//test
	SysFileList[0].addSysFile("D:\\test\\1.txt","testbox");
	numSaved_SysFileList = 1;

	SysFileList[1].addSysFile("D:\\test\\2.docx","testbox");
	numSaved_SysFileList ++;


	//应该创建多个进程，每个盘符创建一个
   Watcher::monitor("D:\\");

	//outchange();
}
