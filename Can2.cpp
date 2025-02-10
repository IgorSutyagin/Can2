
////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//  The CAN2 software is distributed under the following BSD 2-clause license and 
//  additional exclusive clauses. Users are permitted to develop, produce or sell their 
//  own non-commercial or commercial products utilizing, linking or including CAN2 as 
//  long as they comply with the license.BSD 2 - Clause License
// 
//  Copyright(c) 2024, TOPCON, All rights reserved.
// 
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met :
// 
//  1. Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and /or other materials provided with the distribution.
// 
//  3. The software package includes some companion executive binaries or shared 
//  libraries necessary to execute APs on Windows. These licenses succeed to the 
//  original ones of these software.
// 
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// 	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// 	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// 	OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// 	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "Can2.h"
#include "MainFrm.h"
#include "Tools.h"
#include "ChildFrm.h"
#include "Can2Doc.h"
#include "Can2View.h"
#include "RingSourcePg.h"
#include "RingDoc.h"
#include "RingTabView.h"
#include "RingFrm.h"
#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFileView* getFileView()
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();

	return &(pFrm->m_wndFileView);
}

CMainFrame* getMainFrame()
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();

	return pFrm;
}

CCan2App* getCan2App()
{
	return (CCan2App*)AfxGetApp();
}

// CCan2App

BEGIN_MESSAGE_MAP(CCan2App, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CCan2App::OnAppAbout)
	// Standard file based document commands
	//ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &OnFileOpen)
	// Standard print setup command
	//ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
	//ON_COMMAND(ID_FILE_OPEN, &CCan2App::OnFileOpen)
	ON_COMMAND(ID_TOOLS_RINGCALIBRATIONCLUSTERING, &CCan2App::OnToolsRingcalibrationclustering)
	ON_COMMAND(ID_TOOLS_RINGDPCOTOCSV, &CCan2App::OnToolsRingDpco2Csv)
	ON_COMMAND(ID_TOOLS_RINGMAXDPCO, &CCan2App::OnToolsRingMaxDpcoMaxDpcv2Csv)
END_MESSAGE_MAP()


// CCan2App construction

CCan2App::CCan2App() noexcept : m_ptemplPcv(nullptr)
{
	m_bHiColorIcons = TRUE;

	m_nAppLook = 0;
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("TOPCON.Can2.1_0_0_1"));

	m_gpToken = 0;

}

// The one and only CCan2App object

CCan2App theApp;


// CCan2App initialization

BOOL CCan2App::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction();

	Gdiplus::GdiplusStartupInput gpStartupInput;

	Gdiplus::Status st = Gdiplus::GdiplusStartup(&m_gpToken, &gpStartupInput, NULL);
	if (st != Gdiplus::Ok)
	{
		LPCTSTR szMsg = "GDI+ initialization failed";
		AfxMessageBox(szMsg);
		return FALSE;
	}
	// AfxInitRichEdit2() is required to use RichEdit control
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("TOPCON_Can2"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	m_startDirectory = std::filesystem::current_path().u8string();
	std::string fileSettings = m_startDirectory + "/can2.sts";
	can2::gl_settings.load(fileSettings.c_str());
	

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	m_ptemplPcv = new CMultiDocTemplate(IDR_Can2TYPE,
		RUNTIME_CLASS(CCan2Doc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CCan2View));
	if (!m_ptemplPcv)
		return FALSE;
	AddDocTemplate(m_ptemplPcv);

	m_ptemplRing = new CMultiDocTemplate(IDR_RING_TYPE,
		RUNTIME_CLASS(CRingDoc),
		RUNTIME_CLASS(CRingFrame), // custom MDI child frame
		RUNTIME_CLASS(CRingTabView));
	if (!m_ptemplRing)
		return FALSE;
	AddDocTemplate(m_ptemplRing);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	// Initialize screen parameters
	{
		HDC hdc = GetDC(NULL);

		int ncxScreen = GetSystemMetrics(SM_CXSCREEN);
		int ncyScreen = GetSystemMetrics(SM_CYSCREEN);
		int nHorRes = GetDeviceCaps(hdc, HORZRES);
		int hSize = GetDeviceCaps(hdc, HORZSIZE);
		int nVerRes = GetDeviceCaps(hdc, VERTRES);
		int vSize = GetDeviceCaps(hdc, VERTSIZE);

		double v = (double)nVerRes / (double)vSize;
		double h = (double)nHorRes / (double)hSize;

		int xLogPx = GetDeviceCaps(hdc, LOGPIXELSX);
		int yLogPx = GetDeviceCaps(hdc, LOGPIXELSY);
		int xLogPxMM = (int)(xLogPx / 25.4);
		int yLogPxMM = (int)(yLogPx / 25.4);


		m_sp.ppmm = (xLogPx + yLogPx) * 0.5 / 25.4;
		m_sp.ppinch = (xLogPx + yLogPx) * 0.5;
		m_sp.pixInPoint = m_sp.ppinch / 72;
		ReleaseDC(NULL, hdc);
	}


	return TRUE;
}

