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
#include <locale> 

#include"D:/workspace/boosttreetest/boosttreetest/file_tree.hpp"


using namespace System;
using namespace System::IO;
using namespace System::Security::Permissions;
using namespace std;
using namespace System::Runtime::InteropServices;

const string NAME_PRESENT_BOX_BRANCH_NUMBER_TXT="PresentBoxBranchNumber.txt";
const string NAME_ROOT_SYSLIST_LOG = "RootSyslist.log";



//Function Name

int ScreenFileType(FileSystemEventArgs^ e);
int PositionOfSysList(string FileNameFullPathInDisc,int pstart);
DWORD WINAPI BackUpFile(LPVOID lpParam);
string GetTarFileName();
string GetUserName();//NOT
string GetBackUpPath();//NOT
bool JumpWatherOnCreated(FileSystemEventArgs^ e);
bool JumpWatherOnChanged(FileSystemEventArgs^ e);
bool JumpWatherOnDeleted(FileSystemEventArgs^ e);
bool JumpWatherOnRenamed(FileSystemEventArgs^ e);
string GetBoxNameFromFullPathInBox(string FullPathInBox);
int GetPresentBoxBranchNumber(string BoxName);


//class name
class SysFile;



//Function & class ����

/*
  ������Ҫ�ǽ��ļ���Ϣ���ϵ���һ���ط�
*/
class SysFile{

public:
	string FileName;
	string BoxName;//box����
	string FullPathInDisc;//�ļ��ڴ��̵�·��
	string FullPathInBox;//box�е������ļ���·��,����  BoxName/�����ļ�����/�ļ���
	int BranchNumber;
	int threadhandle;//���浱ǰ���ø��ļ����̺߳ţ����û����Ϊ0

	SysFile(){

		FileName = "";
		BoxName = "";
		FullPathInDisc = "";
		FullPathInBox = "";
		threadhandle = 0;
		BranchNumber = 0;
	}

	//Ĭ���ļ�������box�ĸ�Ŀ¼��
	SysFile(string FullPathInDisc_input,string FullPathInBox_in){

		FullPathInDisc = FullPathInDisc_input;
		BoxName = GetBoxNameFromFullPathInBox(FullPathInBox_in);
		FileName = GetFileNameFromFullPath(FullPathInDisc);
		FullPathInBox = FullPathInBox_in;
		BranchNumber = 0;
		threadhandle = 0;
	}

	SysFile(string FullPathInDisc_input,string FullPathInBox_in,int BranchNumber_in){

		FullPathInDisc = FullPathInDisc_input;
		FileName = GetFileNameFromFullPath(FullPathInDisc);
		BoxName = GetBoxNameFromFullPathInBox(FullPathInBox_in);
		FullPathInBox = FullPathInBox_in;
		BranchNumber = BranchNumber_in;
		threadhandle = 0;

	}

	SysFile operator=(SysFile file_in){
		this->FileName = file_in.FileName;
		this->BoxName = file_in.BoxName;
		this->FullPathInDisc = file_in.FullPathInDisc;
		this->FullPathInBox = file_in.FullPathInBox;
		this->BranchNumber = file_in.BranchNumber;

		return *this;
	}

	void addSysFile(string FullPathInDisc_input,string FullPathInBox_in,int BranchNumber_in){
		FullPathInDisc = FullPathInDisc_input;
		FileName = GetFileNameFromFullPath(FullPathInDisc);
		BoxName = GetBoxNameFromFullPathInBox(FullPathInBox_in);
		FullPathInBox = FullPathInBox_in;
		BranchNumber = BranchNumber_in;
		threadhandle = 0;
	}
};

/*
  �ýṹ��Ҫ�Ǵ���backupfile����ʹ��
  //tap Ϊ�ļ��������� created 1 changed 2 deleted 3 renamed 4
*/
class Backupinfile{

public:
	int  tap;//created 1 changed 2 created 3 renamed 4
	string  FullPathInDisc;
	string  OldFullPathInDisc;
	//created 1 changed 2 created 3 renamed 4
	
