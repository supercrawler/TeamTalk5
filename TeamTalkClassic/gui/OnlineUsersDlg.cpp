// OnlineUsersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "OnlineUsersDlg.h"
#include <vector>

extern TTInstance* ttInst;

// COnlineUsersDlg dialog

IMPLEMENT_DYNAMIC(COnlineUsersDlg, CDialog)

COnlineUsersDlg::COnlineUsersDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COnlineUsersDlg::IDD, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

COnlineUsersDlg::~COnlineUsersDlg()
{
}

void COnlineUsersDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_ONLINEUSERS, m_wndUsers);
}


BEGIN_MESSAGE_MAP(COnlineUsersDlg, CDialog)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_POPUP_KICK, &COnlineUsersDlg::OnPopupKick)
    ON_COMMAND(ID_POPUP_KICKANDBAN, &COnlineUsersDlg::OnPopupKickandban)
    ON_COMMAND(ID_POPUP_OP, &COnlineUsersDlg::OnPopupOp)
    ON_COMMAND(ID_POPUP_COPYUSERINFORMATION, &COnlineUsersDlg::OnPopupCopyuserinformation)
END_MESSAGE_MAP()


// COnlineUsersDlg message handlers

BOOL COnlineUsersDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    //load accelerators
    m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(), (LPCTSTR)IDR_ACCELERATOR3);
    if (!m_hAccel)
        MessageBox(_T("The accelerator table was not loaded"));

    m_wndUsers.SetExtendedStyle(m_wndUsers.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

    m_wndUsers.InsertColumn(COLUMN_USERID, _T("ID"));
    m_wndUsers.SetColumnWidth(COLUMN_USERID, 45);
    m_wndUsers.InsertColumn(COLUMN_NICKNAME, _T("Nickname"));
    m_wndUsers.SetColumnWidth(COLUMN_NICKNAME, 100);
    m_wndUsers.InsertColumn(COLUMN_STATUSMSG, _T("Status message"));
    m_wndUsers.SetColumnWidth(COLUMN_STATUSMSG, 100);
    m_wndUsers.InsertColumn(COLUMN_USERNAME, _T("Username"));
    m_wndUsers.SetColumnWidth(COLUMN_USERNAME, 65);
    m_wndUsers.InsertColumn(COLUMN_CHANNEL, _T("Channel"));
    m_wndUsers.SetColumnWidth(COLUMN_CHANNEL, 100);
    m_wndUsers.InsertColumn(COLUMN_IPADDRESS, _T("IP-address"));
    m_wndUsers.SetColumnWidth(COLUMN_IPADDRESS, 100);
    m_wndUsers.InsertColumn(COLUMN_VERSION_, _T("Version"));
    m_wndUsers.SetColumnWidth(COLUMN_VERSION_, 100);

    int nUsers = 0;
    TT_GetServerUsers(ttInst, NULL, &nUsers);

    std::vector<User> users;
    if(nUsers)
    {
        users.resize(nUsers);
        TT_GetServerUsers(ttInst, &users[0], &nUsers);
        for(size_t i=0;i<nUsers;i++)
        {
            CString s;
            s.Format(_T("%d"), users[i].nUserID);
            int iIndex = m_wndUsers.InsertItem(i, s, 0);
            m_wndUsers.SetItemText(iIndex, COLUMN_NICKNAME, users[i].szNickname);
            m_wndUsers.SetItemText(iIndex, COLUMN_STATUSMSG, users[i].szStatusMsg);
            m_wndUsers.SetItemText(iIndex, COLUMN_USERNAME, users[i].szUsername);
            TTCHAR szChannel[TT_STRLEN] = _T("");
            TT_GetChannelPath(ttInst, users[i].nChannelID, szChannel);
            m_wndUsers.SetItemText(iIndex, COLUMN_CHANNEL, szChannel);
            m_wndUsers.SetItemText(iIndex, COLUMN_IPADDRESS, users[i].szIPAddress);
            m_wndUsers.SetItemText(iIndex, COLUMN_VERSION_, GetVersion(users[i]));
            m_wndUsers.SetItemData(iIndex, users[i].nUserID);
        }
    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void COnlineUsersDlg::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    CMenu menu;
    menu.LoadMenu(IDR_MENU_ONLINEUSERS);
    TRANSLATE(menu);
    CMenu* pop = menu.GetSubMenu(0);
    if(!pop)
        return;

    UINT uCmd = pop->TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN |
        TPM_RIGHTBUTTON, point.x, point.y,
        this, NULL );
    MenuCommand(uCmd);
}