int CCan2App::ExitInstance()
{
	AfxOleTerm(FALSE);

	can2::gl_settings.save();

	Gdiplus::GdiplusShutdown(m_gpToken);

	return CWinAppEx::ExitInstance();
}

BOOL CCan2App::ProcessShellCommand(CCommandLineInfo& rCmdInfo)
{
	return TRUE;
}

////////////////////////////////////
// Operations:
void CCan2App::addNode(can2::Node* p)
{
	m_nodes.push_back(std::shared_ptr<can2::Node>(p));
}

void CCan2App::removeNode(can2::Node* p)
{
	auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [&](const std::shared_ptr<can2::Node>& a) {
		return a.get() == p;
	});
	if (it == m_nodes.end())
		return;
	m_nodes.erase(it);
}

////////////////////////////////////
// Implementation
CCan2View* CCan2App::findPcvView(const can2::Node* pn, can2::Gnss::Signal s) const
{
	POSITION pos = m_ptemplPcv->GetFirstDocPosition();
	while (pos != NULL)
	{
		CCan2Doc* pDoc = (CCan2Doc*)m_ptemplPcv->GetNextDoc(pos);
		if (pDoc->m_node == pn && pDoc->m_signal == s)
		{
			POSITION posView = pDoc->GetFirstViewPosition();
			while (posView != NULL)
			{
				CCan2View* pView = (CCan2View*)pDoc->GetNextView(posView);
				return pView;
			}
			return NULL;
		}
	}

	return NULL;
}

CRingTabView* CCan2App::findRingView(const can2::RingNode* pn) const
{
	POSITION pos = m_ptemplRing->GetFirstDocPosition();
	while (pos != NULL)
	{
		CRingDoc* pDoc = (CRingDoc*)m_ptemplRing->GetNextDoc(pos);
		if (pDoc->m_rn == pn)
		{
			POSITION posView = pDoc->GetFirstViewPosition();
			while (posView != NULL)
			{
				CRingTabView* pView = (CRingTabView*)pDoc->GetNextView(posView);
				return pView;
			}
			return NULL;
		}
	}

	return NULL;
}

void CCan2App::showPcv(can2::Node * node, can2::Gnss::Signal s)
{
	CCan2View* pView = findPcvView(node, s);
	if (pView == nullptr)
	{
		CCan2Doc* pDoc = (CCan2Doc*)m_ptemplPcv->CreateNewDocument();
		pDoc->m_node = node;
		pDoc->m_signal = s;

		CFrameWnd* pFrame = m_ptemplPcv->CreateNewFrame(pDoc, NULL);
		m_ptemplPcv->InitialUpdateFrame(pFrame, pDoc);
	}
	else
	{
		CMDIChildWnd* pChild = (CMDIChildWnd*)pView->GetParentFrame();
		pChild->MDIActivate();
	}
}

