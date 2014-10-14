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


const string NAME_ROOT_SYSLIST_LOG = "RootSyslist.log";



//Function Name

int ScreenFileType(FileSystemEventArgs^ e);
int PositionOfSysList(string FileNameFullPathInDisc,int pstart);
DWORD WINAPI BackUpFile(LPVOID lpParam);
string GetTarFileName();
string Getc4rUserName();//NOT
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
	//string BoxName;//box����
	string FullPathInDisc;//�ļ��ڴ��̵�·��
	string FullPathInBox;//box�е������ļ���·��,����  BoxName/�����ļ�����/�ļ���
	int BranchNumber;
	int threadhandle;//���浱ǰ���ø��ļ����̺߳ţ����û����Ϊ0

	SysFile(){

		FullPathInDisc = "";
		FullPathInBox = "";
		threadhandle = 0;
		BranchNumber = 0;
	}

	//Ĭ���ļ�������box�ĸ�Ŀ¼��
	SysFile(string FullPathInDisc_input,string FullPathInBox_in){

		FullPathInDisc = FullPathInDisc_input;
		FullPathInBox = FullPathInBox_in;
		BranchNumber = 0;
		threadhandle = 0;
	}

	SysFile(string FullPathInDisc_input,string FullPathInBox_in,int BranchNumber_in){

		FullPathInDisc = FullPathInDisc_input;
		FullPathInBox = FullPathInBox_in;
		BranchNumber = BranchNumber_in;
		threadhandle = 0;

	}

	SysFile operator=(SysFile file_in){
		this->FullPathInDisc = file_in.FullPathInDisc;
		this->FullPathInBox = file_in.FullPathInBox;
		this->BranchNumber = file_in.BranchNumber;

		return *this;
	}

	void addSysFile(string FullPathInDisc_input,string FullPathInBox_in,int BranchNumber_in){
		FullPathInDisc = FullPathInDisc_input;
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
int len_SysFileList = 2;
SysFile *SysFileList = new SysFile [len_SysFileList];
int numSaved_SysFileList = 0;

ptree PTROOT_files;

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
	char * paths2c = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(e->FullPath);
	string filepath = paths2c;
	Marshal::FreeHGlobal(IntPtr(paths2c));

	if ((lf = _findfirst( filepath.c_str(), &file)) == -1l) {
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

/*
	������box��xml����
	in:
		pt_root ����
	out:(none)
	return:
		1 �ɹ�
		-1 ʧ��,û��boxs��֧
*/
int UpdateAllBoxxml(ptree pt_root){

	try{
		ptree BoxsNode = pt_root.get_child("root.boxs");
		
		for(auto it=BoxsNode.begin();it != BoxsNode.end();++it){
				ptree BoxNode;
				BoxNode.add_child("box",it->second);
				filetree::write_xml_CHECKPATH(GetBackUpPath() + it->second.get<string>("name",""),BoxNode);
		}
	}
	catch(...){
		return -1;
	}	

	return 1;

}

/*
	��SysFileList�д��̵�ַ��OldPath�滻ΪNewPath
	��⵽���̵�ַ����ʱҪʹ�øú���
	in:
		OldPath �ļ����ļ��оɵ�ȫ��ַ
		NewPath �ļ����ļ����µ�ȫ��ַ
	out:(none)
	return:
		1�ɹ�
*/
int RenamedFilePathInDisc(string OldPath,string NewPath){
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
	
	for(int i=0;i<numSaved_SysFileList;i++){
		if(GetBoxNameFromFullPathInBox( SysFileList[i].FullPathInBox) == BoxName){
			return SysFileList[i].BranchNumber;
		}
	}
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

	//����ļ��Ƿ�����ͬ���б�
	int posInSysList = PositionOfSysList(fileFullPathInDisc);

	int existInBox = filetree::AddFilePathToRoot(pt_root,FullPathInBox);
	if(existInBox == -1){
		return existInBox;
	}

	if(posInSysList > 0 && existInBox == -2){
		//˵���ȴ����б��ִ���box�У�����return
		return 1;
	}else if(posInSysList > 0 && existInBox == 1){
		//˵���Ѿ��ڱ��box�����
		//���box�еĹ���
		string ExistedBoxPath = SysFileList[posInSysList-1].FullPathInBox;
		filetree::AddFilePathToRoot(pt_root,FullPathInBox,ExistedBoxPath);

		UpdateAllBoxxml(pt_root);

		filetree::write_xml_CHECKPATH(GetBackUpPath()+"ptreeroot.xml",pt_root);

	}else if(posInSysList <0 && existInBox == -2){
		//˵��Ҫ���ļ����ص����е��ļ��ڵ��ϣ���Ҫ�Ƚ������ļ��Ƿ���ͬ����ͬ��Ὠ����֧
		//��ǰ��֧�ָù���
		return -1;
	}else if(posInSysList <0 && existInBox == 1){
		//��ȫû�ҵ�

		//���ļ������ͬ���б�
		SysFile File_in(fileFullPathInDisc,FullPathInBox,BranchNumber);
		PutSysFileInSysList(File_in);

			//����syslist.log
		write_Syslist(GetBackUpPath()+NAME_ROOT_SYSLIST_LOG);
	
		stringstream sBranchNumber;
		sBranchNumber << BranchNumber;
		filetree::write_box_xml(pt_root,BoxName,GetBackUpPath()+"boxs\\"+BoxName+"\\ptreebox.xml");

		//д��branchnumber
		fstream fout(GetBackUpPath()+"boxs\\"+BoxName+"\\branchnumber.txt",ios::app);
		int inbranchnumber = -1;
		while(fout){
			fout>>inbranchnumber;
			if(fout.fail()){
				break;
			}else if(inbranchnumber == BranchNumber){
				break;
			}
		}
		fout.clear();
		if(inbranchnumber != BranchNumber)
			fout<<BranchNumber<<endl;

		fout.close();

		filetree::write_xml_CHECKPATH(GetBackUpPath()+"ptreeroot.xml",pt_root);
	

		//����
		Backupinfile *filetap = new Backupinfile(1,fileFullPathInDisc);
		HANDLE hThread = CreateThread(NULL, 0, BackUpFile, filetap, 0, NULL);
		WaitForSingleObject(hThread, INFINITE);  
		CloseHandle(hThread);  

	}
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
int RenamedFolderPathInRootWUpxml(ptree& pt_root,string OldPath,string NewPath){

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


	if(GetBoxNameFromFullPathInBox(OldPath) == OldPath){
		//����boxname
		system(
			("rename " + GetBackUpPath() + GetBoxNameFromFullPathInBox(OldPath) + " " 
			+ GetBoxNameFromFullPathInBox(NewPath)).c_str());
	}
	//����ͬ���б�
	for(int i=0;i<numSaved_SysFileList;i++){
		string str = SysFileList[i].FullPathInBox;
		if(str.find(OldPath) < str.length()-unsigned(1)){
			SysFileList[i].FullPathInBox = NewPath+str.substr(OldPath.length());
		}
	}

	if(filetree::RenamedFolderPathInRoot(pt_root,OldPath,NewPath) == -1){
		return -1;
	}

	UpdateAllBoxxml(pt_root);
	write_Syslist(GetBackUpPath()+NAME_ROOT_SYSLIST_LOG);
	filetree::write_xml_CHECKPATH(GetBackUpPath()+"ptreeroot.xml",pt_root);

	return 1;

}

/*
	�ı�box�е������ļ�����
	in:
		pt_root ��box
		OldPath ԭ�ļ���ȫ·�� 
		NewPath ���ļ���ȫ·��
	out:
		pt_root ��box
	return:
		-1 ʧ�ܣ�OldPath NewPath���Ĳ�ֹһ��
		1�ɹ�
*/
int RenamedFilePathInRootWUpxml(ptree& pt_root,string OldPath,string NewPath){

	
	if(OldPath == "" || NewPath == ""){
		return 1;
	}

	//�ж����һλ�Ƿ�Ϊ'/'
	if(OldPath.find_last_of("/") == (OldPath.length()-1)){
		return -1;
	}else{

	}

	if(NewPath.find_last_of("/") == (NewPath.length()-1)){
		return -1;
	}else{

	}

	//�ж�һ���Ƿ�ֻ�����һ���ļ����ֲ�ͬ
	if(OldPath.substr(0,OldPath.find_last_of("/")>(OldPath.length()-unsigned(1))?
		0:OldPath.find_last_of("/")) != 
		NewPath.substr(0,OldPath.find_last_of("/")>(NewPath.length()-unsigned(1))?
		0:NewPath.find_last_of("/"))){
		return -1;
	}

	//����ͬ���б�
	for(int i=0;i<numSaved_SysFileList;i++){
		string str = SysFileList[i].FullPathInBox;
		if(str.find(OldPath) < str.length()-unsigned(1)){
			SysFileList[i].FullPathInBox = NewPath+str.substr(OldPath.length());
		}
	}




	if(filetree::RenamedFilePathInRoot(pt_root,OldPath,NewPath) == -1){
		return -1;
	}

	UpdateAllBoxxml(pt_root);

	write_Syslist(GetBackUpPath()+NAME_ROOT_SYSLIST_LOG);
	filetree::write_xml_CHECKPATH(GetBackUpPath()+"ptreeroot.xml",pt_root);
	
	return 1;

}

//����û���
string Getc4rUserName(){
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

		tarfilename = ( Getc4rUserName() + "_" + stime.str() + ".zip").c_str();

		return tarfilename;
}

/*
	�������ļ����ĵ�xml�ĵ�����
	in:
		BoxName Ҫ���ĵ�������boxname
		pt_files �����ĵ�����
		BranchNumber ��֧�ţ�Ĭ��Ϊbox��ǰ��֧��
	out:
		pt_files ���ش����ĵ�����
	return:
		1 �ɹ�
		-1 ʧ��
*/
int read_filelog_xml(string BoxName,ptree& pt_files,int BranchNumber = 0){

	if(BranchNumber == 0)
		BranchNumber = GetPresentBoxBranchNumber(BoxName);

	stringstream sBranchnumber;
	sBranchnumber<<BranchNumber;

	return filetree::read_xml_CHECKPATH(GetBackUpPath() +"boxs\\" + BoxName + "\\filechange_" + sBranchnumber.str() +".xml" , pt_files);

}

/*
	�����ļ�������Ϣ
	in:
		BoxName Ҫ���ĵ�������boxname
		pt_files �����ĵ�����
		BranchNumber ��֧�ţ�Ĭ��Ϊbox��ǰ��֧��
	return:
		1 �ɹ�
		-1 ʧ��
*/
int write_filelog_xml(string BoxName,ptree pt_Box,int BranchNumber = 0){

	if(BranchNumber == 0)
		BranchNumber = GetPresentBoxBranchNumber(BoxName);

	stringstream sBranchnumber;
	sBranchnumber<<BranchNumber;

	pt_Box.put("root.lasteditor",Getc4rUserName());
	pt_Box.put("root.lastedittime",time(NULL));

	return filetree::write_xml_CHECKPATH(GetBackUpPath() + "boxs\\" + BoxName + "\\filechange_" + sBranchnumber.str() +".xml"  , pt_Box);
}

/*
	��������Ϣ���뵽�������ڵ㣬һ���ȵ���read_filelog_xml���ú���������write_filelog_xml
	in:
		pt_Box ��������box�ڵ�
		FullPathInBox �����ļ������е�ȫ��ַ
		TarFileName ��������� 
					��ʽ��:UserName_BackUpnumber.zip 
					UserName���滻Ϊ��ǰ�û����� BackUpnumber���滻Ϊ���ݺ�
		ChangeTap Ϊ�Ķ�����
			1 | ����ļ�
			2 | �ļ��䶯
			3 | �ļ�ɾ��
			4 | �ļ�������
			5 | commit
	out:
		pt_Box Ϊ���ĺ�ı�����

	return:
		1 �ɹ�
		-1 ʧ��,��Ҫ�����ļ������ڣ���changtap������
	
*/
int AddChangeMesageToPtreeFile(ptree & pt_Box,string FullPathInBox,string TarFileName,
							   int ChangeTap,bool writePtree = false){

	string strchangtap;
	switch (ChangeTap)
	{
	case 1:
		strchangtap = "Add";
		break;
	case 2:
		strchangtap = "Chg";
		break;
	case 3:
		strchangtap = "Del";
		break;
	case 4:
		strchangtap = "Rnm";
		break;
	case 5:
		strchangtap = "Cmm";//commit
		break;
	default:
		return -1;
		break;
	}


	try{

		ptree& pt_files = pt_Box.get_child("root.files");

		for(auto it=pt_files.begin();it!=pt_files.end();++it){

				if(it->second.get<string>("filepath","") == FullPathInBox){
					it->second.add("changes.change",strchangtap + "_" + TarFileName);

					if(writePtree){

						write_filelog_xml(GetBoxNameFromFullPathInBox(FullPathInBox),pt_Box);
					}
					return 1;
				}
		}
	}
	catch(...){

		if(strchangtap == "Add"){
			pt_Box.add("root.creator",Getc4rUserName());
			pt_Box.add("root.creattime",time(NULL));
			pt_Box.add("root.lasteditor",Getc4rUserName());
			pt_Box.add("root.lastedittime",time(NULL));
			ptree pttemp;

			pttemp.put("filepath",FullPathInBox);
			pt_Box.add_child("root.files.file",pttemp);
			ptree &pt_files = pt_Box.get_child("root.files");



			for(auto it=pt_files.begin();it!=pt_files.end();++it){

					if(it->second.get<string>("filepath","") == FullPathInBox){
						it->second.add("changes.change",strchangtap + "_" + TarFileName);

						if(writePtree){

							write_filelog_xml(GetBoxNameFromFullPathInBox(FullPathInBox),pt_Box);
						}
						return 1;
					}
			}
		}else{
			return -1;
		}
	}



	return -1;
}


/*
	��������Ϣ�洢������������ı���tree��
*/
int AddChangeMesageToAllPaths(ptree & pt_root,string FullPathInBoxChanged,string TarFileName,int ChangeTap ){

	ptree ptfile;//ָ�򱸷���
	if(read_filelog_xml(GetBoxNameFromFullPathInBox(FullPathInBoxChanged),ptfile) == -1){

	}
	AddChangeMesageToPtreeFile(ptfile,FullPathInBoxChanged,TarFileName,ChangeTap,true);
	//�����ҵ�FullPathInBoxChangedָ����ļ�
	ptree* FileNode;
	if(filetree::FindPtreeAdressByFullPath(pt_root,FullPathInBoxChanged,FileNode) == -1){
		return 1;
	}
	try{
		for(auto it=(*FileNode).get_child("otherpaths").begin();it != (*FileNode).get_child("otherpaths").end();++it){
			string otherpath = it->second.get_value<string>();
			ptfile.clear();
			read_filelog_xml(GetBoxNameFromFullPathInBox(FullPathInBoxChanged),ptfile);
			AddChangeMesageToPtreeFile(ptfile,FullPathInBoxChanged,TarFileName,ChangeTap,true);
		}
	}
	catch(...){

	}

	return 1;
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

	SysFileList[position-1].threadhandle = GetCurrentThreadId();
 

	if( position>0 ){

		Sleep(1000);
		if(SysFileList[position-1].threadhandle != GetCurrentThreadId())
			return 0;

		//����ѹ�����ݺ���
		cout<<"����"<<endl;
		system(("makecab /d compressiontype=mszip " + FileFullPathInDisc + " " + GetBackUpPath() + backupfilename + " >nul 2>nul" ).c_str());
		

	}else{
		return 0;
	}

		//�ҵ�box�ı����ĵ�����������Ϣ��ӽ�ȥ
		
		position = PositionOfSysList(FileFullPathInDisc,position);
		string FullPathInBox = SysFileList[position-1].FullPathInBox;
		cout<<FullPathInBox<<endl;
		cout<<AddChangeMesageToAllPaths(PTROOT_files,FullPathInBox,backupfilename,(*filetap).tap)<<endl;


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
	char * FileNames2c = (char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath);
	string FileName = GetFileNameFromFullPath(FileNames2c);

	Marshal::FreeHGlobal(IntPtr(FileNames2c));
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
	char * FileNames2c = (char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath);
	string FileName = GetFileNameFromFullPath(FileNames2c);

	Marshal::FreeHGlobal(IntPtr(FileNames2c));

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
	char * FileNames2c = (char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath);
	string FileName = GetFileNameFromFullPath(FileNames2c);

	Marshal::FreeHGlobal(IntPtr(FileNames2c));

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

	char * FileNames2c = (char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath);
	string FileNameNew = GetFileNameFromFullPath(FileNames2c);
	Marshal::FreeHGlobal(IntPtr(FileNames2c));
	FileNames2c = (char*)(void*)Marshal::StringToHGlobalAnsi(e->OldFullPath);
	string FileNameOld = GetFileNameFromFullPath(FileNames2c);
	Marshal::FreeHGlobal(IntPtr(FileNames2c));


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
	[ThreadStatic] static String^  MonitorPath; //����·��

   /*
      ����⵽�ļ������ı�ʱ���õĺ���
   */
   static void OnCreated( Object^ /*source*/, FileSystemEventArgs^ e )
   {
      // Specify what is done when a file is created.
	   /*
		if(JumpWatherOnCreated(e)){
				return ;
		}
		*/
      Console::WriteLine( "File: {0} {1}", e->FullPath, e->ChangeType );
	  
   }

   /*
      ����⵽�ļ���ɾ��ʱ���õĺ���
   */
     static void OnDeleted( Object^ /*source*/, FileSystemEventArgs^ e )
   {
      // Specify what is done when a file is deleted.
	   /*
	   if(JumpWatherOndeleted(e)){
				return ;
		}
		*/
		Console::WriteLine( "File: {0} {1}", e->FullPath, e->ChangeType );
	  
   }

   /*
      ����⵽�ļ������ı�ʱ���õĺ���
   */
      static void OnChanged( Object^ /*source*/, FileSystemEventArgs^ e )
   {
      // Specify what is done when a file is deleted.
	   /*
	   if(JumpWatherOnChanged(e)){
				return ;
		}
		*/

		
		//PositionOfSysList((char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath));
		

      Console::WriteLine( "File: {0} {1}", e->FullPath, e->ChangeType );


	  //debug
	  //Backupinfile *filetap = new Backupinfile(2,string((char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath)));

	  //HANDLE hThread = CreateThread(NULL, 0, BackUpFile, filetap, 0, NULL);
	  
	  //BackUpFile(filetap);
   }

   /*
      ����⵽�ļ�������ʱ���õĺ���
   */
   static void OnRenamed( Object^ /*source*/, RenamedEventArgs^ e )
   {
      // Specify what is done when a file is renamed.
	   /*
	   if(JumpWatherOnRenamed(e)){
			return ;
	   }
	   */
      Console::WriteLine( "File: {0} renamed to {1}", e->OldFullPath, e->FullPath );

	  /*debug
	  Backupinfile *filetap = new Backupinfile(4,(char*)(void*)Marshal::StringToHGlobalAnsi(e->FullPath),(char*)(void*)Marshal::StringToHGlobalAnsi(e->OldFullPath));
	  HANDLE hThread = CreateThread(NULL, 0, BackUpFile, filetap, 0, NULL);
	  */

   }

public:
	static bool CheckSleep = false;
   [PermissionSet(SecurityAction::Demand, Name="FullTrust")]
   
   /*
     ����ļ�����Ҫ���������øú������ĳһ�ļ����µ��ļ��䶯
	 in:
		MonitorPath ��Ҫ��ص��ļ�Ŀ¼
   */

   void monitor(Object^ Pathin)
   {


      // Create a new FileSystemWatcher and set its properties.
      FileSystemWatcher^ watcher = gcnew FileSystemWatcher;

	  //��·�����趨
	  MonitorPath = (String^)Pathin;
      watcher->Path = MonitorPath;

	  watcher->InternalBufferSize = 1048576;

      /* Watch for changes in LastAccess and LastWrite times, and 
          the renaming of files or directories. */
      watcher->NotifyFilter = static_cast<NotifyFilters>(//NotifyFilters::LastAccess |
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
     // Console::WriteLine( "Press \'q\' to quit the sample." );
	  
	  
	while ( !CheckSleep){
		Sleep(1000);  
	  };//�������ļ����ļ��б�ɾ��ʱҲӦ����������
	//while(1);
      return ;
   }

   void  SetMonitorPath(String^ Path){
	   MonitorPath = Path;
   }

    String^ GetMonitorPath(){
		return MonitorPath;
   }
};


int main() {

	read_Syslist(GetBackUpPath()+NAME_ROOT_SYSLIST_LOG);
	AddFileToSys("D:\\test\\4.txt","Box/folde",PTROOT_files);
	Watcher^ a = gcnew Watcher;


	Threading::Thread^ oThread1 = gcnew Threading::Thread (gcnew Threading:: ParameterizedThreadStart( a,&Watcher::monitor));
	//Threading::Thread^ oThread2= gcnew Threading::Thread (gcnew Threading:: ParameterizedThreadStart( a,&Watcher::monitor));
	oThread1->Start("D:\\");
	//oThread2->Start("D:\\");
	//hello
	return 1;
	//outchange();
}

