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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

using namespace can2;

////////////////////////////////////////////////////////////////////////////////////////////////
// AntexFile implementation

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