void CCan2App::showRingView(can2::RingNode* node)
{
	CRingTabView* pView = findRingView(node);
	if (pView == nullptr)
	{
		CRingDoc* pDoc = (CRingDoc*)m_ptemplRing->CreateNewDocument();
		pDoc->m_rn = node;

		CFrameWnd* pFrame = m_ptemplRing->CreateNewFrame(pDoc, NULL);
		m_ptemplRing->InitialUpdateFrame(pFrame, pDoc);
	}
	else
	{
		CMDIChildWnd* pChild = (CMDIChildWnd*)pView->GetParentFrame();
		pChild->MDIActivate();
	}

}
// CCan2App message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CCan2App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CCan2App customization load/save methods

void CCan2App::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CCan2App::LoadCustomState()
{
}

void CCan2App::SaveCustomState()
{
}

CDocument* CCan2App::OpenDocumentFile(LPCTSTR szFileName)
{
	return OpenDocumentFile(szFileName, FALSE);
}

CDocument* CCan2App::OpenDocumentFile(LPCTSTR lpszFileName, BOOL bAddToMRU)
{
	std::string fileName = lpszFileName;

	CWaitCursor wc;

	std::filesystem::path path(fileName);
	std::string strName = path.stem().u8string();
	std::string ext = path.extension().u8string();
	can2::ltrim(ext, "."); 
	ext = can2::tolower(ext.c_str());
	CFileView* pView = getFileView();

	if (ext == "atx")
	{
		std::shared_ptr<can2::AntexFile> patx;
		try
		{
			patx = can2::AntexFile::load(lpszFileName);
		}
		catch (std::exception e)
		{
			AfxMessageBox(e.what());
			return NULL;
		}

		if (patx == nullptr)
		{
			AfxMessageBox("Can't load atx file");
			return NULL;
		}

		m_nodes.push_back(patx);
		pView->createNode(patx.get());
	}
	else if (ext == "rng")
	{
		{
			std::ofstream ofs("test.bin");
			can2::Archive ar(ofs, can2::Archive::store);
			std::vector<double> v1(100000);
			std::vector<double> v2(100000);
			ar << v1;
			ar << v2;
		}
		try
		{
			std::ifstream ifs("test.bin");
			can2::Archive ar(ifs, can2::Archive::store);
			std::vector<double> v1;
			std::vector<double> v2;
			ar >> v1;
			ar >> v2;
		}
		catch (std::exception e)
		{
			AfxMessageBox(e.what());
		}

		std::shared_ptr<can2::RingNode> prn (new can2::RingNode());
		try
		{
			std::ifstream ifs(lpszFileName, std::ios_base::binary|std::ios_base::in);
			if (!ifs.good())
			{
				std::string str = can2::stringFormat("Can't open %s", lpszFileName);
				AfxMessageBox(str.c_str());
				return NULL;
			}

			can2::Archive ar(ifs, can2::Archive::load);

			prn->serialize(ar);

			m_nodes.push_back(prn);
			pView->createNode(prn.get());
		}
		catch (std::exception e)
		{
			AfxMessageBox(e.what());
			return NULL;
		}
	}
	else
	{
		AfxMessageBox("Unknown file type");
		return NULL;
	}

	if (bAddToMRU)
		AddToRecentFileList(lpszFileName);

	return NULL;
}