	Backupinfile(int tap_in,string FullPathInDisc_in,string OldFullPathInDisc_in){
		tap = tap_in;
		FullPathInDisc = FullPathInDisc_in;
		OldFullPathInDisc = OldFullPathInDisc_in;
	}
	//created 1 changed 2 created 3 renamed 4
	Backupinfile(int tap_in,string FullPathInDisc_in){
		tap = tap_in;
		FullPathInDisc = FullPathInDisc_in;
		OldFullPathInDisc = "";
	}

	
	Backupinfile operator=(Backupinfile a){
		this->tap = a.tap;
		this->FullPathInDisc = a.FullPathInDisc;
		this->OldFullPathInDisc = a.OldFullPathInDisc;

		return (*this);
	}
	
};

//global var
//ͬ���б�
int len_SysFileList = 1000;
SysFile *SysFileList = new SysFile [len_SysFileList];
int numSaved_SysFileList = 0;


/*
   ͬ���б�����
*/
int ExpandSysList(){
	SysFile *SysFileListTemp = new SysFile [len_SysFileList];
	for(int i=0;i<len_SysFileList;i++){
		SysFileListTemp[i] = SysFileList[i];
	}
	delete []SysFileList;
	SysFileList = NULL;
	SysFileList = new SysFile [2*len_SysFileList];
	for(int i=0;i<len_SysFileList;i++){
		SysFileList[i] = SysFileListTemp[i];
	}
	delete [] SysFileListTemp;

	len_SysFileList *= 2;
	return 1;
}

/*
  ��File_in�����ͬ���б���
*/
int PutSysFileInSysList(SysFile File_in){
	SysFileList[numSaved_SysFileList] = File_in;
	numSaved_SysFileList++;

	if(numSaved_SysFileList == len_SysFileList){
		ExpandSysList();
	}

	return 1;
}

/*
   ��Syslistд��FullPathToStoreSyslist�ļ���
   in:
	SyslistҪ������ļ�·��
   out:(none)
   return:
	1Ϊ�ɹ�
*/
int write_Syslist(string FullPathToStoreSyslist){
	ofstream fout(FullPathToStoreSyslist);
	for(int i=0;i<numSaved_SysFileList;i++){
		fout<<SysFileList[i].BranchNumber<<">"<<SysFileList[i].FullPathInDisc<<">"
			<<SysFileList[i].FullPathInBox<<endl;
	}
	fout.close();
	return 1;
} 

/*
  ��syslist���ļ��ж���
  in:
	FullPathToStoreSyslist ����syslist���ļ�ȫ·��
  out:(none)
  return:
	1Ϊ�ɹ�
*/
int read_Syslist(string FullPathToStoreSyslist){
	FILE* fp = NULL; 
	fp = fopen((FullPathToStoreSyslist).c_str(),"r");
	if(NULL==fp)
	{
		return -1;
	}
	char chartxt[1024];
	string strtxt;
	int BranchNumber = 1;

	string FullPathInDisc;
	string FullPathInBox;

	while(!feof(fp)){
		memset(chartxt , 0 , sizeof(chartxt) );
		fgets(chartxt,1024,fp);
		if(string(chartxt) == ""){
			break;
		}
		strtxt = chartxt;
		int pos = strtxt.find(">");
		int start = 0;
		BranchNumber = atoi((strtxt.substr(start,pos).c_str()));
		start += (pos+1);
		pos = strtxt.find(">",start)- start;
		FullPathInDisc = strtxt.substr(start,pos);
		start += (pos+1);
		pos = strtxt.length()-1- start;
		FullPathInBox = strtxt.substr(start,pos);
		//add
		SysFile File(FullPathInDisc,FullPathInBox,BranchNumber);
		PutSysFileInSysList(File);
	}
	
	fclose(fp);
	return 1;
} 




/*
  ���ش����ļ����ļ�����:�ļ����ļ���
  ========================================
  in:
	e Ҫ�����ļ�
  ----------------------------------------
  out:(none)
  ----------------------------------------
  return:
	�����ļ�����
*/
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

