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

using namespace System;
using namespace System::IO;
using namespace System::Security::Permissions;
using namespace std;
using namespace System::Runtime::InteropServices;


//Function Name
string GetFileNameFromeFullPath(string FullPath);
int ScreenFileType(FileSystemEventArgs^ e);
int PositionOfSysList(string FileNameFullPath,int pstart);
int BackUpFile(string FileNameFullPath);
string GetTarFileName();
string GetUserName();//NOT
string GetBackUpPath();//NOT


//class name
class SysFile;



//Function & class ����

class SysFile{

public:
	string FileName;
	string BoxName;//box����
	string FullPath;//�ļ��ڴ��̵�·��
	string BoxFullPath;//box�е������ļ���·��,����  BoxName::�����ļ�����/�ļ���

	SysFile(){

		FileName = "";
		BoxName = "";
		FullPath = "";
		BoxFullPath = "";
	}

	//Ĭ���ļ�������box�ĸ�Ŀ¼��
	SysFile(string FullPath_input,string BoxName_input){

		FullPath = FullPath_input;
		BoxName = BoxName_input;
		FileName = GetFileNameFromeFullPath(FullPath);
		BoxFullPath = BoxName+"::"+FileName;
	}

	SysFile(string FullPath_input,string BoxName_input,string BoxFullPath){

		FullPath = FullPath_input;
		FileName = GetFileNameFromeFullPath(FullPath);
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
		FileName = GetFileNameFromeFullPath(FullPath);
		BoxFullPath = BoxName+"::"+FileName;
	}
};
	
//global var
//ͬ���б�
int len_SysFileList = 1000;
SysFile *SysFileList = new SysFile [len_SysFileList];
int numSaved_SysFileList = 0;


//��ȫ·����ȡ���ļ���
string GetFileNameFromeFullPath(string FullPath){


	string FileName;
	FileName = FullPath.substr(FullPath.find_last_of('/')+1);


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

		tarfilename = (stime.str() + ":" + GetUserName() + ".tar.gz").c_str();

		return tarfilename;
}


//�ļ��ı䣬���ļ����ݣ��������ļ����ݼ�¼(����ѹ����)��¼��box�����ĵ���
//in:FileNameFullPath
//out:0���Ǳ����ļ�
//	-1����ʧ��
int BackUpFile(string FileNameFullPath){

	int position = 1;
	string backupfilename = GetTarFileName();

	if(position = PositionOfSysList(FileNameFullPath) >0 ){
		//����ѹ�����ݺ���
		cout<<"����"<<endl;
		system(("tar -zcvf " + FileNameFullPath + " " + GetBackUpPath() + backupfilename + "--force-local").c_str());

	}else{
		return 0;
	}

	do{

		//�ҵ�box�ı����ĵ�����������Ϣ��ӽ�ȥ
		//NOT
		break;

	}while((position = PositionOfSysList(FileNameFullPath,position))>=0);

	return 1;
}


public ref class Watcher
{
private:
   // Define the event handlers.
   static void OnCreated( Object^ /*source*/, FileSystemEventArgs^ e )
   {
      // Specify what is done when a file is created.
		if(e->FullPath->IndexOf("$RECYCLE.BIN")>=0){
				return ;
		}
      Console::WriteLine( "File: {0} {1}", e->FullPath, e->ChangeType );
	  
   }

      static void OnDeleted( Object^ /*source*/, FileSystemEventArgs^ e )
   {
      // Specify what is done when a file is deleted.
	   	if(e->FullPath->IndexOf("$RECYCLE.BIN")>=0){
				return ;
		}
		Console::WriteLine( "File: {0} {1}", e->FullPath, e->ChangeType );
	  
   }

      static void OnChanged( Object^ /*source*/, FileSystemEventArgs^ e )
   {
      // Specify what is done when a file is deleted.
		if(e->FullPath->IndexOf("$RECYCLE.BIN")>=0){
				return ;
		}

		//�ļ�Ϊ�ļ���
		if((ScreenFileType(e) & _A_SUBDIR) != 0 ){
			return ;
		}
		
		//PositionOfSysList((char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath));
		

      Console::WriteLine( "File: {0} {1}", e->FullPath, e->ChangeType );

	  //BackUpFile((char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath));
	  
   }

   static void OnRenamed( Object^ /*source*/, RenamedEventArgs^ e )
   {
      // Specify what is done when a file is renamed.
      Console::WriteLine( "File: {0} renamed to {1}", e->OldFullPath, e->FullPath );
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
	  while ( Console::Read() != 'q' );//�������ļ����ļ��б�ɾ��ʱҲӦ����������

      return 0;
   }
};

int main() {

	//test
	SysFileList[0].addSysFile("D:\\test\1.txt","testbox");
	len_SysFileList = 1;


	//Ӧ�ô���������̣�ÿ���̷�����һ��
   Watcher::monitor("D:\\");
	//outchange();
}