// CCan2App message handlers
void CCan2App::OnFileOpen()
{
	const int FILE_LIST_BUFFER_SIZE = 2048;
	CString fileName;
	CHAR* p = fileName.GetBuffer(FILE_LIST_BUFFER_SIZE);

	CFileDialog dlgFile(TRUE, "", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT,
		"All valid files (*.atx;*.rng)|*.atx;*.rng|Antex files (*.atx)|*.atx|Ring calibrations (*.rng)|*.rng|All files (*.*)|*.*||", AfxGetMainWnd());

	OPENFILENAME& ofn = dlgFile.GetOFN();
	ofn.Flags |= OFN_ALLOWMULTISELECT;
	ofn.lpstrFile = p;
	ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;

	if (dlgFile.DoModal() != IDOK)
		return;

	fileName.ReleaseBuffer();
	CStringArray strFiles;

	CHAR* pBufEnd = p + FILE_LIST_BUFFER_SIZE - 2;
	CHAR* start = p;
	while ((p < pBufEnd) && (*p))
		p++;
	if (p > start)
	{
		TRACE(_T("Path to folder where files were selected:  %s\r\n\r\n"), start);
		CString strPath = start;
		p++;

		CFileStatus fs;
		if (CFile::GetStatus(strPath, fs))
		{
			if (!(fs.m_attribute & CFile::directory))
			{
				strFiles.Add(strPath);
			}
		}

		int fileCount = 1;
		while ((p < pBufEnd) && (*p))
		{
			start = p;
			while ((p < pBufEnd) && (*p))
				p++;
			if (p > start)
				TRACE(_T("%2d. %s\r\n"), fileCount, start);
			p++;

			CString str = strPath + "\\" + start;
			bool bAlreadyExist = false;
			for (int j = 0; j < strFiles.GetSize(); j++)
			{
				if (strFiles[j].CompareNoCase(str) == 0)
				{
					bAlreadyExist = true;
					break;
				}
			}
			if (!bAlreadyExist)
			{
				fileCount++;
				strFiles.Add(str);
			}
		}
	}

	for (int i = 0; i < strFiles.GetSize(); i++)
	{
		CString str = strFiles[i];
		OpenDocumentFile(str);
	}

}


void CCan2App::OnToolsRingcalibrationclustering()
{
	CPropertySheet dlg;

	CRingSourcePg pgRing;

	dlg.AddPage(&pgRing);

	if (dlg.DoModal() != IDOK)
		return;

	m_nodes.push_back(pgRing.m_prn);
	CFileView* pView = getFileView();
	pView->createNode(pgRing.m_prn.get());

}