/*
   ����FileNameFullPathInDisc��ͬ���б��е�λ��(����ֵ��1��ʼ)
   ==========================================================
   in:
	FileNameFullPath Ҫ���ҵ��ļ��ڱ��ش��̵�ȫ��ַ
	pstart Ϊ��ͬ���б�ĵڼ�λ��ʼ
   ----------------------------------------------------------
   out:
	��SysList�е�λ��,���ҵ���һ��ʱֹͣ
	����ļ�����ͬ���б��򷵻�-1
*/
int PositionOfSysList(string FileNameFullPathInDisc,int pstart = 1){

	int position = pstart-1;


	for(;position<numSaved_SysFileList;position++){
		if(FileNameFullPathInDisc == SysFileList[position].FullPathInDisc){
			return position+1;
		}

	}

	return -1;
}

/*
	����folder��ͬ���б��е�һ�γ��ֵ�λ��
	in:
		folderfullPath folder�ڴ����е�ȫ��ַ
		start ��syslist�еڼ�λ��ʼ���ң�Ĭ�ϴӵ�һλ����
	out:(none)
	return:
	-1 Ϊû���ҵ�
	>0 Ϊ�ҵ��ĵ�һ��λ��
*/
int PositonOfFolderFullPathInSyslist(string FolderFullPath,int start = 1){

	
	for(int i=(start>=1?start-1:0);i<numSaved_SysFileList;i++){
		if(SysFileList[i].FullPathInDisc.find(FolderFullPath) < SysFileList[i].FullPathInDisc.length()){
			return i+1;
		}
	}

	return -1;
}



int RenamedFolderPathInSysList(string OldPath,string NewPath){
	//�����β��"\\",ȥ���������ļ�renamedҲ�ʺ�
	if(OldPath.find_last_of("\\") == (OldPath.length()-1)){
		OldPath.erase(OldPath.find_last_of("\\"));
	}else{

	}
	if(NewPath.find_last_of("\\") == (NewPath.length()-1)){
		NewPath.erase(NewPath.find_last_of("\\"));
	}else{

	}

	for(int i=0;i<numSaved_SysFileList;i++){
		SysFileList[i].FullPathInDisc = ReplaceString(SysFileList[i].FullPathInDisc,OldPath,NewPath);
	}

	return 1;
}


/*
   �õ�Box��ǰ���ڷ�֧��branchNumber
   in:
	BoxName Ҫ����box������
   out:(none)
   return:
	branchnumber
	-1 û���ҵ�box

*/
int GetPresentBoxBranchNumber(string BoxName){
	FILE* fp = NULL; 
	fp = fopen((GetBackUpPath()+NAME_PRESENT_BOX_BRANCH_NUMBER_TXT).c_str(),"r");
	if(NULL==fp)
	{
		return -1;
	}
	char strtxt[1024];
	string BoxNameRead;
	int BranchNumberRead;
	while(!feof(fp)){
		fgets(strtxt,1024,fp);
		BoxNameRead = string(strtxt).substr(0,string(strtxt).find(">"));
		BranchNumberRead = atoi((string(strtxt).substr(string(strtxt).find(">")+1)).c_str());
		if(BoxNameRead == BoxName){
			fclose(fp);
			return BranchNumberRead;
		}
	}
	
	fclose(fp);
	return -1;
	
}

