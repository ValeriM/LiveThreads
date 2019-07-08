#pragma once
#define STATUS_ALIVE 0
#define STATUS_DEAD 1
#define MAX_OBJECTS 200

#define DELAY 50
#define IDM_STARTSIM             100
#define IDM_STOPSIM 		 101
#define IDM_ADDFLY     102

#define WM_OBJECT_DIED WM_USER+0xc001
#define WM_TOTAL WM_USER+1
#define WM_SUCCESS WM_TOTAL+1
#define WM_FAIL WM_SUCCESS+1
#define WM_ENABLEOBJECTBUTTON WM_FAIL+1

struct Info
{
	int number;
};
void DoSend(int size);
#ifdef CLASSES
CRITICAL_SECTION csSerializeDraw;
short status[MAX_OBJECTS];
int total;
int fail;
int success;
HANDLE endEvent;
#else
extern CRITICAL_SECTION csSerializeDraw;
extern short status[MAX_OBJECTS];
extern int total;
extern int fail;
extern int success;
extern HANDLE endEvent;
#endif

class ThreadPool
{
  HANDLE myHandle;
  DWORD ID;
  int size;
  
  public:
  bool finished;
  ThreadPool(int Size)
  {
	  finished = false;
	  size = Size;
      myHandle = CreateThread(
        NULL,                        // default security attributes
        0,                           // use default stack size
        run,                  // thread function
        this,                // argument to thread function
        0,                           // use default creation flags
        &ID);                // returns the thread identifier
  }
  ~ThreadPool(void)
  {
	  CloseHandle(myHandle);
  }
  
  static DWORD WINAPI run(LPVOID p)
  {
    ThreadPool *me = (ThreadPool *)p;

    me->Run();
	
    return 0;
  }
  void Run(void)
  {
	  DoSend(size);
	  SetEvent(endEvent);
	  finished = true;
  }
};

class WaitingThread
{
  HANDLE myHandle;
  DWORD ID;
  int size;
  ThreadPool *myPool;
  HWND hWindow;
  public:
  bool finished;
  WaitingThread(ThreadPool *pool, HWND hDlg)
  {
	  hWindow = hDlg;
	  myPool = pool;
	  finished = false;
      myHandle = CreateThread(
        NULL,                        // default security attributes
        0,                           // use default stack size
        run,                  // thread function
        this,                // argument to thread function
        0,                           // use default creation flags
        &ID);                // returns the thread identifier
  }
  ~WaitingThread(void)
  {
	  CloseHandle(myHandle);
  }
  
  static DWORD WINAPI run(LPVOID p)
  {
    WaitingThread *me = (WaitingThread *)p;

    me->Run();
	
    return 0;
  }
  void Run(void)
  {
	if (WaitForSingleObject(endEvent, INFINITE) == WAIT_TIMEOUT)
    {
    	;
    }
    CloseHandle(endEvent);
    delete myPool;
	myPool = NULL;
	finished = true;
	PostMessage(hWindow, WM_ENABLEOBJECTBUTTON, 0, 0);
  }
};

class Calc
{
  HANDLE myHandle;
  HWND hDialog;
  DWORD ID;
  int index;
  int R;
  
  public:
  bool finished;
  bool Success()
  {
	  return (R > 500);
  }
  Calc(int Index, int r, HWND hwDialog)
  {
      hDialog = hwDialog;
	  R = r;
	  finished = false;
	  index = Index;
	  status[index] = STATUS_ALIVE;	
      myHandle = CreateThread(
        NULL,                        // default security attributes
        0,                           // use default stack size
        run,                  // thread function
        this,                // argument to thread function
        0,                           // use default creation flags
        &ID);                // returns the thread identifier
	  //SetThreadPriority(myHandle, THREAD_PRIORITY_BELOW_NORMAL);
  }
  ~Calc(void)
  {
	  CloseHandle(myHandle);
  }
  
  static DWORD WINAPI run(LPVOID p)
  {
    Calc *me = (Calc *)p;

    me->Run();
	
    return 0;
  }
  void Run(void)
    {
      Sleep(R);
	  total++;
	  PostMessage(hDialog, WM_TOTAL, 0, 0);
	  if (Success())
	  {
		success++;
		PostMessage(hDialog, WM_SUCCESS, 0, 0);
	  }
	  else
	  {
		fail++;
		PostMessage(hDialog, WM_FAIL, 0, 0);
	  }
	  
	  status[index] = STATUS_DEAD;
	  finished = true;
    }
};

template <class Qtype> class queue
{
	Qtype *q;
	int sloc, rloc;
	int length;
public:
	queue(int size);
	~queue() { delete []q; }
	bool push(Qtype i);
	Qtype pop();
	bool isEmpty(void) {return (rloc == sloc);}
};

template <class Qtype> queue <Qtype>::queue(int size)
{
	size++;
	q = new Qtype[size];
	length = size;
	sloc = rloc = 0;	
}

template <class Qtype> bool queue <Qtype>::push(Qtype i)
{
	if (sloc+1 == length) return false;
	sloc++;
	q[sloc] = i;
	return true;
}

template <class Qtype> Qtype queue <Qtype>::pop()
{
	if (rloc == sloc) return NULL;
	rloc++;
	return q[rloc];
}