void CCan2App::OnToolsRingDpco2Csv()
{
	const char* szFiles[3] = {
		"D:\\User\\Ivs\\Tps2014\\cv2\\cv2\\RingAnt\\PPP\\dpcoVsClsm.csv",
		"D:\\User\\Ivs\\Tps2014\\cv2\\cv2\\RingAnt\\PPP\\dpcoVsGpp4.csv",
		"D:\\User\\Ivs\\Tps2014\\cv2\\cv2\\RingAnt\\PPP\\dpcoVsIGS.csv"
	};

	const char* szHeader = "antenna,GNSS,group,deltaN(mm),deltaE(mm),deltaU(mm),dpcc(mm)";

	struct DpcoDpcc {
		DpcoDpcc() : dpco(NAN, NAN, NAN), dpcc(NAN) {}
		can2::Point3d dpco;
		double dpcc;
	};

	struct LabData
	{
		std::string name;
		DpcoDpcc data[4];
	};

	std::vector<can2::RingNode*> rns;
	for (int i = 0; i < (int)m_nodes.size(); i++)
	{
		if (!m_nodes[i]->isRing())
			continue;
		can2::RingNode* prn = (can2::RingNode*)m_nodes[i].get();
		rns.push_back(prn);
	}
	if (rns.size() <= 0)
		return;

	auto findRef = [](int nref, can2::RingNode* prn) {
		for (int nc = 0; nc < (int)prn->m_cls.size(); nc++)
		{
			can2::RingNode::Cluster& c = prn->m_cls[nc];
			if (nref == 0)
			{
				if (c.ants.size() > 1)
					return c.atm.get();
			}
			else if (nref == 1)
			{
				if (c.ants.size() == 1 && c.ants[0]->getName() == "GPP4")
					return c.ants[0].get();
			}
			else if (nref == 2)
			{
				if (c.ants.size() == 1 && c.ants[0]->getName() == "IGS")
					return c.ants[0].get();
			}
		}
		return (can2::RingAntenna *)nullptr;
	};

	for (int nref = 0; nref < 3; nref++)
	{
		std::ofstream ofs(szFiles[nref]);
		if (!ofs.good())
		{
			AfxMessageBox("Can't save to file");
			continue;
		}

		ofs << szHeader << std::endl;

		for (int na = 0; na < (int)rns.size(); na++)
		{
			can2::RingNode* prn = rns[na];
			can2::RingAntenna* pref = findRef(nref, prn);
			if (pref == nullptr)
				continue;
			std::string antName = prn->m_title;
			for (int nlab = 0; nlab < (int)prn->m_ants.size(); nlab++)
			{
				can2::RingAntenna* pa = prn->m_ants[nlab].get();
				if (pa->getName() == pref->getName())
					continue;
				std::shared_ptr<can2::Node> pd(pref->subtract(pa));
				if (pa == pref)
					continue;
				can2::AntexAntenna* pda = (can2::AntexAntenna*)pd.get();
				if (!pda->isAntexAntenna())
					continue;

				std::string labName = pa->getName();
				can2::Gnss::Signal sigs[4] = { can2::Gnss::GIFL2, can2::Gnss::RIFL2, can2::Gnss::EIFL5a, can2::Gnss::CIF0206 };
				const char* szSys[4] = { "G", "R", "E", "C" };
				for (int sys = 0; sys < 4; sys++)
				{
					double ro = 0;
					can2::Point3d dpco = pda->calcOffset(sigs[sys], 7.0, can2::Node::eSinAndCos, &ro) * 1000;
					double dpcc = pda->calcNorm(sigs[sys], can2::Node::eSinAndCos, true) * 1000;
					ofs << antName << "," << szSys[sys] << "," << labName.c_str() << ",";
					ofs << (_isnan(dpco.n) ? "nan" : can2::stringFormat("%.1f", dpco.n)) << ",";
					ofs << (_isnan(dpco.e) ? "nan" : can2::stringFormat("%.1f", dpco.e)) << ",";
					ofs << (_isnan(dpco.u) ? "nan" : can2::stringFormat("%.1f", dpco.u)) << ",";
					ofs << (_isnan(dpcc) ? "nan" : can2::stringFormat("%.1f", dpcc)) << std::endl;
				}
			}
		}
	}
}

