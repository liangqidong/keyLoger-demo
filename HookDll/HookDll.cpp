/*---------------------------------------------------------------------------
FileName:HookDll.cpp
Usage:Hook Library V1.2
������ �����������[F.S.T] 2005/10/08
��վ:http://www.wrsky.com
��Ȩ��ԭ��������
----------------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <imm.h>
#pragma comment(lib,"imm32.lib") 
#define HOOKDLL_API __declspec(dllexport)
#define WM_KEYHOOK       WM_USER+100 //������Ϣ
#define WM_MSHOOK        WM_USER+101 //�����Ϣ
#define WM_KEYHOOK_CN    WM_USER+102 //���ļ�����Ϣ
#define WM_KEYHOOK_CN_EN WM_USER+103 //��Ӣ����
#pragma data_seg(".inidata")
//HWND APP_hdl;                       //����������ó��򴰿ھ��
static HINSTANCE hdll_lib;                   //��������ö�̬���ӿ�ľ��
static HHOOK KBhookEn,KBhookCn,MShook;                  //���Ӿ��
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
LRESULT CALLBACK KBhook_deal(int nCode,WPARAM wParam,LPARAM lParam);//���̹��ӻص�����
LRESULT CALLBACK KBhookCn_deal(int nCode,WPARAM wParam,LPARAM lParam);//֧�����ĵļ��̹��ӻص�����
LRESULT CALLBACK MShook_deal(int nCode,WPARAM wParam,LPARAM lParam);//��깳�ӻص�����
                                    //DLL����ں���
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
  switch(ul_reason_for_call){
    case DLL_PROCESS_ATTACH://���̹߳��캯��
          hdll_lib=(HINSTANCE)hModule; //��������DLL���
          break;
    case DLL_PROCESS_DETACH://���߳���������
         break;
    default:
         break;
    }
    return TRUE;
}
/*---------------------------���̹������ú���-----------------------------------*/
extern "C" __declspec(dllexport) void SetKBHook(void){
  if(KBhookEn==NULL)
     KBhookEn=SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC)KBhook_deal,hdll_lib,0);//����ȫ�ֹ���
}
/*---------------------------���̹������ú���[֧������]-----------------------------------*/
extern "C" __declspec(dllexport) void SetKBHookCn(void){
  if(KBhookCn==NULL)
     KBhookCn=SetWindowsHookEx(WH_GETMESSAGE,(HOOKPROC)KBhookCn_deal,hdll_lib,0);//����ȫ�ֹ���
}
/*---------------------------���̹���ж�غ���-----------------------------------*/
extern "C" __declspec(dllexport) void RemoveKBHook(void){
  if(KBhookCn)
    UnhookWindowsHookEx(KBhookCn); //ж�ع���
  if(KBhookEn)
    UnhookWindowsHookEx(KBhookEn); //ж�ع���
}
/*--------------------------���̹��ӻص���������--------------------------------*/
LRESULT CALLBACK KBhook_deal(int nCode,WPARAM wParam,LPARAM lParam){
  if(nCode<0) return(CallNextHookEx(KBhookEn,nCode,wParam,lParam)); //����Ϣ���ݸ���һ������
  //SendMessage(APP_hdl,WM_KEYHOOK,wParam,lParam);                //��KEYSPY���ó�������Ϣ
  HANDLE hThreadid=OpenFileMapping(FILE_MAP_WRITE,false,"Threadid");
  if(hThreadid){
    int *pThreadid=(int *)MapViewOfFile(hThreadid,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
    if(pThreadid) KBthreadid=*pThreadid;
    }
  else MessageBox(NULL,"no find meme","no",MB_OK);
  if(true){
    PostThreadMessage(KBthreadid,WM_KEYHOOK,wParam,lParam);         //���̷߳�����Ϣ
	}
  //else MessageBox(NULL,"false","false",MB_OK);
  if(DisableKB) return 1;
  return 0;
}
/*--------------------------���̹��ӻص���������[֧������]--------------------------------*/
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
  /* this code from ZWELL ������뷨�������ַ��� */
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
                 //�Ȼ�ȡ��ǰ��������Ĵ��ڵ����뷨���
                 hIMC = ImmGetContext(hWnd);
                 if(!hIMC)  return 0;
                 // �Ƚ�ImmGetCompositionString�Ļ�ȡ������Ϊ0����ȡ�ַ�����С.
                 dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
                 // ��������СҪ�����ַ�����NULL��������С,
                 // ���ǵ�UNICODE
                 dwSize += sizeof(WCHAR);
                 memset(lpstr, 0, sizeof(lpstr));
                 // �ٵ���һ��.ImmGetCompositionString��ȡ�ַ���
                 ImmGetCompositionString(hIMC, GCS_RESULTSTR, lpstr, dwSize);
                 //����lpstr���漴������ĺ����ˡ�����Դ���lpstr,��ȻҲ���Ա���Ϊ�ļ�...
                 //MessageBox(NULL, lpstr, lpstr, MB_OK); 
                 strncpy(pCNString,lpstr,sizeof(lpstr));
                 ImmReleaseContext(hWnd, hIMC);
                 PostThreadMessage(KBthreadid,WM_KEYHOOK_CN_EN,wParam,lParam);         //���̷߳�����Ϣ
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
            //	WM_CHAR��Ӧ����ʾ�ַ�������Ϣ��WM_KEYDOWN��Ӧ���Ƽ���Ϣ
            /*
            if(ImmIsIME(GetKeyboardLayout(0))){                       //�ж��������뷨
              if((unsigned char)(pmsg->wParam)>0x7F){
                //MessageBox(NULL,"quit","char",MB_OK);
                return 0;
                }
              }
            strncpy(pCNString,(char *)&(pmsg->wParam),1);
            */
            lParam=pmsg->lParam;//ɨ����
            wParam=pmsg->wParam;//������
            char str[20]={0};
            sprintf(str,"0x=%x",(unsigned char)pmsg->wParam);
            //MessageBox(NULL,str,"char",MB_OK);
            PostThreadMessage(KBthreadid,WM_KEYHOOK_CN,wParam,lParam);         //���̷߳�����Ϣ
            }
  if(DisableKB) return 1;
  return 0;
}
//==============================================================================
/*---------------------------��깳�����ú���-----------------------------------*/
extern "C" __declspec(dllexport) void SetMSHook(void){
  if(MShook==NULL)
     MShook=SetWindowsHookEx(WH_MOUSE,(HOOKPROC)MShook_deal,hdll_lib,0);//����ȫ�ֹ���
}
/*---------------------------��깳��ж�غ���-----------------------------------*/
extern "C" __declspec(dllexport) void RemoveMSHook(void){
  if(MShook)
    UnhookWindowsHookEx(MShook); //ж�ع���
}
/*--------------------------��깳�ӻص���������--------------------------------*/
LRESULT CALLBACK MShook_deal(int nCode,WPARAM wParam,LPARAM lParam){
  if(nCode<0) return(CallNextHookEx(MShook,nCode,wParam,lParam)); //����Ϣ���ݸ���һ������
  //SendMessage(APP_hdl,WM_KEYHOOK,wParam,lParam);                //��KEYSPY���ó�������Ϣ
  if(MSStart) PostThreadMessage(MSthreadid,WM_MSHOOK,wParam,lParam);          //���̷߳�����Ϣ
  if(DisableMS) return 1;
  return 0;
}
/*--------------------------�ϳ�����--------------------------------*/
extern "C" __declspec(dllexport) void DisableKeyBoard(bool flag){
DisableKB=flag;
}
/*--------------------------�ϳ����--------------------------------*/
extern "C" __declspec(dllexport) void DisableMouse(bool flag){
DisableMS=flag;
}
/*--------------------------������̹�����Ϣ--------------------------------*/
extern "C" __declspec(dllexport) void StartKBHook(){
KBStart=true;
}
/*--------------------------������깳����Ϣ--------------------------------*/
extern "C" __declspec(dllexport) void StartMSHook(){
MSStart=true;
}
/*--------------------------ֹͣ���̹��Ӵ���--------------------------------*/
extern "C" __declspec(dllexport) void StopKBHook(){
KBStart=false;
}
/*--------------------------ֹͣ��깳�Ӵ���--------------------------------*/
extern "C" __declspec(dllexport) void StopMSHook(){
MSStart=false;
}
/*--------------------------���ü��̹�����Ϣ�����߳�ID--------------------------------*/
extern "C" __declspec(dllexport) void SetKBHookThreadId(int threadid){
KBthreadid=threadid;
}
/*--------------------------������깳����Ϣ�����߳�ID--------------------------------*/
extern "C" __declspec(dllexport) void SetMSHookThreadId(int threadid){
MSthreadid=threadid;
}
