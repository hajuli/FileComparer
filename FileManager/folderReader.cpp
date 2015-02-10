// stdafx.cpp : source file that includes just the standard includes
// FileManager.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

#include "folderReader.h"
#include<iostream>
#include<io.h>

using namespace std;

FolderReader::FolderReader(std::string path):
m_folderName(path)
{
}


FolderReader::~FolderReader()
{
}

void FolderReader::getFiles(std::string path, bool withSub)
{
	_finddata_t file;
    long lf;
	if((lf = _findfirst((path + "\\*.*").c_str(), &file))==-1l)//_findfirst返回的是long型; long __cdecl _findfirst(const char *, struct _finddata_t *)
	{
		cout<<"文件没有找到!\n";
	}
    else
    {
        cout<<"\n文件列表:\n";
        while( _findnext( lf, &file ) == 0 )//int __cdecl _findnext(long, struct _finddata_t *);如果找到下个文件的名字成功的话就返回0,否则返回-1
        {
            cout<<file.name;
            if(file.attrib == _A_NORMAL)cout<<"  普通文件  ";
            else if(file.attrib == _A_RDONLY)cout<<"  只读文件  ";
            else if(file.attrib == _A_HIDDEN )cout<<"  隐藏文件  ";
            else if(file.attrib == _A_SYSTEM )cout<<"  系统文件  ";
            else if(file.attrib == _A_SUBDIR)
			{
				cout<<"  子目录  ";
				if (withSub && '.' != file.name[0])
				{
					getFiles(path + "\\" + file.name, withSub);
				}
			}
            else cout<<"  存档文件  ";
            cout<<endl;
        }
    }
    _findclose(lf);
}
