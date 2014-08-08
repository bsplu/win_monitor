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



//Function & class ����

class SysFile{

public:
	string FileName;
	string BoxName;//box����
	string FullPath;//�ļ��ڴ��̵�·��
	string BoxFullPath;//box�е������ļ���·��,����  BoxName::�����ļ�����/�ļ���
	int handle;

	SysFile(){

		FileName = "";
		BoxName = "";
		FullPath = "";
		BoxFullPath = "";
		handle = 0;
	}

	//Ĭ���ļ�������box�ĸ�Ŀ¼��
	SysFile(string FullPath_input,string BoxName_input){

		FullPath = FullPath_input;
		BoxName = BoxName_input;
		FileName = GetFileNameFromFullPath(FullPath);
		BoxFullPath = BoxName+"::"+FileName;
	}

	SysFile(string FullPath_input,string BoxName_input,string BoxFullPath){

		FullPath = FullPath_input;
		FileName = GetFileNameFromFullPath(FullPath);
		BoxName = BoxName_input;
		BoxFullPath = BoxFullPath;
	}

	SysFile operator=(SysFile file_in){
		this->FileName = file_in.FileName;
		this->BoxName = file_in.BoxName;
		this->FullPath = file_in.FullPath;
		this->BoxFullPath = file_in.BoxFullPath;

		return *this;
	}

	void addSysFile(string FullPath_input,string BoxName_input){
		FullPath = FullPath_input;
		BoxName = BoxName_input;
		FileName = GetFileNameFromFullPath(FullPath);
		BoxFullPath = BoxName+"::"+FileName;
	}
};
	
//global var
//ͬ���б�
int len_SysFileList = 1000;
SysFile *SysFileList = new SysFile [len_SysFileList];
int numSaved_SysFileList = 0;


//��ȫ·����ȡ���ļ���
string GetFileNameFromFullPath(string FullPath){


	string FileName;
	FileName = FullPath.substr(FullPath.find_last_of('\\')+1);


	return FileName;
}


