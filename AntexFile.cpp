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

#include "Tools.h"
#include "TreeParser.h"
#include "Can2Exception.h"
#include "Points.h"
#include "AntexFile.h"
#include "AntexAntenna.h"
#include "RingNode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

using namespace can2;

////////////////////////////////////////////////////////////////////////////////////////////////
// AntexFile implementation

bool AntexFile::create(const char* dest, std::vector<AntexAntenna*>& as)
{
	std::ofstream ofs(dest);
	if (!ofs.good())
		return false;

	beginHeader(ofs);
	endHeader(ofs);

	for (int na = 0; na < (int)as.size(); na++)
	{
		AntexAntenna& a = *as[na];

		int nAnts = a.isRingAntenna() ? atoi(((RingAntenna*)&a)->m_antNum.c_str()) : 1;

		beginAntenna(ofs, a.m_type.c_str(), a.m_radome.c_str(), a.m_serial.c_str(), nAnts, a.m_date.c_str(), 
			a.m_grid.step.az, a.m_grid.za0.zen, a.m_grid.za1.zen, a.m_grid.step.zen, a.getFreqNum());

		comment(ofs, a.m_comment.c_str());

		for (auto it = a.m_sigs.begin(); it != a.m_sigs.end(); it++)
		{
			Gnss::Signal sig = it->first;
			const AntexAntenna::SignalData& s = it->second;

			beginFreq(ofs, sig, false);
			{

				northEastUp(ofs, s.pco);

				noazi(ofs, s, a.m_grid);

				for (double az = a.m_grid.za0.az; az <= a.m_grid.za1.az; az += a.m_grid.step.az)
				{
					std::vector<double> pcv;
					pcv.resize(a.m_grid.nz);
					int nz = 0;
					for (double zen = a.m_grid.za0.zen; zen <= a.m_grid.za1.zen; zen += a.m_grid.step.zen)
					{
						pcv[nz++] = s.getPcv(zen, az);
					}
					azi(ofs, az, pcv.data(), pcv.size());
				}
			}
			endFreq(ofs, sig, false);
		}
		endAntenna(ofs);
	}

	return true;

}