/*
   ���ͬ���ļ����������ļ������ͬ���б����ļ������box�У�
   in:
	fileFullPathInDisc Ҫ����ļ���ȫ·��
	ToFullPathInBox Ҫ��ӵ�box·��,�������ļ���
	BranchNumber ΪҪ��ӵ�box�ķ�֧��,Ĭ��Ϊ��ӵ�����֧
   out:
	pt_root ����Ӻ���ļ�tree����
   return:
	1 Ϊ��ӳɹ�
	-1 0 Ϊʧ��
	-2 Ϊbox������ͬ���ֵ��ļ�
*/
int AddFileToSys(string fileFullPathInDisc,string ToBoxPath,ptree& pt_root){
	//NOT
	/*
	Ҫ�ж�fileFullPathInDisc ToBoxPath �Ƿ��Ѿ���ͬ���б�����
	*/
	string FileName = GetFileNameFromFullPath(fileFullPathInDisc);
	int BranchNumber = 1;
	string FullPathInBox;

	if(ToBoxPath.find_last_of("/") == (ToBoxPath.length()-1)){
		FullPathInBox = ToBoxPath + FileName;
	}else{
		FullPathInBox = ToBoxPath + ("/" + FileName);
	}

	string BoxName = GetBoxNameFromFullPathInBox(FullPathInBox);

	BranchNumber = GetPresentBoxBranchNumber(BoxName);

	if(BranchNumber == -1){
		BranchNumber = 1;
	}

	int return_value = AddFilePathToRoot(pt_root,FullPathInBox);
	if(return_value != 1){
		return return_value;
	}

	//���ļ������ͬ���б�
	SysFile File_in(fileFullPathInDisc,FullPathInBox,BranchNumber);
	PutSysFileInSysList(File_in);

	//����syslist.log
	write_Syslist(GetBackUpPath()+NAME_ROOT_SYSLIST_LOG);
	
	stringstream sBranchNumber;
	sBranchNumber << BranchNumber;
	write_box_xml(pt_root,BoxName,GetBackUpPath()+"boxs\\"+BoxName+"\\branches\\"+sBranchNumber.str()+"\\ptreebox.xml");
	ofstream fout(GetBackUpPath()+"boxs\\"+BoxName+"\\branches\\branchnumber.txt",ios::app);
	fout<<BranchNumber<<endl;
	fout.close();
	write_xml_CHECKPATH(GetBackUpPath()+"ptreeroot.xml",pt_root);
	

	//����
	Backupinfile *filetap = new Backupinfile(1,fileFullPathInDisc);
	HANDLE hThread = CreateThread(NULL, 0, BackUpFile, filetap, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);  
	CloseHandle(hThread);  
	
	return 1;
}

/*
	�ı�box�е������ļ������ƣ������box����
	in:
		pt_root ��box
		OldPath ԭ�ļ��е�ȫ·�� 
		NewPath ���ļ��е�ȫ·��
	out:
		pt_root ��box
	return:
		-1 ʧ�ܣ�OldPath NewPath���Ĳ�ֹһ��
		1�ɹ�
*/
int RenameFolderInBoxName(ptree& pt_root,string OldPath,string NewPath){
	//��Ҫ��Ӹ���boxname�ĺ���
		
	if(OldPath == "" || NewPath == ""){
		return 1;
	}

	//�ж����һλ�Ƿ�Ϊ'/'
	if(OldPath.find_last_of("/") == (OldPath.length()-1)){
		OldPath.erase(OldPath.length()-1);
	}else{

	}

	if(NewPath.find_last_of("/") == (NewPath.length()-1)){
		NewPath.erase(NewPath.length()-1);
	}else{

	}

	//�ж�һ���Ƿ�ֻ�����һ���ļ����ֲ�ͬ
	if(OldPath.substr(0,OldPath.find_last_of("/")>(OldPath.length()-unsigned(1))?
		0:OldPath.find_last_of("/")) != 
		NewPath.substr(0,OldPath.find_last_of("/")>(NewPath.length()-unsigned(1))?
		0:NewPath.find_last_of("/"))){
		return -1;
	}

	bool changeboxname = false;

	if(GetBoxNameFromFullPathInBox(OldPath) == OldPath){
		changeboxname = true;
	}
	//����ͬ���б�
	for(int i=0;i<numSaved_SysFileList;i++){
		string str = SysFileList[i].FullPathInBox;
		if(str.find(OldPath) < str.length()-unsigned(1)){
			SysFileList[i].FullPathInBox = NewPath+str.substr(OldPath.length());
			SysFileList[i].BoxName = GetBoxNameFromFullPathInBox(SysFileList[i].FullPathInBox);
		}
	}


	//����tree
	bool lastfolder = false;
	string itor = OldPath;
	if(itor != "" && itor.find("/") >= int(itor.length())){
		lastfolder = true;
	}

	//����branches�ļ��У��������ÿһ��box������һ��
	try{
		for(auto itbox = pt_root.get_child("root.boxs").begin(); itbox != pt_root.get_child("root.boxs").end();++itbox){
			//��ø�branchnumber
			const int MAX_BRANCHNUMBER = 1000;
			int branchnumber[MAX_BRANCHNUMBER];
			int num_branchnumber = 0;

			string BoxName = itbox->second.get<string>("name");
			ifstream fin(GetBackUpPath()+"boxs\\"+BoxName+"\\branches\\branchnumber.txt");//ֻ��
			if(!fin){
				return -1;
			}
			while(fin){
				  
				fin>>branchnumber[num_branchnumber];
				if(fin.fail()) {  
					 break;  
				}
				cout<<branchnumber[num_branchnumber]<<endl;
				num_branchnumber++;
			};

			fin.close();
			for(int i=0;i<num_branchnumber;i++){
				ptree BoxNode;
				stringstream sBranchNumber;
				sBranchNumber << branchnumber[i];
				read_xml_CHECKPATH(GetBackUpPath()+"boxs\\"+BoxName+"\\branches\\" + sBranchNumber.str() +"\\ptreebox.xml",BoxNode);
				if(lastfolder && BoxNode.get_child("box").get<string>("name","") == itor){
					BoxNode.get_child("box").put("name",GetFileNameFromFullPath(NewPath,"/"));
					RenamedFolderPathInNode(BoxNode.get_child("box"),OldPath,NewPath);
				}else if(itor != "" && (!lastfolder) && BoxNode.get_child("box").get<string>("name","") == GetBoxNameFromFullPathInBox(itor)){
					RenamedFolderPathInNode(BoxNode.get_child("box"),OldPath,NewPath,itor.substr(itor.find("/")+1));
				}else{
					RenamedFolderPathInNode(BoxNode.get_child("box"),OldPath,NewPath);
				}
				//����дtree
				write_xml_CHECKPATH(GetBackUpPath()+"boxs\\"+BoxName+"\\branches\\" + sBranchNumber.str() +"\\ptreebox.xml",BoxNode);
			}

			if(changeboxname){
				system(("rename " + GetBackUpPath()+"boxs\\"+BoxName + " " + NewPath ).c_str());
			}
		}		
	}
	catch(...){

	}
	if(RenamedFolderPathInRoot(pt_root,OldPath,NewPath) == -1){
		return -1;
	}

	write_Syslist(GetBackUpPath()+NAME_ROOT_SYSLIST_LOG);
	write_xml_CHECKPATH(GetBackUpPath()+"ptreeroot.xml",pt_root);

	return 1;

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
	backuppath = "E:\\testbackupfolder\\";

	return backuppath;
}