//�������ĵ����ͣ���������Ӧ��Ϊ
int ScreenFileType(FileSystemEventArgs^ e){
	//����$RECYCAL


	//�����ļ���changed

	//�ļ����ļ���creat��Ҫ�鿴��û��delete���о͸���ͬ��·��

	//�ļ�changged������Ҫ����

	_finddata_t file;
	long lf;
	if ((lf = _findfirst( (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(e->FullPath), &file)) == -1l) {
		cout << "���ļ���Ŀ¼�����ڣ�\n";
		return -1;
	}else{
		return file.attrib ;
	}
	_findclose(lf);

}


	//����FileNameFullPath��ͬ���б��е�λ��
	//in:FileNameFullPathΪҪ���ҵ��ļ�ȫ��ַ
	//   pstartΪ�ӵڼ�λ��ʼ
	//out:��SysList�е�λ��,���ҵ���һ��ʱֹͣ
	//����ļ�����ͬ���б��򷵻�-1
int PositionOfSysList(string FileNameFullPath,int pstart = 1){

	int position = pstart-1;


	for(;position<numSaved_SysFileList;position++){
		if(FileNameFullPath == SysFileList[position].FullPath){
			return position+1;
		}

	}

	return -1;
}

//����û���
string GetUserName(){
	string username="";

	//NOT
	username = "bsplu";

	return username;
}

//�õ�����·��
string GetBackUpPath(){
	string backuppath = "";

	//NOT
	backuppath = "E:\\";

	return backuppath;
}

//�õ�ѹ���ļ���
string GetTarFileName(){
	string tarfilename;

		stringstream stime;
		stime << int(time(NULL));

		tarfilename = (stime.str() + "_" + GetUserName() + ".zip").c_str();

		return tarfilename;
}


//�ļ��ı䣬���ļ����ݣ��������ļ����ݼ�¼(����ѹ����)��¼��box�����ĵ���
//in:FileNameFullPath
//out:0���Ǳ����ļ�
//	-1����ʧ��
DWORD WINAPI BackUpFile(LPVOID lpParam){

	string FileNameFullPath = string((char*)(lpParam));
	int position = 1;
	string backupfilename = GetTarFileName();

	position = PositionOfSysList(FileNameFullPath);
	SysFileList[position].handle = GetCurrentThreadId();

	

	if( position>0 ){

		Sleep(1000);
		if(SysFileList[position].handle != GetCurrentThreadId())
			return 0;

		//����ѹ�����ݺ���
		cout<<"����"<<endl;
		system(("makecab /d compressiontype=mszip " + FileNameFullPath + " " + GetBackUpPath() + backupfilename + ">" + " " + GetBackUpPath() + "log").c_str());

	}else{
		return 0;
	}

	do{

		//�ҵ�box�ı����ĵ�����������Ϣ��ӽ�ȥ
		//NOT
		break;
		(position = PositionOfSysList(FileNameFullPath,position));
	}while(position>=0);

	return 1;
}


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
	}else if((ScreenFileType(e) & _A_SUBDIR) != 0 ){//�ļ�Ϊ�ļ���
		return true;
	}

	return false;
}

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
public ref class Watcher
{
private:
   // Define the event handlers.
   static void OnCreated( Object^ /*source*/, FileSystemEventArgs^ e )
   {
      // Specify what is done when a file is created.
		if(JumpWatherOnCreated(e)){
				return ;
		}
      Console::WriteLine( "File: {0} {1}", e->FullPath, e->ChangeType );
	  
   }

     static void OnDeleted( Object^ /*source*/, FileSystemEventArgs^ e )
   {
      // Specify what is done when a file is deleted.
	   if(JumpWatherOndeleted(e)){
				return ;
		}
		Console::WriteLine( "File: {0} {1}", e->FullPath, e->ChangeType );
	  
   }

      static void OnChanged( Object^ /*source*/, FileSystemEventArgs^ e )
   {
      // Specify what is done when a file is deleted.
	   if(JumpWatherOnChanged(e)){
				return ;
		}

		
		//PositionOfSysList((char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath));
		

      Console::WriteLine( "File: {0} {1}", e->FullPath, e->ChangeType );

	  HANDLE hThread = CreateThread(NULL, 0, BackUpFile, (char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath), 0, NULL);

	  
   }

   static void OnRenamed( Object^ /*source*/, RenamedEventArgs^ e )
   {
      // Specify what is done when a file is renamed.
	   if(JumpWatherOnRenamed(e)){
			return ;
	   }
      Console::WriteLine( "File: {0} renamed to {1}", e->OldFullPath, e->FullPath );

	  HANDLE hThread = CreateThread(NULL, 0, BackUpFile, (char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath), 0, NULL);

   }

public:
   [PermissionSet(SecurityAction::Demand, Name="FullTrust")]
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

	  //��·�����趨
      watcher->Path = MonitorPath;


      /* Watch for changes in LastAccess and LastWrite times, and 
          the renaming of files or directories. */
      watcher->NotifyFilter = static_cast<NotifyFilters>(NotifyFilters::LastAccess |
            NotifyFilters::LastWrite | NotifyFilters::FileName | NotifyFilters::DirectoryName);//����

      // Only watch text files.
      watcher->Filter = "*.*";//�鿴����

	  //������Ŀ¼
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
	  
	  while ( Console::Read() != 'q');//�������ļ����ļ��б�ɾ��ʱҲӦ����������

      return 0;
   }
};

int main() {

	//test
	SysFileList[0].addSysFile("D:\\test\\1.txt","testbox");
	numSaved_SysFileList = 1;

	SysFileList[1].addSysFile("D:\\test\\2.docx","testbox");
	numSaved_SysFileList ++;


	//Ӧ�ô���������̣�ÿ���̷�����һ��
   Watcher::monitor("D:\\");

	//outchange();
}