void CCan2App::OnToolsRingMaxDpcoMaxDpcv2Csv()
{
	const char* szFiles[3] = {
		"D:\\User\\Ivs\\Tps2014\\cv2\\cv2\\RingAnt\\PPP\\max_pco_pcc_pcv_vsClsm.csv",
		"D:\\User\\Ivs\\Tps2014\\cv2\\cv2\\RingAnt\\PPP\\max_pco_pcc_pcv_vsGpp4.csv",
		"D:\\User\\Ivs\\Tps2014\\cv2\\cv2\\RingAnt\\PPP\\max_pco_pcc_pcv_vsIGS.csv"
	};

	const char* szHeader = "antenna,GNSS,group,deltaPco(mm),deltaPcc(mm),dpcvAbove10(mm),dpcvBelow10(mm)";

	struct DpcoDpccDpcv {
		DpcoDpccDpcv() : dpco(NAN, NAN, NAN), dpcc(NAN), dpcv(NAN, NAN) {}
		can2::Point3d dpco;
		double dpcc;
		std::pair<double, double> dpcv; // first - above 10 deg, second - below 10 deg
	};

	struct LabData
	{
		std::string name;
		DpcoDpccDpcv data[4];
	};

	std::vector<can2::RingNode*> rns;
	for (int i = 0; i < (int)m_nodes.size(); i++)
	{
		if (!m_nodes[i]->isRing())
			continue;
		can2::RingNode* prn = (can2::RingNode*)m_nodes[i].get();
		rns.push_back(prn);
	}
	if (rns.size() <= 0)
		return;

	auto findRef = [](int nref, can2::RingNode* prn) {
		if (nref == 0)
		{
			for (int nc = 0; nc < (int)prn->m_cls.size(); nc++)
			{
				can2::RingNode::Cluster& c = prn->m_cls[nc];
				if (c.ants.size() > 1)
					return c.atm.get();
			}
		}
		else
		{
			for (int na = 0; na < (int)prn->m_ants.size(); na++)
			{
				can2::RingAntenna * pa = prn->m_ants[na].get();
				if (nref == 1)
				{
					if (pa->getName() == "GPP4")
						return pa;
				}
				else if (nref == 2)
				{
					if (pa->getName() == "IGS")
						return pa;
				}
			}
		}
		return (can2::RingAntenna*)nullptr;
	};

	for (int nref = 0; nref < 3; nref++)
	{
		std::ofstream ofs(szFiles[nref]);
		if (!ofs.good())
		{
			AfxMessageBox("Can't save to file");
			continue;
		}

		ofs << szHeader << std::endl;

		for (int na = 0; na < (int)rns.size(); na++)
		{
			can2::RingNode* prn = rns[na];
			can2::RingAntenna* pref = findRef(nref, prn);
			if (pref == nullptr)
				continue;
			std::string antName = prn->m_title;
			for (int nlab = 0; nlab < (int)prn->m_ants.size(); nlab++)
			{
				can2::RingAntenna* pa = prn->m_ants[nlab].get();
				if (pa->getName() == pref->getName())
					continue;
				std::shared_ptr<can2::Node> pd(pref->subtract(pa));
				if (pa == pref)
					continue;
				can2::AntexAntenna* pda = (can2::AntexAntenna*)pd.get();
				if (!pda->isAntexAntenna())
					continue;

				std::string labName = pa->getName();

				double ro = 0;
				can2::Point3d pcoComp(NAN, NAN, NAN);
				double dpccS = NAN;
				double dpco = NAN;
				{
					can2::Gnss::Signal sigMax = can2::Gnss::esigInvalid;
					for (int i = 0; i < sigMax; i++)
					{
						if (!pda->hasPcc(i))
							continue;
						can2::Point3d p = pda->calcOffset((can2::Gnss::Signal)i, 0.0, can2::Node::eSinAndCos, &ro) * 1000;
						double dS = pda->calcNorm((can2::Gnss::Signal)i, can2::Node::eSinAndCos, true) * 1000;
						for (int j = 0; j < 3; j++)
						{
							if (_isnan(pcoComp[j]))
								pcoComp[j] = p[j];
							else if (fabs(p[j]) > fabs(pcoComp[j]))
								pcoComp[j] = p[j];
						}
						if (_isnan(dpccS))
							dpccS = dS;
						else if (fabs(dpccS) < fabs(dS))
							dpccS = dS;
						if (_isnan(dpco))
							dpco = p.rad();
						else if (dpco < p.rad())
							dpco = p.rad();
					}
				}

				std::pair<double, double> r = pda->getMaxPcc(can2::Gnss::esigInvalid, false, 10.0, can2::Node::epmMinMax, can2::Node::eSinAndCos);
				double pcvHi = r.first * 1000;
				double pcvLo = r.second * 1000;

				ofs << antName << "," << labName.c_str() << "," 
					<< (_isnan(dpco) ? "nan" : can2::stringFormat ("%.1f", dpco)) << "," 
					<< (_isnan(dpccS) ? "nan" : can2::stringFormat ("%.1f", dpccS)) << "," 
					<< (_isnan(pcvHi) ? "nan" : can2::stringFormat ("%.1f", pcvHi)) << "," 
					<< (_isnan(pcvLo) ? "nan" : can2::stringFormat ("%.1f", pcvLo)) << std::endl;
			}
		}
	}
}