/*
   ���ļ���Ҫ����ʱ�����øú�����������һ��Ψһ���ļ���
   ���ɸ�ʽΪ: <ʱ����ɵ�����>_UserName
   in:(none)
   out:(none)
   return:
	�ļ���
*/
string GetTarFileName(){
	string tarfilename;

		stringstream stime;
		stime << int(time(NULL));

		tarfilename = (stime.str() + "_" + GetUserName() + ".zip").c_str();

		return tarfilename;
}


/*
   ���øú������Ա��ݣ�ѹ�����ڼ���б��е�ָ���ļ�
   in:
	lpParam Backupinfile
   out:
	0 ���Ǳ����ļ�
	-1 ����ʧ��
*/

DWORD WINAPI BackUpFile(LPVOID lpParam){

	Backupinfile* filetap = (Backupinfile*) lpParam;

	string FileFullPathInDisc = (*filetap).FullPathInDisc;
	string FileOldeFullPathInDisc = (*filetap).OldFullPathInDisc;
	
	int position = 1;
	string backupfilename = GetTarFileName();

	if((*filetap).tap == 4){
		position = PositionOfSysList(FileOldeFullPathInDisc);
	}else{
		position = PositionOfSysList(FileFullPathInDisc);
	}

	SysFileList[position].threadhandle = GetCurrentThreadId();
 

	if( position>0 ){

		Sleep(1000);
		if(SysFileList[position].threadhandle != GetCurrentThreadId())
			return 0;

		//����ѹ�����ݺ���
		cout<<"����"<<endl;
		system(("makecab /d compressiontype=mszip " + FileFullPathInDisc + " " + GetBackUpPath() + backupfilename + " >nul 2>nul" ).c_str());


	}else{
		return 0;
	}

	do{

		//�ҵ�box�ı����ĵ�����������Ϣ��ӽ�ȥ
		//NOT
			/*
			|��������Ϣѹ�����ļ�log��
			|��������Ϣѹ��box�ļ�log��
			|
			*/

		break;
		(position = PositionOfSysList(FileFullPathInDisc,position));
	}while(position>=0);


	delete filetap;
	return 1;
}

