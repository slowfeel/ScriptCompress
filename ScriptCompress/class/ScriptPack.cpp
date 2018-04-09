#include "ScriptPack.h"
#include <fstream>
#include "zconf.h"
#include "zlib.h"

#define SCRIPE_BUFF_LENGTH 10240000
ScriptPack* ScriptPack::create()
{
	return new ScriptPack();
}
ScriptPack* ScriptPack::create(const char* fileName)
{
	ScriptPack* sp = new ScriptPack();
	sp->unCompressFile(fileName,Op::eWriteWhenExist);
	return sp;
}

void ScriptPack::dispose(ScriptPack* sp)
{
	std::map<std::string,Pack>& srcMap = sp->getFileMap();
	for (auto iter = srcMap.begin();iter != srcMap.end();iter++)
	{
		delete iter->second.content;
	}
	srcMap.clear();
	delete sp;
}
int8 ScriptPack::mergeAndDelete(ScriptPack* sp,int op)
{
	std::map<std::string,Pack>& srcMap = sp->getFileMap();
	for (auto iter = srcMap.begin();iter != srcMap.end();iter++)
	{
		auto iterDst = _fileMap.find(iter->first);
		if (iterDst != _fileMap.end())
		{
			if (Op::eWriteWhenExist == op)
			{
				Pack pack = iter->second;
				delete iterDst->second.content;
				iterDst->second.contentLength = iter->second.contentLength;
				iterDst->second.content = iter->second.content;
			}
		}
		else
		{
			_fileMap[iter->first] = iter->second;
		}
	}
	srcMap.clear();
	return Error::eSuccess;
}

int8 ScriptPack::addFile(const char* file,const std::string& key,int op)
{

	std::ifstream i(file,std::ios::binary);
	
	long l = i.tellg();
	i.seekg(0,std::ios::end);
	long m = i.tellg();
	size_t sSize = m-l;
	i.seekg(0);
	unsigned char* sBuff = (unsigned char*)malloc(sSize);
	i.read((char *)sBuff,sSize);
	if (Op::eIgnoreWhenExist == op)
	{
		auto iter = _fileMap.find(key);
		if (iter != _fileMap.end())
		{
			return Error::eFileExistError;
		}
	}
	ScriptPack::Pack pack;
	pack.contentLength = sSize;
	pack.content = sBuff;
	_fileMap[key] = pack;
	return Error::eSuccess;
}

int8 ScriptPack::writeToFile(const char* file)
{
	if (_fileMap.size() <= 0)
	{
		return Error::eNoContentSize;
	}

	std::ofstream o(file, std::ios::binary);

	size_t t = SCRIPE_BUFF_LENGTH;
	unsigned char* buff = (unsigned char*)malloc(SCRIPE_BUFF_LENGTH);
	size_t fileCount = 0;
	memset(buff,0,SCRIPE_BUFF_LENGTH);
	uint64 pos = sizeof(fileCount);
	for (auto iter = _fileMap.begin();iter != _fileMap.end();iter++)
	{
		std::string name = iter->first;
		uint16 nameLength = name.size();
		memcpy(&buff[pos],&nameLength,sizeof(nameLength));
		pos += sizeof(nameLength);

		memcpy(&buff[pos],iter->first.c_str(),nameLength);
		pos += nameLength;

		memcpy(&buff[pos],&iter->second.contentLength,sizeof(iter->second.contentLength));
		pos += sizeof(iter->second.contentLength);

		memcpy(&buff[pos],iter->second.content,(size_t)iter->second.contentLength);
		pos += iter->second.contentLength;
		delete iter->second.content;
		iter->second.content = nullptr;
		fileCount ++;

		if (t - pos < SCRIPE_BUFF_LENGTH * 0.2)
		{
			t += SCRIPE_BUFF_LENGTH;
			void * newBuff = realloc(buff, t);
			buff = (unsigned char*)newBuff;
		}
	}
	_fileMap.clear();

	uint64 len = pos;
	memcpy(buff,&fileCount,sizeof(fileCount));
	uLongf newLen = (uLongf)(pos < 1024?1024:pos);
	newLen += 8;
	unsigned char* newBuff = (unsigned char*)malloc(newLen);
	compress(&newBuff[sizeof(len)],&newLen,buff,(uLongf)pos);
	delete buff;
	memcpy(newBuff,&len,sizeof(len));
	o.write((const char*)newBuff,newLen+sizeof(len));
	o.close();
	delete newBuff;
	return Error::eSuccess;
}

int8 ScriptPack::decode(const unsigned char* sBuffData,int op)
{
	uint64 pos = 0;
	uint32 fileCount = 0;
	memcpy(&fileCount,sBuffData,4);
	pos = 4;

	for (uint32 i = 0;i<fileCount;i++)
	{
		uint16 nameLength = 0;
		memcpy(&nameLength,sBuffData+pos,sizeof(nameLength));
		pos += sizeof(nameLength);
		char* name = new char[nameLength+1];
		memset(name,0,nameLength+1);
		memcpy(name,sBuffData+pos,nameLength);
		pos += nameLength;
		uint64 contentLength = 0;
		memcpy(&contentLength,sBuffData+pos,sizeof(contentLength));
		pos += sizeof(contentLength);
		if (Op::eWriteWhenExist == op)
		{
			Pack pack;
			pack.contentLength = contentLength;
			pack.content = (unsigned char*)malloc((size_t)contentLength);
			memcpy(pack.content,sBuffData+pos,(size_t)contentLength);
			_fileMap[name] = pack;
		}
		pos += contentLength;
		delete[] name;
	}
	return Error::eSuccess;
}

int8 ScriptPack::unCompressFile(const char* file,int op)
{
	std::ifstream i(file,std::ios::binary);
	long l = i.tellg();
	i.seekg(0,std::ios::end);
	long m = i.tellg();
	size_t sSize = m-l;
	i.seekg(0);
	unsigned char* sBuff = (unsigned char*)malloc(sSize);
	i.read((char *)sBuff,sSize);

	uint64 pos = 0;
	uint64 len = 0;
	memcpy(&len,sBuff,8);
	if (len == 0)
		return Error::eNoContentSize;

	pos = sizeof(len);
	unsigned char* sBuffData = (unsigned char *)malloc((size_t)len+pos);
	memset(sBuffData,0,len+pos);
	if (!sBuffData)
		return 4;

	uLongf destLen = len;
	uncompress(sBuffData,&destLen,sBuff + 8,sSize-pos);
	delete sBuff;

	this->decode(sBuffData,op);
	delete sBuffData;
	return Error::eSuccess;
}

std::map<std::string,ScriptPack::Pack>& ScriptPack::getFileMap()
{
	return _fileMap;
}