std::shared_ptr<AntexFile> AntexFile::load(const char* source)
{
	can2::TreeParser tp;

	enum AntexKey
	{
		eakInvalid = 0,
		eakAntexVersion = 1,
		eakPcvType = 2,
		eakComment = 3,
		eakEndOfHeader = 4,
		eakStartOfAntenna = 5,
		eakTypeSerNum = 6,
		eakMethByDate = 7,
		eakDazi = 8,
		eakZenDzen = 9,
		eakNumOfFreq = 10,
		eakSinexCode = 11,
		eakStartOfFreq = 12,
		eakNorthEastUp = 13,
		eakEndOfFreq = 14,
		eakStartOfFreqRms = 15,
		eakEndOfFreqRms = 16,
		eakEndOfAntenna = 17
	};


	tp.addKey("ANTEX VERSION / SYST", eakAntexVersion);
	tp.addKey("PCV TYPE / REFANT", eakPcvType);
	tp.addKey("COMMENT", eakComment);
	tp.addKey("END OF HEADER", eakEndOfHeader);
	tp.addKey("START OF ANTENNA", eakStartOfAntenna);
	tp.addKey("TYPE / SERIAL NO", eakTypeSerNum);
	tp.addKey("METH / BY / # / DATE", eakMethByDate);
	tp.addKey("DAZI", eakDazi);
	tp.addKey("ZEN1 / ZEN2 / DZEN", eakZenDzen);
	tp.addKey("# OF FREQUENCIES", eakNumOfFreq);
	tp.addKey("SINEX CODE", eakSinexCode);
	tp.addKey("START OF FREQUENCY", eakStartOfFreq);
	tp.addKey("NORTH / EAST / UP", eakNorthEastUp);
	tp.addKey("END OF FREQUENCY", eakEndOfFreq);
	tp.addKey("START OF FREQ RMS", eakStartOfFreqRms);
	tp.addKey("END OF FREQ RMS", eakEndOfFreqRms);
	tp.addKey("END OF ANTENNA", eakEndOfAntenna);

	auto parseSignalCode = [](const std::string_view& code) {
		for (int i = 0; i < Gnss::esigInvalid; i++) {
			if (code == Gnss::c_sigs[i].code)
				return (Gnss::Signal)i;
		}
		return Gnss::esigInvalid;
	};
	auto parseNorthEastUp = [](const std::string_view& neu) {
		return Point3d(std::strtod(neu.substr(10, 10).data(), nullptr) / 1000.0,
			std::strtod(neu.substr(0, 10).data(), nullptr) / 1000.0,
			std::strtod(neu.substr(20, 10).data(), nullptr) / 1000.0);
	};
	auto parseZenDzen = [](const std::string_view& zdz, AntexAntenna::Grid& grid) {
		grid.za0.zen = std::strtod(zdz.substr(2, 6).data(), nullptr);
		grid.za1.zen = std::strtod(zdz.substr(8, 6).data(), nullptr);
		grid.step.zen = std::strtod(zdz.substr(14, 6).data(), nullptr);
		if (_isnan(grid.step.zen) || _isnan(grid.za0.zen) || _isnan(grid.za0.zen))
			return false;
		if (grid.step.zen <= 0)
			return false;
		if (grid.za0.zen >= grid.za1.zen)
			return false;
		grid.nz = (int)floor((grid.za1.zen - grid.za0.zen) / grid.step.zen + 0.5) + 1;
		return true;
	};
	auto parseDazi = [](const std::string_view& dazi, AntexAntenna::Grid& grid) {
		grid.step.az = std::strtod(dazi.substr(2, 6).data(), nullptr);
		if (_isnan(grid.step.az) || grid.step.az < 0)
			return false;
		grid.za0.az = 0;
		grid.za1.az = 360;
		grid.na = (int)floor(360 / grid.step.az + 0.5) + 1;
		return true;
	};
	auto parseNoazi = [](const std::string_view& noazi, AntexAntenna::SignalData& s) {
		if (noazi.substr(3, 5) != "NOAZI")
			return false;
		if ((int)noazi.size() < 8 + 8 * s.ant->m_grid.nz)
			return false;
		s.naPcv.resize(s.ant->m_grid.nz);
		for (int i = 0; i < s.ant->m_grid.nz; i++) {
			// The PCV in ANTEX are stored with '-' sign. We change it here, 
			// so PCV in AntexAntenna::SignalData are with correct sign. They are in meters also
			s.naPcv[i] = -std::strtod(noazi.substr(8 + 8 * i, 8).data(), nullptr) / 1000.0;
		}
		return true;
	};
	auto parseNoaziRms = [](const std::string_view& noazi, AntexAntenna::SignalData& s) {
		if (noazi.substr(3, 5) != "NOAZI")
			return false;
		if ((int)noazi.size() < 8 + 8 * s.ant->m_grid.nz)
			return false;
		s.naRms.resize(s.ant->m_grid.nz);
		for (int i = 0; i < s.ant->m_grid.nz; i++) {
			s.naRms[i] = std::strtod(noazi.substr(8 + 8 * i, 8).data(), nullptr) / 1000.0;
		}
		return true;
	};
	auto parsePcv = [](const std::string_view& pcv, int na, AntexAntenna::SignalData& s) {

		double az = std::strtod(pcv.substr(0, 8).data(), nullptr);
		if (az != na * s.ant->m_grid.step.az)
			return false;
		for (int nz = 0; nz < s.ant->m_grid.nz; nz++) {
			// The PCV in ANTEX are stored with '-' sign. We change it here, 
			// so PCV in AntexAntenna::SignalData are with correct sign. They are in meters also
			s.aPcv[na * s.ant->m_grid.nz + nz] = -std::strtod(pcv.substr(8 + 8 * nz, 8).data(), nullptr) / 1000.0;
		}
		return true;
	};
	auto parseRms = [](const std::string_view& rms, int na, AntexAntenna::SignalData& s) {

		double az = std::strtod(rms.substr(0, 8).data(), nullptr);
		if (az != na * s.ant->m_grid.step.az)
			return false;
		for (int nz = 0; nz < s.ant->m_grid.nz; nz++) {
			s.aRms[na * s.ant->m_grid.nz + nz] = std::strtod(rms.substr(8 + 8 * nz, 8).data(), nullptr) / 1000.0;
		}
		return true;
	};
	auto parseStringField = [](std::string& s, int pos, int len) {
		std::string str = s.substr(pos, len);
		ltrim(str);
		rtrim(str);
		return str;
	};

	std::ifstream ifs(source);
	if (!ifs.good())
		return nullptr;

	std::string line;
	std::shared_ptr<AntexFile> af (new AntexFile());
	af->m_sourceFile = source;
	std::shared_ptr<AntexAntenna> aa(nullptr);
	enum State
	{
		esNone = 0,
		esHeader = 1,
		esAntennaHeader = 2,
		esAntFrequency = 3,
		esAntFrequencyRms = 4
	} state = esNone;
	int lastKey = 0;
	int lineNum = 0;
	Gnss::Signal eSignal = Gnss::esigInvalid;
	while (std::getline(ifs, line))
	{
		lineNum++;
		int key = tp.find(line.c_str() + std::min(60, (int)line.length()));
		if (state == esNone)
		{
			if (key == eakAntexVersion)
			{
				state = esHeader;
				af->m_version = parseStringField(line, 0, 8);
				af->m_system = line.substr(20, 1);
			}
			else if (key == eakStartOfAntenna)
			{
				state = esAntennaHeader;
				aa = std::shared_ptr<AntexAntenna>(new AntexAntenna());
				aa->m_parent = af.get();
				af->m_ants.push_back(aa);
			}
			continue;
		}
		else if (state == esHeader)
		{
			if (key == eakPcvType)
			{
				af->m_pcvType = line.substr(0, 1);
				af->m_refAnt = parseStringField(line, 20, 20);
				af->m_refSerial = parseStringField(line, 40, 20);
			}
			else if (key == eakComment)
			{
			}
			else if (key == eakEndOfHeader)
			{
				state = esNone;
			}
		}
		else if (state == esAntennaHeader)
		{
			//assert(aa != nullptr);
			if (aa == nullptr)
				throw AntexException("Unexpected antenna header", lineNum);
			if (key == eakTypeSerNum)
			{
				aa->m_type = parseStringField (line, 0, 15); 
				aa->m_radome = parseStringField( line, 16, 4);
				aa->m_serial = parseStringField(line, 20, 20);
			}
			else if (key == eakMethByDate)
			{
				aa->m_method = parseStringField(line, 0, 20);
				aa->m_agency = parseStringField(line, 20, 20);
				aa->m_numIndAnt = parseStringField(line, 40, 6);
				aa->m_date = parseStringField(line, 50, 10);
			}
			else if (key == eakZenDzen)
			{
				std::string_view zdz(line.c_str());
				if (!parseZenDzen(zdz, aa->m_grid))
					throw AntexException("Can't parse ZEN1 / ZEN2 / DZEN", lineNum);
			}
			else if (key == eakDazi)
			{
				std::string_view zdz(line.c_str());
				if (!parseDazi(zdz, aa->m_grid))
					throw AntexException("Can't parse Dazi", lineNum);
			}
			else if (key == eakStartOfFreq)
			{
				if (!aa->m_grid.isValid())
					throw AntexException("Grid is not defined", lineNum);

				std::string_view sig(line.c_str() + 3, 3);
				eSignal = parseSignalCode(sig);
				if (eSignal == Gnss::esigInvalid)
				{
					std::cerr << "Unknown signal " << sig << " in line " << lineNum << std::endl;
					state = esNone;
					continue;
				}
				state = esAntFrequency;
				AntexAntenna::SignalData s;
				s.ant = aa.get();
				s.es = eSignal;

				while (std::getline(ifs, line))
				{
					key = tp.find(line.c_str() + std::min(60, (int)line.length()));
					if (key == eakNorthEastUp)
					{
						std::string_view neu(line.c_str());
						s.pco = parseNorthEastUp(neu);
					}
					else
					{
						throw AntexException("NORTH / EAST / UP not found", lineNum);
					}
					lineNum++;
					break;
				}

				if (!std::getline(ifs, line))
					throw AntexException("Unexpected EOF", lineNum);
				if (!parseNoazi(std::string_view(line.c_str()), s))
					throw AntexException("Can't parse NOAZI PCV", lineNum);
				lineNum++;

				if (aa->m_grid.na > 0)
				{
					s.aPcv.resize(aa->m_grid.na * aa->m_grid.nz, NAN);
					for (int na = 0; na<aa->m_grid.na; na++)
					{
						if (!std::getline(ifs, line))
							throw AntexException("Unexpected EOF", lineNum);
						std::string_view pcv(line.c_str());
						
						if (!parsePcv(pcv, na, s))
							throw AntexException("Can't parse PCV", lineNum);

						lineNum++;
					}
				}

				if (!std::getline(ifs, line))
					throw AntexException("Unexpected EOF", lineNum);
				key = tp.find(line.c_str() + std::min(60, (int)line.length()));
				if (key != eakEndOfFreq)
					throw AntexException("END OF FREQUENCY not found", lineNum);
				state = esAntennaHeader;
				aa->m_sigs[s.es] = s;
				lineNum++;
			}
			else if (key == eakEndOfAntenna)
			{
				aa = nullptr;
				state = esNone;
			}
			else if (key == eakStartOfFreqRms)
			{
				if (!aa->m_grid.isValid())
					throw AntexException("Grid is not defined", lineNum);

				std::string_view sig(line.c_str() + 3, 3);
				eSignal = parseSignalCode(sig);
				if (eSignal == Gnss::esigInvalid)
				{
					std::cerr << "Unknown signal " << sig << " in line " << lineNum << std::endl;
					state = esNone;
					continue;
				}
				if (aa->m_sigs.find(eSignal) == aa->m_sigs.end())
				{
					throw AntexException("RMS is defined before PCV", lineNum);
				}
				state = esAntFrequencyRms;
				AntexAntenna::SignalData& s = aa->m_sigs[eSignal];

				if (aa->m_grid.na > 0)
				{
					s.aRms.resize(aa->m_grid.na * aa->m_grid.nz, NAN);
					for (int na = 0; na < aa->m_grid.na; na++)
					{
						if (!std::getline(ifs, line))
							throw AntexException("Unexpected EOF", lineNum);
						std::string_view rms(line.c_str());

						key = tp.find(line.c_str() + std::min(60, (int)line.length()));
						if (key == eakNorthEastUp)
						{
							std::string_view neu(line.c_str());
							s.pcoRms = parseNorthEastUp(neu);
							lineNum++;
							na--;
							continue;
						}
						else if (key == eakComment)
						{
							lineNum++;
							na--;
							continue;
						}

						if (rms.substr(3, 5) == "NOAZI")
						{
							if (!parseNoaziRms(rms, s))
								throw AntexException("Can't parse NOAZI RMS", lineNum);
							na--;
						}
						else
						{
							if (!parseRms(rms, na, s))
								throw AntexException("Can't parse RMS", lineNum);
						}

						lineNum++;
					}
				}

				if (!std::getline(ifs, line))
					throw AntexException("Unexpected EOF", lineNum);
				key = tp.find(line.c_str() + std::min(60, (int)line.length()));
				if (key != eakEndOfFreqRms)
					throw AntexException("END OF FREQ RMS not found", lineNum);
				state = esAntennaHeader;
				lineNum++;
			}
		}
	}

	for (auto ita = af->m_ants.begin(); ita != af->m_ants.end(); ita++)
	{
		(*ita)->normalizePcv();
	}

	return af;
}