/*
  ����⵽�ļ�������ʱ����һЩ�������Ҫ������������Ӧ
  in:
	e Ϊ�������
  return:
	true ��Ҫ����
	false ������
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
  ����⵽�ļ������ı�ʱ����һЩ�������Ҫ������������Ӧ
  in:
	e Ϊ�������
  return:
	true ��Ҫ����
	false ������
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
	}else if((ScreenFileType(e) & _A_SUBDIR) != 0 ){//�ļ�Ϊ�ļ���
		return true;
	}

	return false;
}

/*
  ����⵽�ļ���ɾ��ʱ����һЩ�������Ҫ������������Ӧ
  in:
	e Ϊ�������
  return:
	true ��Ҫ����
	false ������
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
  ����⵽�ļ���������ʱ����һЩ�������Ҫ������������Ӧ
  in:
	e Ϊ�������
  return:
	true ��Ҫ����
	false ������

	*ע��:��office���ĵ��ļ���������ʱ�������Ĳ���changed����renamed
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
   Watcher�࣬�������ļ�Ŀ¼��������Ŀ¼���ļ���ͬ��Ӧ������Ӧ�ı�

*/
public ref class Watcher
{
private:
	static String^  MonitorPath; //����·��

   /*
      ����⵽�ļ������ı�ʱ���õĺ���
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
      ����⵽�ļ���ɾ��ʱ���õĺ���
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
      ����⵽�ļ������ı�ʱ���õĺ���
   */
      static void OnChanged( Object^ /*source*/, FileSystemEventArgs^ e )
   {
      // Specify what is done when a file is deleted.
	   if(JumpWatherOnChanged(e)){
				return ;
		}

		
		//PositionOfSysList((char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath));
		

      Console::WriteLine( "File: {0} {1}", e->FullPath, e->ChangeType );

	  Backupinfile *filetap = new Backupinfile(2,string((char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath)));

	  HANDLE hThread = CreateThread(NULL, 0, BackUpFile, filetap, 0, NULL);
	  
   }

   /*
      ����⵽�ļ�������ʱ���õĺ���
   */
   static void OnRenamed( Object^ /*source*/, RenamedEventArgs^ e )
   {
      // Specify what is done when a file is renamed.
	   if(JumpWatherOnRenamed(e)){
			return ;
	   }
      Console::WriteLine( "File: {0} renamed to {1}", e->OldFullPath, e->FullPath );

	  //Backupinfile filetap(4,(char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath),(char*)(void*)Marshal::StringToHGlobalAnsi(e->OldFullPath));
	  Backupinfile *filetap = new Backupinfile(4,(char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath),(char*)(void*)Marshal::StringToHGlobalAnsi(e->OldFullPath));
	  HANDLE hThread = CreateThread(NULL, 0, BackUpFile, filetap, 0, NULL);

   }

public:
	
   [PermissionSet(SecurityAction::Demand, Name="FullTrust")]
   
   /*
     ����ļ�����Ҫ���������øú������ĳһ�ļ����µ��ļ��䶯
	 in:
		MonitorPath ��Ҫ��ص��ļ�Ŀ¼
   */

   void static monitor()
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

      return ;
   }

   void static SetMonitorPath(String^ Path){
	   MonitorPath = Path;
   }

    static String^ GetMonitorPath(){
		return MonitorPath;
   }
};

int main() {
	
	ptree pt_root;
	//read_Syslist(GetBackUpPath()+NAME_ROOT_SYSLIST_LOG);
	AddFileToSys("D:\\test\\1.txt","BoxTest/folder1",pt_root);
	//Watcher a;
	//a.SetMonitorPath("D:\\");
	
	//Threading::Thread^ oThread = gcnew Threading::Thread (gcnew Threading::ThreadStart( a.monitor));

	//oThread->Start();
	RenameFolderInBoxName(pt_root,"BoxTest","Box1");


	//outchange();
}
