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
	if((lf = _findfirst((path + "\\*.*").c_str(), &file))==-1l)//_findfirst���ص���long��; long __cdecl _findfirst(const char *, struct _finddata_t *)
	{
		cout<<"�ļ�û���ҵ�!\n";
	}
    else
    {
        cout<<"\n�ļ��б�:\n";
        while( _findnext( lf, &file ) == 0 )//int __cdecl _findnext(long, struct _finddata_t *);����ҵ��¸��ļ������ֳɹ��Ļ��ͷ���0,���򷵻�-1
        {
            cout<<file.name;
            if(file.attrib == _A_NORMAL)cout<<"  ��ͨ�ļ�  ";
            else if(file.attrib == _A_RDONLY)cout<<"  ֻ���ļ�  ";
            else if(file.attrib == _A_HIDDEN )cout<<"  �����ļ�  ";
            else if(file.attrib == _A_SYSTEM )cout<<"  ϵͳ�ļ�  ";
            else if(file.attrib == _A_SUBDIR)
			{
				cout<<"  ��Ŀ¼  ";
				if (withSub && '.' != file.name[0])
				{
					getFiles(path + "\\" + file.name, withSub);
				}
			}
            else cout<<"  �浵�ļ�  ";
            cout<<endl;
        }
    }
    _findclose(lf);
}