BOOL COnlineUsersDlg::PreTranslateMessage(MSG* pMsg)
{
    if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
        if (m_hAccel && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
            return TRUE;

    return CDialog::PreTranslateMessage(pMsg);
}

void COnlineUsersDlg::MenuCommand(UINT uCmd)
{
    int nUserID =  0;
    int count = m_wndUsers.GetItemCount();
    for(int i=0;i<count;i++)
    {
        if(m_wndUsers.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
            nUserID = m_wndUsers.GetItemData(i);
    }

    User user = {0};
    TT_GetUser(ttInst, nUserID, &user);
    Channel chan = {0};
    TT_GetChannel(ttInst, user.nChannelID, &chan);

    switch(uCmd)
    {
    case ID_POPUP_KICKANDBAN :
        TT_DoBanUser(ttInst, nUserID, 0);
        TT_DoKickUser(ttInst, nUserID, 0);
        break;
    case ID_POPUP_KICK :
        TT_DoKickUser(ttInst, nUserID, user.nChannelID);
        break;
    case ID_POPUP_OP :
        TT_DoChannelOpEx(ttInst, nUserID, user.nChannelID, chan.szOpPassword, 
                         !TT_IsChannelOperator(ttInst, nUserID, user.nChannelID));
        break;
    case ID_POPUP_COPYUSERINFORMATION :
    {
        User user;
        if(TT_GetUser(ttInst, nUserID, &user))
        {
            CString szText;
            CString szUserID;
            szUserID.Format(_T("%d"), user.nUserID);
            TTCHAR szChannel[TT_STRLEN] = _T("");
            TT_GetChannelPath(ttInst, user.nChannelID, szChannel);
            szText = szUserID;
            szText += _T("\t");
            szText += user.szNickname;
            szText += _T("\t");
            szText += user.szStatusMsg;
            szText += _T("\t");
            szText += user.szUsername;
            szText += _T("\t");
            szText += szChannel;
            szText += _T("\t");
            szText += user.szIPAddress;
            szText += _T("\t");
            szText += GetVersion(user);

            OpenClipboard();
            EmptyClipboard();
            HGLOBAL hglbCopy;
            hglbCopy = GlobalAlloc(GMEM_MOVEABLE,
                                   (szText.GetLength() + 1) * sizeof(TCHAR));
            if(hglbCopy)
            {
                LPVOID szStr = GlobalLock(hglbCopy);
                memcpy(szStr, szText.GetBuffer(), (szText.GetLength() + 1) * sizeof(TCHAR));
                GlobalUnlock(hglbCopy);
#if defined(UNICODE) || defined(_UNICODE)
                SetClipboardData(CF_UNICODETEXT, hglbCopy);
#else
                SetClipboardData(CF_TEXT, hglbCopy);
#endif
            }
            CloseClipboard();
        }
    }
    }
}

void COnlineUsersDlg::OnPopupKick()
{
    MenuCommand(ID_POPUP_KICK);
}

void COnlineUsersDlg::OnPopupKickandban()
{
    MenuCommand(ID_POPUP_KICKANDBAN);
}

void COnlineUsersDlg::OnPopupOp()
{
    MenuCommand(ID_POPUP_OP);
}

void COnlineUsersDlg::OnPopupCopyuserinformation()
{
    MenuCommand(ID_POPUP_COPYUSERINFORMATION);
}