std::string AntexFile::getShortPathName(const char * sourcePath, int maxSize)
{
	//std::filesystem::path source(sourcePath);
	//std::string name = source.stem().u8string();
	//std::string path = source.parent_path().u8string();
	//return path.substr(std::max((int)path.length() - 20, 0), std::min(20, (int)path.length())) + name;
	std::string	str = sourcePath;
	return str.substr(std::max((int)str.length() - maxSize, 0), std::min(maxSize, (int)str.length()));
}


/////////////////////////////////////////////////
// Implementation:
void AntexFile::beginHeader(std::ofstream& ofs)
{
	static char cSatSys[] = { 'G', 'R', 'E', 'C', 'J', 'S', 'M' };
	static CHAR cPcvType[] = { 'A', 'R' };
	LPCTSTR szRefAntType = "";
	LPCTSTR szRefAntSn = "";

	CString str;
	str.Format("%8.1f%*c%c%*cANTEX VERSION / SYST\n", 1.4, 12, ' ', cSatSys[6], 39, ' ');
	ofs << str;

	str.Format("%c%*c%20.20s%20.20sPCV TYPE / REFANT   \n", cPcvType[0], 19, ' ', szRefAntType, szRefAntSn);
	ofs << str;
}

void AntexFile::endHeader(std::ofstream& ofs)
{
	CString str;
	str.Format("%*cEND OF HEADER       \n", 60, ' ');
	ofs << str;
}

