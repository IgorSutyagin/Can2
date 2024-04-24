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

#include "Settings.h"
#include "RingNode.h"

namespace can2
{
	Settings gl_settings;
}

using namespace can2;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Collection of Can2 settings implementation

/////////////////////////////
// Construction
Settings::Settings()
{
	for (int i = 0; i < ep3Max; i++)
	{
		for (int j = 0; j < Plot3dCamera::ecMax; j++)
		{
			plot3d[i].camera[j] = std::shared_ptr<Plot3dCamera>(new Plot3dCamera());
		}
		plot3d[i].params = std::shared_ptr<Plot3dParams>(new Plot3dParams());
	}
}

Settings::~Settings()
{
}

void Settings::savePlot3d(const Node* node, const Plot3d* plot)
{
	if (node == nullptr)
		return;

	int type = 0;
	if (node->isAntenna())
	{
		AntexAntenna* pa = (AntexAntenna*)node;
		if (pa->m_bDifference)
			type = 1;
	}

	*(plot3d[type].params) = plot->getViewParams();
	*(plot3d[type].camera[plot3d[type].params->coords]) = plot->getCamera();
}

void Settings::loadPlot3d(const Node* node, Plot3d* plot) const
{
	if (node == nullptr)
		return;

	int type = 0;
	if (node->isAntenna())
	{
		AntexAntenna* pa = (AntexAntenna*)node;
		if (pa->m_bDifference)
			type = 1;
	}

	plot->getViewParams() = *(plot3d[type].params);
	plot->getCamera() = *(plot3d[type].camera[plot3d[type].params->coords]);
}

//////////////////////////////////
// Serialization:
void Settings::load(const char * szPathName)
{
	pathName = szPathName;
	try
	{
		std::ifstream ifs(szPathName, std::ios_base::binary | std::ios_base::in);
		if (!ifs.good())
		{
			return;
		}

		Archive ar(ifs, Archive::load);

		serialize(ar);
	}
	catch (std::exception e)
	{
		AfxMessageBox(e.what());
	}
}

void Settings::save()
{
	if (pathName.empty())
		return;

	std::ofstream ofs(pathName.c_str(), std::ios_base::binary | std::ios_base::out);
	if (!ofs.good())
	{
		AfxMessageBox("Can't save settings");
		return;
	}

	{
		can2::Archive ar(ofs, Archive::store);

		serialize(ar);
	}
}

void Settings::serialize(Archive& ar)
{
	DWORD dwVer = 1;
	if (ar.isStoring())
	{
		ar << dwVer;

		int types = ep3Max;
		ar << types;
		int coords = Plot3dCamera::ecMax;
		ar << coords;
		for (int i = 0; i < types; i++)
		{
			for (int j = 0; j < Plot3dCamera::ecMax; j++)
			{
				plot3d[i].camera[j]->serialize(ar);
			}
			plot3d[i].params->serialize(ar);
		}
	}
	else
	{
		ar >> dwVer;

		int count = 0;
		ar >> count;
		int coords = 0;
		ar >> coords;
		for (int i = 0; i < std::min(count, (int)ep3Max); i++)
		{
			for (int j = 0; j < std::min(coords, (int)Plot3dCamera::ecMax); j++)
			{
				plot3d[i].camera[j]->serialize(ar);
			}
			plot3d[i].params->serialize(ar);
		}
	}
}
// End of Collection of Can2 settings implementation
///////////////////////////////////////////////////////////////////////////////////////////////////
