// LiveThreadsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LiveThreads.h"
#include "LiveThreadsDlg.h"
#include ".\netsend.h"
#include ".\livethreadsdlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define QSIZE 1000

int ThreadCount;
int AliveThreads;
HWND hDlg;
// CLiveThreadsDlg dialog
Calc *the_calc[MAX_OBJECTS];
queue <Info *> list(QSIZE);
WaitingThread *wt;
Info I[100];

//HANDLE endEvent;

CLiveThreadsDlg::CLiveThreadsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLiveThreadsDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLiveThreadsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLiveThreadsDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_OBJECT, OnBnClickedObject)
	ON_MESSAGE(WM_ENABLEOBJECTBUTTON, OnEnableObjectButton)
	ON_BN_CLICKED(IDC_CHECK, OnBnClickedCheck)
	ON_WM_TIMER()
	ON_MESSAGE(WM_TOTAL, OnTotal)
	ON_MESSAGE(WM_SUCCESS, OnSuccess)
	ON_MESSAGE(WM_FAIL, OnFail)
ON_WM_DESTROY()
END_MESSAGE_MAP()


// CLiveThreadsDlg message handlers
LRESULT CLiveThreadsDlg::OnTotal(WPARAM, LPARAM)
{
	char buf[20];
	sprintf(buf, "%d", total);
	GetDlgItem(IDC_TOTAL)->SetWindowText(buf);
	return 0;
}

LRESULT CLiveThreadsDlg::OnSuccess(WPARAM, LPARAM)
{
	char buf[20];
	sprintf(buf, "%d", success);
	GetDlgItem(IDC_SUCCESS)->SetWindowText(buf);
	return 0;
}

LRESULT CLiveThreadsDlg::OnFail(WPARAM, LPARAM)
{
	char buf[20];
	sprintf(buf, "%d", fail);
	GetDlgItem(IDC_FAIL)->SetWindowText(buf);
	return 0;
}
LRESULT CLiveThreadsDlg::OnEnableObjectButton(WPARAM, LPARAM)
{
	GetDlgItem(IDC_OBJECT)->EnableWindow();
	return 0;
}
BOOL CLiveThreadsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	InitializeCriticalSection(&csSerializeDraw);
	ThreadCount = 0;
	AliveThreads = 0;
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		status[i] = STATUS_DEAD;
		the_calc[i] = NULL;
	}
	hDlg = m_hWnd;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLiveThreadsDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLiveThreadsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void WaitThreads(void)
{
	bool stop = false;	
	int AliveThreads = 0;

	while (!stop)
	{
		AliveThreads = 0;
		for (int i = 0; i < ThreadCount; i++)
		{
			if (status[i] == STATUS_DEAD)
			{
				if (the_calc[i] != NULL)
				{
					delete the_calc[i];
					the_calc[i] = NULL;
				}
			}
			else
				AliveThreads++;
		}
		stop = (AliveThreads == 0);
	}
}

int nextFree(int start)
{
	int j = -1;
	if (start >= MAX_OBJECTS) return j;
	for (int i = start; i < MAX_OBJECTS; i++)
	{
		if (status[i] == STATUS_DEAD)
		{
			j = i;
			break;
		}
	}
	return j;
}

void StartFromQueue(void)
{
	int k = 0;
	while (!list.isEmpty())
	{
		k = nextFree(k);
		if (k >= 0)
		{
			Info *i = list.pop();
			//printf("from queue %d %d\n", k, i->number);
			the_calc[k] = new Calc(k, i->number, hDlg);
			ThreadCount++;
			k++;
		}
		else
		{
			WaitThreads();
			k = 0;
			ThreadCount = 0;
		}
	}
	WaitThreads();
}

void DoSend(int size)
{
	srand( (unsigned)time( NULL ) );
	for (int i = 0; i < size; i++)
	{
		int r = (i+1)*100;
		I[i].number = r;
		list.push(&I[i]);
 	}
 	StartFromQueue();
}



void CLiveThreadsDlg::OnBnClickedObject()
{
	total = 0;
	fail = 0;
	success = 0;
	OnTotal(0,0);
	OnSuccess(0,0);
	OnFail(0,0);
	AliveThreads = 0;
	ThreadCount = 0;
	for (int i = 0; i < MAX_OBJECTS; i++)
		status[i] = STATUS_DEAD;
    endEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    ThreadPool *the_pool = new ThreadPool(12);
	if (wt != NULL)
	{
		delete wt;
		//wt = NULL;
	}
	GetDlgItem(IDC_OBJECT)->EnableWindow(FALSE);
	wt = new WaitingThread(the_pool, m_hWnd);
    /*if (WaitForSingleObject(endEvent, INFINITE) == WAIT_TIMEOUT)
    {
    	;
    }
    CloseHandle(endEvent);
    delete the_pool;
	the_pool = NULL;*/
}

void CLiveThreadsDlg::OnBnClickedCheck()
{
	int AliveThreads = 0;

	for (int i = 0; i < ThreadCount; i++)
	{
		if (status[i] == STATUS_ALIVE)
			AliveThreads++;
	}
	char buf[20];
	sprintf(buf, "%d", AliveThreads);
	GetDlgItem(IDC_WAITING)->SetWindowText(buf);
}

void CLiveThreadsDlg::OnDestroy()
{
	
	if ((wt != NULL) && (wt->finished))
	{
		delete wt;
		//wt = NULL;
	}
	CDialog::OnDestroy();
}
