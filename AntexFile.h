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


#include "Node.h"
#include "AntexAntenna.h"


namespace can2
{
	class AntexAntenna;

	////////////////////////////////////////////////////////
	// AntexFile interface
	class AntexFile : public Node
	{
	// Construction:
	public:
		AntexFile() {
			m_ent = entAntexFile;
		}
		virtual ~AntexFile() {
			m_ants.clear();
		}

		static bool create(const char* dest, std::vector<AntexAntenna*>& as);
		static std::shared_ptr<AntexFile> load(const char* source);


	// Attributes:
	public:
		std::string m_sourceFile;
		std::string m_version;
		std::string m_system;
		std::string m_pcvType;
		std::string m_refAnt;
		std::string m_refSerial;

		std::vector<std::shared_ptr<AntexAntenna>> m_ants;

		static std::string getShortPathName(const char* source, int maxSize=40);
		std::string getShortPathName() const {
			return getShortPathName(m_sourceFile.c_str());
		}

	// Overrides:
	public:
		virtual std::string getName() const { return m_sourceFile; }
		virtual int childs() const { return m_ants.size(); }
		virtual Node* getChild(int index) const { return (can2::Node *)m_ants.at(index).get(); }

	// Implementation:
	protected:
		static void beginHeader(std::ofstream& ofs);
		static void endHeader(std::ofstream& ofs);
		static void comment(std::ofstream& ofs, const char * szComment);
		static void beginAntenna(std::ofstream& ofs, const char* szTypeName, const char* szDome, const char* szSn, int nAnts, const char* szCal, double da, double ele1, double ele2, double de, int nFreqs);
		static void beginFreq(std::ofstream& ofs, Gnss::Signal sig, bool bGdv = false);
		static void northEastUp(std::ofstream& ofs, const Point3d& ptEnu);
		static void noazi(std::ofstream& ofs, const AntexAntenna::SignalData& s, const AntexAntenna::Grid& grid);
		static void azi(std::ofstream& ofs, double a, const double* pds, int nSize);
		static void endFreq(std::ofstream& ofs, Gnss::Signal sig, bool bGdv);
		static void beginFreqRms(std::ofstream& ofs, Gnss::Signal sig, bool bGdv);
		static void noaziRms(std::ofstream& ofs, const double* pds, int nSize);
		static void aziRms(std::ofstream& ofs, double a, const double* pds, int nSize);
		static void endFreqRms(std::ofstream& ofs, Gnss::Signal sig, bool bGdv);
		static void endAntenna(std::ofstream& ofs);

	};
	// End of AntexFile interface
	///////////////////////////////////////////////////////////
}