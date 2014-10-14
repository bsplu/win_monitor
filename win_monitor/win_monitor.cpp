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



//Function & class 定义

/*
  该类主要是将文件信息整合当到一个地方
*/
class SysFile{

public:
	//string BoxName;//box名称
	string FullPathInDisc;//文件在磁盘的路径
	string FullPathInBox;//box中的虚拟文件夹路径,构成  BoxName/虚拟文件夹名/文件名
	int BranchNumber;
	int threadhandle;//储存当前调用该文件的线程号，如果没有则为0

	SysFile(){

		FullPathInDisc = "";
		FullPathInBox = "";
		threadhandle = 0;
		BranchNumber = 0;
	}

	//默认文件储存在box的根目录下
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
  该结构主要是传入backupfile函数使用
  //tap 为文件更改类型 created 1 changed 2 deleted 3 renamed 4
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
//同步列表
int len_SysFileList = 2;
SysFile *SysFileList = new SysFile [len_SysFileList];
int numSaved_SysFileList = 0;

ptree PTROOT_files;

/*
   同步列表扩容
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
  将File_in添加至同步列表中
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
   将Syslist写入FullPathToStoreSyslist文件中
   in:
	Syslist要储存的文件路径
   out:(none)
   return:
	1为成功
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
  将syslist从文件中读入
  in:
	FullPathToStoreSyslist 储存syslist的文件全路径
  out:(none)
  return:
	1为成功
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
	char * paths2c = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(e->FullPath);
	string filepath = paths2c;
	Marshal::FreeHGlobal(IntPtr(paths2c));

	if ((lf = _findfirst( filepath.c_str(), &file)) == -1l) {
		cout << "该文件或目录不存在！\n";

		return -1;
	}else{
		return file.attrib ;
	}
	_findclose(lf);

}

/*
   返回FileNameFullPathInDisc在同步列表中的位置(返回值从1开始)
   ==========================================================
   in:
	FileNameFullPath 要查找的文件在本地磁盘的全地址
	pstart 为从同步列表的第几位开始
   ----------------------------------------------------------
   out:
	在SysList中的位置,在找到第一个时停止
	如果文件不在同步列表则返回-1
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
	返回folder在同步列表中第一次出现的位置
	in:
		folderfullPath folder在磁盘中的全地址
		start 从syslist中第几位开始查找，默认从第一位查找
	out:(none)
	return:
	-1 为没有找到
	>0 为找到的第一个位置
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
	将所有box的xml更新
	in:
		pt_root 根树
	out:(none)
	return:
		1 成功
		-1 失败,没有boxs分支
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
	将SysFileList中磁盘地址中OldPath替换为NewPath
	检测到磁盘地址更改时要使用该函数
	in:
		OldPath 文件或文件夹旧的全地址
		NewPath 文件或文件夹新的全地址
	out:(none)
	return:
		1成功
*/
int RenamedFilePathInDisc(string OldPath,string NewPath){
	//如果结尾带"\\",去掉，这样文件renamed也适合
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
   得到Box当前所在分支的branchNumber
   in:
	BoxName 要查找box的名字
   out:(none)
   return:
	branchnumber
	-1 没有找到box

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
   添加同步文件（包括将文件添加至同步列表，将文件添加至box中）
   in:
	fileFullPathInDisc 要添加文件的全路径
	ToFullPathInBox 要添加的box路径,不包含文件名
	BranchNumber 为要添加到box的分支号,默认为添加到主分支
   out:
	pt_root 将添加后的文件tree返回
   return:
	1 为添加成功
	-1 0 为失败
	-2 为box存在相同名字的文件
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

	//检查文件是否已在同步列表
	int posInSysList = PositionOfSysList(fileFullPathInDisc);

	int existInBox = filetree::AddFilePathToRoot(pt_root,FullPathInBox);
	if(existInBox == -1){
		return existInBox;
	}

	if(posInSysList > 0 && existInBox == -2){
		//说明既存在列表又存在box中，可以return
		return 1;
	}else if(posInSysList > 0 && existInBox == 1){
		//说明已经在别的box中添加
		//添加box中的关联
		string ExistedBoxPath = SysFileList[posInSysList-1].FullPathInBox;
		filetree::AddFilePathToRoot(pt_root,FullPathInBox,ExistedBoxPath);

		UpdateAllBoxxml(pt_root);

		filetree::write_xml_CHECKPATH(GetBackUpPath()+"ptreeroot.xml",pt_root);

	}else if(posInSysList <0 && existInBox == -2){
		//说明要把文件挂载到已有的文件节点上，需要比较已有文件是否相同，不同则会建立分支
		//当前不支持该功能
		return -1;
	}else if(posInSysList <0 && existInBox == 1){
		//完全没找到

		//将文件添加至同步列表
		SysFile File_in(fileFullPathInDisc,FullPathInBox,BranchNumber);
		PutSysFileInSysList(File_in);

			//更新syslist.log
		write_Syslist(GetBackUpPath()+NAME_ROOT_SYSLIST_LOG);
	
		stringstream sBranchNumber;
		sBranchNumber << BranchNumber;
		filetree::write_box_xml(pt_root,BoxName,GetBackUpPath()+"boxs\\"+BoxName+"\\ptreebox.xml");

		//写入branchnumber
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
	

		//备份
		Backupinfile *filetap = new Backupinfile(1,fileFullPathInDisc);
		HANDLE hThread = CreateThread(NULL, 0, BackUpFile, filetap, 0, NULL);
		WaitForSingleObject(hThread, INFINITE);  
		CloseHandle(hThread);  

	}
	return 1;
}

/*
	改变box中的虚拟文件夹名称，或更改box名称
	in:
		pt_root 根box
		OldPath 原文件夹的全路径 
		NewPath 新文件夹的全路径
	out:
		pt_root 根box
	return:
		-1 失败，OldPath NewPath更改不止一处
		1成功
*/
int RenamedFolderPathInRootWUpxml(ptree& pt_root,string OldPath,string NewPath){

	if(OldPath == "" || NewPath == ""){
		return 1;
	}

	//判断最后一位是否为'/'
	if(OldPath.find_last_of("/") == (OldPath.length()-1)){
		OldPath.erase(OldPath.length()-1);
	}else{

	}

	if(NewPath.find_last_of("/") == (NewPath.length()-1)){
		NewPath.erase(NewPath.length()-1);
	}else{

	}

	//判断一下是否只有最后一个文件名字不同
	if(OldPath.substr(0,OldPath.find_last_of("/")>(OldPath.length()-unsigned(1))?
		0:OldPath.find_last_of("/")) != 
		NewPath.substr(0,OldPath.find_last_of("/")>(NewPath.length()-unsigned(1))?
		0:NewPath.find_last_of("/"))){
		return -1;
	}


	if(GetBoxNameFromFullPathInBox(OldPath) == OldPath){
		//更改boxname
		system(
			("rename " + GetBackUpPath() + GetBoxNameFromFullPathInBox(OldPath) + " " 
			+ GetBoxNameFromFullPathInBox(NewPath)).c_str());
	}
	//更改同步列表
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
	改变box中的虚拟文件名称
	in:
		pt_root 根box
		OldPath 原文件的全路径 
		NewPath 新文件的全路径
	out:
		pt_root 根box
	return:
		-1 失败，OldPath NewPath更改不止一处
		1成功
*/
int RenamedFilePathInRootWUpxml(ptree& pt_root,string OldPath,string NewPath){

	
	if(OldPath == "" || NewPath == ""){
		return 1;
	}

	//判断最后一位是否为'/'
	if(OldPath.find_last_of("/") == (OldPath.length()-1)){
		return -1;
	}else{

	}

	if(NewPath.find_last_of("/") == (NewPath.length()-1)){
		return -1;
	}else{

	}

	//判断一下是否只有最后一个文件名字不同
	if(OldPath.substr(0,OldPath.find_last_of("/")>(OldPath.length()-unsigned(1))?
		0:OldPath.find_last_of("/")) != 
		NewPath.substr(0,OldPath.find_last_of("/")>(NewPath.length()-unsigned(1))?
		0:NewPath.find_last_of("/"))){
		return -1;
	}

	//更改同步列表
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

//获得用户名
string Getc4rUserName(){
	string username="";

	//NOT
	username = "bsplu";

	return username;
}

//得到备份路径
string GetBackUpPath(){
	string backuppath = "";

	//NOT
	backuppath = "E:\\testbackupfolder\\";

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

		tarfilename = ( Getc4rUserName() + "_" + stime.str() + ".zip").c_str();

		return tarfilename;
}

/*
	将储存文件更改的xml文档读入
	in:
		BoxName 要读文档所属的boxname
		pt_files 储存文档的树
		BranchNumber 分支号，默认为box当前分支号
	out:
		pt_files 返回储存文档的树
	return:
		1 成功
		-1 失败
*/
int read_filelog_xml(string BoxName,ptree& pt_files,int BranchNumber = 0){

	if(BranchNumber == 0)
		BranchNumber = GetPresentBoxBranchNumber(BoxName);

	stringstream sBranchnumber;
	sBranchnumber<<BranchNumber;

	return filetree::read_xml_CHECKPATH(GetBackUpPath() +"boxs\\" + BoxName + "\\filechange_" + sBranchnumber.str() +".xml" , pt_files);

}

/*
	储存文件更改信息
	in:
		BoxName 要读文档所属的boxname
		pt_files 储存文档的树
		BranchNumber 分支号，默认为box当前分支号
	return:
		1 成功
		-1 失败
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
	将备份信息加入到备份树节点，一般先调用read_filelog_xml，该函数后会调用write_filelog_xml
	in:
		pt_Box 备份树的box节点
		FullPathInBox 备份文件在树中的全地址
		TarFileName 打包的名字 
					格式是:UserName_BackUpnumber.zip 
					UserName请替换为当前用户名字 BackUpnumber请替换为备份号
		ChangeTap 为改动类型
			1 | 添加文件
			2 | 文件变动
			3 | 文件删除
			4 | 文件重命名
			5 | commit
	out:
		pt_Box 为更改后的备份树

	return:
		1 成功
		-1 失败,主要可能文件不存在，或changtap不符合
	
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
	将备份信息存储到所有相关联的备份tree中
*/
int AddChangeMesageToAllPaths(ptree & pt_root,string FullPathInBoxChanged,string TarFileName,int ChangeTap ){

	ptree ptfile;//指向备份树
	if(read_filelog_xml(GetBoxNameFromFullPathInBox(FullPathInBoxChanged),ptfile) == -1){

	}
	AddChangeMesageToPtreeFile(ptfile,FullPathInBoxChanged,TarFileName,ChangeTap,true);
	//首先找到FullPathInBoxChanged指向的文件
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
   调用该函数可以备份（压缩）在监控列表中的指定文件
   in:
	lpParam Backupinfile
   out:
	0 不是备份文件
	-1 备份失败
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

		//调用压缩备份函数
		cout<<"备份"<<endl;
		system(("makecab /d compressiontype=mszip " + FileFullPathInDisc + " " + GetBackUpPath() + backupfilename + " >nul 2>nul" ).c_str());
		

	}else{
		return 0;
	}

		//找到box的备份文档，将更改信息添加进去
		
		position = PositionOfSysList(FileFullPathInDisc,position);
		string FullPathInBox = SysFileList[position-1].FullPathInBox;
		cout<<FullPathInBox<<endl;
		cout<<AddChangeMesageToAllPaths(PTROOT_files,FullPathInBox,backupfilename,(*filetap).tap)<<endl;


	delete filetap;
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
  当检测到文件发生改变时，在一些情况，需要跳过不做出反应
  in:
	e 为动作句柄
  return:
	true 需要跳过
	false 不跳过
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
  当检测到文件被重命名时，在一些情况，需要跳过不做出反应
  in:
	e 为动作句柄
  return:
	true 需要跳过
	false 不跳过

	*注意:像office的文档文件，当保存时，触发的不是changed而是renamed
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
   Watcher类，负责监控文件目录，并根据目录下文件不同反应做出相应改变

*/
public ref class Watcher
{
private:
	[ThreadStatic] static String^  MonitorPath; //监视路径

   /*
      当检测到文件创建改变时调用的函数
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
      当检测到文件被删除时调用的函数
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
      当检测到文件发生改变时调用的函数
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
      当检测到文件重命名时调用的函数
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
     监控文件的主要函数，调用该函数监控某一文件夹下的文件变动
	 in:
		MonitorPath 需要监控的文件目录
   */

   void monitor(Object^ Pathin)
   {


      // Create a new FileSystemWatcher and set its properties.
      FileSystemWatcher^ watcher = gcnew FileSystemWatcher;

	  //对路径的设定
	  MonitorPath = (String^)Pathin;
      watcher->Path = MonitorPath;

	  watcher->InternalBufferSize = 1048576;

      /* Watch for changes in LastAccess and LastWrite times, and 
          the renaming of files or directories. */
      watcher->NotifyFilter = static_cast<NotifyFilters>(//NotifyFilters::LastAccess |
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
     // Console::WriteLine( "Press \'q\' to quit the sample." );
	  
	  
	while ( !CheckSleep){
		Sleep(1000);  
	  };//当检测的文件或文件夹被删除时也应结束掉任务
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

