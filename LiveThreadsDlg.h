// LiveThreadsDlg.h : header file
//

#pragma once


// CLiveThreadsDlg dialog
class CLiveThreadsDlg : public CDialog
{
// Construction
public:
	CLiveThreadsDlg(CWnd* pParent = NULL);	// standard constructor
	void DoSend(int size);
// Dialog Data
	enum { IDD = IDD_LIVETHREADS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedObject();
	afx_msg void OnBnClickedCheck();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTotal(WPARAM, LPARAM);
	afx_msg LRESULT OnSuccess(WPARAM, LPARAM);
	afx_msg LRESULT OnFail(WPARAM, LPARAM);
	afx_msg LRESULT OnEnableObjectButton(WPARAM, LPARAM);
};