void AntexFile::comment(std::ofstream& ofs, const char* szComment)
{
	//CString str;
	//str.Format("%60.60sCOMMENT\n", szComment);
	//m_f.WriteString(str);
	LPCTSTR p = szComment;
	while (*p)
	{
		p += strspn(p, "\r\n");
		int nLen = strcspn(p, "\r\n");
		if (nLen <= 0)
			break;

		CString strCom(p, std::min(nLen, 80));
		p += nLen;

		CString str;
		str.Format("%-60.60sCOMMENT             \n", (LPCTSTR)strCom);
		ofs << str;
	}
}

void AntexFile::beginAntenna(std::ofstream& ofs, const char* szTypeName, const char* szDome, const char* szSn, int nAnts, const char* szCal, double da, double ele1, double ele2, double de, int nFreqs)
{
	CString str;
	str.Format("%*cSTART OF ANTENNA    \n", 60, ' ');
	ofs << str;

	CHAR szTypeDome[21];
	{
		memset(szTypeDome, ' ', 20);
		LPCTSTR szd = szDome != NULL && strlen(szDome) != 0 ? szDome : "NONE";
		int nDomeLen = strlen(szd);
		for (int i = 0; i < std::min(nDomeLen, 20); i++)
		{
			szTypeDome[19 - i] = szd[nDomeLen - 1 - i];
		}

		LPCTSTR szt = szTypeName != NULL ? szTypeName : "NONE";
		int nTypeLen = strlen(szt);
		for (int i = 0; i < std::min(nTypeLen, 20); i++)
		{
			szTypeDome[i] = szt[i];
		}
		szTypeDome[20] = '\0';
	}

	str.Format("%20.20s%20.20s%*cTYPE / SERIAL NO    \n", szTypeDome, szSn != NULL ? szSn : "", 20, ' ');
	ofs << str;

	CString strCal = szCal;
	strCal.MakeUpper();
	str.Format("%-20.20s%-20.20s%6i    %9.9s METH / BY / # / DATE\n", "ROBOT", "TOPCON", nAnts, (LPCTSTR)strCal);
	ofs << str;

	str.Format("  %6.1f%*cDAZI                \n", da, 52, ' ');
	ofs << str;

	str.Format("  %6.1f%6.1f%6.1f%*cZEN1 / ZEN2 / DZEN  \n", ele1, ele2, de, 40, ' ');
	ofs << str;

	str.Format("%6i%*c# OF FREQUENCIES    \n", nFreqs, 54, ' ');
	ofs << str;

}

