#ifndef _SCRIPT_PACK_H_
#define _SCRIPT_PACK_H_
#pragma  comment(lib,"libzlib.lib")
#include "headers.h"

class ScriptPack
{

	struct Pack
	{
		uint64 contentLength;
		unsigned char* content;
	};
public:
	enum Error
	{
		eSuccess,
		eGetPathError,
		eReadFileError,
		eFileExistError,
		eKeyError,
		eNoContentSize,
		eNoLuaEngine,
		eNoLuaStack,
		eNoLuaState,
	};
	enum Op
	{
		eIgnoreWhenExist,
		eWriteWhenExist,
	};

	static ScriptPack* create();
	static ScriptPack* create(const char* fileName);
	static void dispose(ScriptPack* sp);
	int8 mergeAndDelete(ScriptPack* sp,int op); //添加后将scriptPack内容删除
	int8 addFile(const char* file,const std::string& key,int op);
	int8 writeToFile(const char* file);
	int8 decode(const unsigned char* sBuffData,int op);
	int8 unCompressFile(const char* file,int op);
private:
	std::map<std::string,Pack>& getFileMap();
	ScriptPack(void){_fileMap.clear();};
	~ScriptPack(void){};
	std::map<std::string,Pack> _fileMap;
};
#endif
