
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
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "Tools.h"
#include "RingNode.h"

// CCan2App:
// See Can2.cpp for the implementation of this class
//

#include "Gnss.h"
#include "AntexFile.h"

class CCan2View;
class CFileView;
class CMainFrame;
class CRingTabView;
class CCan2App;

CFileView* getFileView();
CMainFrame* getMainFrame();
CCan2App* getCan2App();


class CCan2App : public CWinAppEx
{
public:
	CCan2App() noexcept;
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	BOOL ProcessShellCommand(CCommandLineInfo& rCmdInfo);

// Operations:
public:
	void addNode(can2::Node* p);
	void removeNode(can2::Node* p);
	void showPcv(can2::Node * node, can2::Gnss::Signal s);
	void showRingView(can2::RingNode* node);

// Attributes:
public:
	can2::ScreenParams m_sp;
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	std::string m_startDirectory;
	ULONG_PTR m_gpToken;

// Implementation
protected:

	std::vector<std::shared_ptr<can2::Node>> m_nodes;
	CMultiDocTemplate* m_ptemplPcv;
	CMultiDocTemplate* m_ptemplRing;
	CCan2View* findPcvView(const can2::Node* pn, can2::Gnss::Signal s) const;
	CRingTabView* findRingView(const can2::RingNode* pn) const;

	// Overrides
public:

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName, BOOL bAddToMRU);

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileOpen();
	afx_msg void OnToolsRingcalibrationclustering();
};

extern CCan2App theApp;

inline const can2::ScreenParams& getSP() {
	return theApp.m_sp;
}