void AntexFile::beginFreq(std::ofstream& ofs, Gnss::Signal sig, bool bGdv)
{
	//LPCTSTR szSys = nSys == 0 ? "G" : "R";
	//LPCTSTR szFreq = nSlot == 2 ? "02" : "01";
	CString str;
	LPCTSTR szCode = Gnss::getSignalCode(sig);
	if (!bGdv)
		str.Format("   %3.3s%*cSTART OF FREQUENCY  \n", szCode, 54, ' ');
	else
		str.Format("   %3.3s CPV%*cSTART OF FREQUENCY  \n", szCode, 50, ' ');
	ofs << str;
}

void AntexFile::northEastUp(std::ofstream& ofs, const Point3d& ptEnu)
{
	CString str;
	str.Format("%+10.2f%+10.2f%+10.2f%*cNORTH / EAST / UP   \n", ptEnu.n * 1000, ptEnu.e * 1000, ptEnu.u * 1000, 30, ' ');
	ofs << str;
}

void AntexFile::noazi(std::ofstream& ofs, const AntexAntenna::SignalData& sig, const AntexAntenna::Grid& grid)
{
	CString str = "   NOAZI";
	if ((int)sig.naPcv.size() < grid.nz || _isnan(sig.pcv(0)))
	{
		std::vector<double> pcv;
		for (int nz = 0; nz < grid.nz; nz++)
		{
			double sum = 0;
			for (int na = 0; na < grid.na-1; na++)
			{
				sum += sig.pcv(nz, na);
			}
			sum /= (grid.na - 1);
			pcv.push_back(sum);
		}

		for (int i = 0; i < (int)pcv.size(); i++)
		{
			CString s;
			s.Format("%+8.2f", -1000 * pcv[i]);
			str += s;
		}
		str += "\n";
		ofs << str;
	}
	else
	{
		for (int i = 0; i < (int)grid.nz; i++)
		{
			CString s;
			s.Format("%+8.2f", -1000 * sig.pcv(i));
			str += s;
		}
		str += "\n";
		//str.Format(str);
		ofs << str;
	}
}

