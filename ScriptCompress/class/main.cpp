#include "main.h"
#include <io.h>
#include <iostream>
#include <cstring>
#include <map>
#include "ScriptPack.h"
#include <direct.h>
using namespace std;



void listFiles(const char* dir,string key,std::map<string,string>& fileMap)
{
	 intptr_t handle;
	 _finddata_t findData;
	 handle = _findfirst(string(dir).append("*.*").c_str(), &findData);
	 if (handle == -1)
	 {
		 cout << "Failed to find first file!\n";
		 return;
	 }

	 do
	 {
		 if (findData.attrib & _A_SUBDIR)
		 {
			 if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
			 {
				 // 是否是子目录并且不为"."或".."
				 //cout << dir << findData.name << "\t<dir>\n";
			 }
			 else
			 {
				 std::string newKey = key;
				 if (strcmp(newKey.c_str(),"") != 0)
				 {
					 newKey.append(".");
				 }
				 newKey.append(findData.name);
				 listFiles(string(dir).append(findData.name).append("/").c_str(),newKey,fileMap);
			 }
		 }
		 else
		 {
			 std::string newKey = key;
			 if (strcmp(newKey.c_str(),"") != 0)
			 {
				 newKey.append(".");
			 }
			 newKey.append(findData.name);
			 //cout << dir <<findData.name << "\t" << findData.size << endl;
			 std::size_t found = newKey.rfind(".lua");
			 std::size_t i = newKey.find("#");
			 if (i == std::string::npos && found != std::string::npos)
			 {
				 newKey.erase(found);
				 //lua file

				 //cout << newKey.c_str() << "\t" << findData.size << endl;
				 fileMap[newKey] = string(dir).append(findData.name);
			 }
		 }

	 } while (_findnext(handle, &findData) == 0);    // 查找目录中的下一个文件

	 _findclose(handle);    // 关闭搜索句柄
}

int main(int argc,char *argv[])
{

	if (1 == argc)
	{
		cout << "example :: package one dir" << endl;
		cout << "ScriptCompress.exe -p D:/ScriptCompress/ScriptCompress/src D:/JD04/v4/src/script" << endl;
		cout << "example :: merge two package" << endl;
		cout << "ScriptCompress.exe -m D:/JD04/v4/src/script D:/JD04/v4/src/script1 D:/JD04/v4/src/script2" << endl;
		return 0;
	}
	char c = *argv[1];
	if (c != '-')
	{
		return 0;
	}
	int op = 0;
	char p = *(argv[1]+1);
	if ((p == 'm') || (p == 'M'))
	{
		if (argc < 4)
		{
			return 0;
		}
		op = 1;
	}
	else if ((p == 'p') || (p == 'P'))
	{
		if (argc < 4)
		{
			return 0;
		}
		op = 2;
	}
	else
	{
		return 0;
	}
	
	if (1 == op)
	{
		intptr_t handle;
		_finddata_t findData;
		handle = _findfirst(argv[2], &findData);
		if (handle == -1)
		{
			return 0;
		}
		 _findclose(handle);
		handle = _findfirst(argv[3], &findData);
		if (handle == -1)
		{
			return 0;
		}
		 _findclose(handle);
		ScriptPack* sp = ScriptPack::create();
		int ret = sp->unCompressFile(argv[2],1);
		if (ret > 0)
		{
			return 0;
		}
		ScriptPack* spt = ScriptPack::create();
		ret = spt->unCompressFile(argv[3],1);
		if (ret > 0)
		{
			return 0;
		}
		ret = spt->mergeAndDelete(sp,1);
		if (ret > 0)
		{
			return 0;
		}

		if (argv[4])
		{
			ret = spt->writeToFile(argv[4]);
		}
		else
		{
			ret = spt->writeToFile(argv[3]);
		}
		if (ret > 0)
		{
			return 0;
		}
		ScriptPack::dispose(sp);
		ScriptPack::dispose(spt);
		cout << argv[2] << " merge to " << argv[3];
		if (argv[4])
		{
			cout << " write in" << argv[4] << endl;
		}
		else
		{
			cout << " write in" << argv[3] << endl;
		}
	}
	else if (2 == op)
	{
		intptr_t handle;
		_finddata_t findData;
		handle = _findfirst(argv[2], &findData);
		if (handle == -1)
		{
			return 0;
		}

		std::map<std::string,std::string> fileMap;
		listFiles(string(argv[2]).append("/").c_str(),"",fileMap);
		ScriptPack* sp = ScriptPack::create();
		int file_count = 0;
		for (auto iter = fileMap.begin();iter != fileMap.end();iter++)
		{
			int ret = sp->addFile(iter->second.c_str(),iter->first,ScriptPack::Op::eWriteWhenExist);
			if (ret == 0)
			{
				file_count ++;
				cout << iter->first << endl;
			}
		}
		sp->writeToFile(argv[3]);
		ScriptPack::dispose(sp);
		cout << "pack done!, write in " <<  argv[3] << " file count:" << file_count << endl;
	}
	return 0;
}
