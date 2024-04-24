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

#include "Gnss.h"

using namespace can2;

Gnss::SignalProps Gnss::c_sigs[Gnss::esigInvalid] =
{
	{ Gnss::G01, 1575.420e6, Gnss::GPS, "G01", "GPS L1 1575.42" },
	{ Gnss::G02, 1227.600e6, Gnss::GPS, "G02", "GPS L2 1227.42" },
	{ Gnss::G05, 1176.450e6, Gnss::GPS, "G05", "GPS L5 1176.45" },

	{ Gnss::R01, 1602.000e6, Gnss::GLO, "R01", "GLO L1 1602.00" },
	{ Gnss::R02, 1246.000e6, Gnss::GLO, "R02", "GLO L2 1246.00" },
	{ Gnss::R03, 1202.025e6, Gnss::GLO, "R03", "GLO L2 1202.025" },
	{ Gnss::R04, 1600.995e6, Gnss::GLO, "R03", "GLO L1 1600.995" },
	{ Gnss::R06, 1248.060e6, Gnss::GLO, "R06", "GLO L1 1248.060" },

	{ Gnss::E01, 1575.420e6, Gnss::GAL, "E01", "GAL E1 1575.42" },
	{ Gnss::E05, 1176.450e6, Gnss::GAL, "E05", "GAL E5a 1176.45" },
	{ Gnss::E07, 1207.140e6, Gnss::GAL, "E07", "GAL E5b 1207.14" },
	{ Gnss::E08, 1191.795e6, Gnss::GAL, "E08", "GAL E5ab 1191.795" },
	{ Gnss::E06, 1278.750e6, Gnss::GAL, "E06", "GAL E6 1278.75" },

	{ Gnss::C01, 1575.420e6, Gnss::BDS, "C01", "BDS B1C 1575.42" },
	{ Gnss::C02, 1561.098e6, Gnss::BDS, "C02", "BDS B1I 1561.098" },
	{ Gnss::C05, 1176.450e6, Gnss::BDS, "C05", "BDS B2a 1176.450" },
	{ Gnss::C06, 1268.520e6, Gnss::BDS, "C06", "BDS B3 1268.52" },
	{ Gnss::C07, 1207.140e6, Gnss::BDS, "C07", "BDS B2b 1207.14" },
	{ Gnss::C08, 1191.795e6, Gnss::BDS, "C08", "BDS B2ab 1191.795" },

	{ Gnss::J01, 1575.420e6, Gnss::QZSS, "J01", "QZSS L1 1575.42" },
	{ Gnss::J02, 1227.600e6, Gnss::QZSS, "J02", "QZSS L2 1227.6" },
	{ Gnss::J05, 1176.450e6, Gnss::QZSS, "J05", "QZSS L5 1176.450" },
	{ Gnss::J06, 1278.750e6, Gnss::QZSS, "J06", "QZSS LEX 1278.750" },

	{ Gnss::S01, 1575.42e6, Gnss::SBAS, "S01", "SBAS L1 1575.42" },
	{ Gnss::S05, 1176.45e6, Gnss::SBAS, "S05", "SBAS L5 1176.45" },

	{ Gnss::I05, 1176.450e6, Gnss::IRN, "I05", "IRN 1176.450" }

};