void AntexFile::azi(std::ofstream& ofs, double a, const double* pds, int nSize)
{
	CString str;
	str.Format("%8.1f", a);
	for (int i = 0; i < nSize; i++)
	{
		CString s;
		s.Format("%+8.2f", -1000*pds[i]);
		str += s;
	}
	str += "\n";
	str.Format(str);
	ofs << str;
}

void AntexFile::endFreq(std::ofstream& ofs, Gnss::Signal sig, bool bGdv)
{
	//LPCTSTR szSys = nSys == 0 ? "G" : "R";
	//LPCTSTR szFreq = nSlot == 2 ? "02" : "01";
	//str.Format("   %1.1s%2.2s%*cEND OF FREQUENCY\n", szSys, szFreq, 54, ' ');
	LPCTSTR szCode = Gnss::getSignalCode(sig);
	CString str;
	if (!bGdv)
		str.Format("   %3.3s%*cEND OF FREQUENCY    \n", szCode, 54, ' ');
	else
		str.Format("   %3.3s CPV%*cEND OF FREQUENCY    \n", szCode, 50, ' ');
	ofs << str;
}

void AntexFile::beginFreqRms(std::ofstream& ofs, Gnss::Signal sig, bool bGdv)
{
	//LPCTSTR szSys = nSys == 0 ? "G" : "R";
	//LPCTSTR szFreq = nSlot == 2 ? "02" : "01";
	LPCTSTR szCode = Gnss::getSignalCode(sig);
	CString str;
	if (!bGdv)
		str.Format("   %3.3s%*cSTART OF FREQ RMS   \n", szCode, 54, ' ');
	else
		str.Format("   %3.3s CPV%*cSTART OF FREQ RMS   \n", szCode, 50, ' ');
	ofs << str;
}

void AntexFile::noaziRms(std::ofstream& ofs, const double* pds, int nSize)
{
	CString str = "   NOAZI";
	for (int i = 0; i < nSize; i++)
	{
		CString s;
		s.Format("%8.2f", 1000*pds[i]);
		str += s;
	}
	str += "\n";
	//str.Format(str);
	ofs << str;
}

void AntexFile::aziRms(std::ofstream& ofs, double a, const double* pds, int nSize)
{
	CString str;
	str.Format("%8.1f", a);
	for (int i = 0; i < nSize; i++)
	{
		CString s;
		s.Format("%8.2f", 1000*pds[i]);
		str += s;
	}
	str += "\n";
	//str.Format(str);
	ofs << str;
}


void AntexFile::endFreqRms(std::ofstream& ofs, Gnss::Signal sig, bool bGdv)
{
	//LPCTSTR szSys = nSys == 0 ? "G" : "R";
	//LPCTSTR szFreq = nSlot == 2 ? "02" : "01";
	//CString str;
	//str.Format("   %1.1s%2.2s%*cEND OF FREQ RMS\n", szSys, szFreq, 54, ' ');
	LPCTSTR szCode = Gnss::getSignalCode(sig);
	CString str;
	if (!bGdv)
		str.Format("   %3.3s%*cEND OF FREQ RMS     \n", szCode, 54, ' ');
	else
		str.Format("   %3.3s CPV%*cEND OF FREQ RMS     \n", szCode, 50, ' ');
	ofs << str;
}

void AntexFile::endAntenna(std::ofstream& ofs)
{
	CString str;
	str.Format("%*cEND OF ANTENNA      \n", 60, ' ');
	ofs << str;
}

