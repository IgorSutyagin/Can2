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

Gnss::SignalProps Gnss::c_sigs[Gnss::esigMax] =
{
	{ Gnss::G01, 1575.420e6, Gnss::GPS, "G01", "GPS L1 1575.42", ebL1, RGB(255, 0, 0) },
	{ Gnss::G02, 1227.600e6, Gnss::GPS, "G02", "GPS L2 1227.42", ebL2, RGB(0, 0, 255) },
	{ Gnss::G05, 1176.450e6, Gnss::GPS, "G05", "GPS L5 1176.45", ebL5, RGB(0, 255, 0) },

	{ Gnss::R01, 1602.000e6, Gnss::GLO, "R01", "GLO L1 1602.00", ebL1, RGB(150, 0, 0) },
	{ Gnss::R02, 1246.000e6, Gnss::GLO, "R02", "GLO L2 1246.00", ebL2, RGB(0, 0, 150) },
	{ Gnss::R03, 1202.025e6, Gnss::GLO, "R03", "GLO L3 1202.025", ebL5, RGB(0, 150, 0) },
	{ Gnss::R04, 1600.995e6, Gnss::GLO, "R04", "GLO L1 1600.995", ebL1, RGB(160, 0, 0) },
	{ Gnss::R06, 1248.060e6, Gnss::GLO, "R06", "GLO L1 1248.060", ebL2, RGB(0, 0, 160) },

	{ Gnss::E01, 1575.420e6, Gnss::GAL, "E01", "GAL E1 1575.42", ebL1, RGB(200, 0, 0) },
	{ Gnss::E05, 1176.450e6, Gnss::GAL, "E05", "GAL E5a 1176.45", ebL5, RGB(0, 200, 0) },
	{ Gnss::E07, 1207.140e6, Gnss::GAL, "E07", "GAL E5b 1207.14", ebL5, RGB(0, 160, 0) },
	{ Gnss::E08, 1191.795e6, Gnss::GAL, "E08", "GAL E5ab 1191.795", ebL5, RGB(0, 180, 0) },
	{ Gnss::E06, 1278.750e6, Gnss::GAL, "E06", "GAL E6 1278.75", ebB3E6, RGB (100, 0, 100) },

	{ Gnss::C01, 1575.420e6, Gnss::BDS, "C01", "BDS B1C 1575.42", ebL1, RGB(80, 0, 0) },
	{ Gnss::C02, 1561.098e6, Gnss::BDS, "C02", "BDS B1I 1561.098", ebL1, RGB(60, 0, 0) },
	{ Gnss::C05, 1176.450e6, Gnss::BDS, "C05", "BDS B2a 1176.450", ebL5, RGB(0, 80, 0) },
	{ Gnss::C06, 1268.520e6, Gnss::BDS, "C06", "BDS B3 1268.52", ebB3E6, RGB(80, 0, 80) },
	{ Gnss::C07, 1207.140e6, Gnss::BDS, "C07", "BDS B2b 1207.14", ebL5, RGB(0, 90, 0) },
	{ Gnss::C08, 1191.795e6, Gnss::BDS, "C08", "BDS B2ab 1191.795", ebL5, RGB(0, 60, 0) },

	{ Gnss::J01, 1575.420e6, Gnss::QZSS, "J01", "QZSS L1 1575.42", ebL5, RGB(220, 0, 0) },
	{ Gnss::J02, 1227.600e6, Gnss::QZSS, "J02", "QZSS L2 1227.6", ebL2, RGB(0, 0, 220) },
	{ Gnss::J05, 1176.450e6, Gnss::QZSS, "J05", "QZSS L5 1176.450", ebL5, RGB(0, 220, 0) },
	{ Gnss::J06, 1278.750e6, Gnss::QZSS, "J06", "QZSS LEX 1278.750", ebB3E6, RGB(220, 0, 220) },

	{ Gnss::S01, 1575.42e6, Gnss::SBAS, "S01", "SBAS L1 1575.42", ebL1, RGB(210, 0, 0) },
	{ Gnss::S05, 1176.45e6, Gnss::SBAS, "S05", "SBAS L5 1176.45", ebL5, RGB(0, 210, 0) },

	{ Gnss::I05, 1176.450e6, Gnss::IRN, "I05", "IRN 1176.450", ebL5, RGB(0, 180, 0) },
	{ Gnss::I09, 2492.028e6, Gnss::IRN, "I09", "IRN 2492.028", ebS, RGB(180, 0, 180) },

	{ Gnss::esigInvalid, NAN, Gnss::esysInvalid, "", "", ebMax },

	{ Gnss::GIFL2, 1575.42e6 * 1227.6e6 / (1575.42e6 + 1227.6e6), Gnss::GPS, "GIFL2", "GPS IF L1L2", ebMax, RGB(255, 0, 255) },
	{ Gnss::GIFL5, 1575.42e6 * 1176.45e6 / (1575.42e6 + 1176.45e6), Gnss::GPS, "GIFL5", "GPS IF L1L5", ebMax, RGB(200, 0, 200) },
	{ Gnss::RIFL2, 1602.0e6 * 1246.0e6 / (1602.0e6 + 1246.0e6), Gnss::GLO, "RIFL2", "GLO IF L1L2", ebMax, RGB(100, 0, 100) },
	{ Gnss::EIFL5a, 1575.42e6 * 1176.450e6 / (1575.42e6 + 1176.450e6), Gnss::GAL, "EIFL5a", "GAL IF L1L5a", ebMax, RGB(200, 200, 0) },
	{ Gnss::EIFL5ab, 1575.42e6 * 1191.795e6 / (1575.42e6 + 1191.795e6), Gnss::GAL, "EIFL5ab", "GAL IF L1L5ab", ebMax, RGB(210, 210, 0) },
	{ Gnss::EIFL5b, 1575.42e6 * 1207.140e6 / (1575.42e6 + 1207.140e6), Gnss::GAL, "EIFL5b", "GAL IF L1L5b", ebMax, RGB(220, 220, 0) },
	{ Gnss::CIF0106, 1575.42e6 * 1268.52e6 / (1575.42e6 + 1268.52e6), Gnss::BDS, "CIFC01C06", "BDS IF C01C06", ebMax, RGB(80, 80, 0) },
	{ Gnss::CIF0206, 1561.098e6 * 1268.52e6 / (1561.098e6 + 1268.52e6), Gnss::BDS, "CIFC02C06", "BDS IF C02C06", ebMax, RGB(60, 60, 0) },

	{ Gnss::GWLL2, (1575.42e6 - 1227.6e6), Gnss::GPS, "GWLL2", "GPS WL L1L2", ebMax, RGB(255, 0, 255) },
	{ Gnss::GWLL5,(1575.42e6 - 1176.45e6), Gnss::GPS, "GWLL5", "GPS WL L1L5", ebMax, RGB(200, 0, 200) },
	{ Gnss::RWLL2, (1602.0e6 - 1246.0e6), Gnss::GLO, "RWLL2", "GLO WL L1L2", ebMax, RGB(100, 0, 100) },
	{ Gnss::EWLL5a, (1575.42e6 - 1176.450e6), Gnss::GAL, "EWLL5a", "GAL WL L1L5a", ebMax, RGB(200, 200, 0) },
	{ Gnss::EWLL5ab, (1575.42e6 - 1191.795e6), Gnss::GAL, "EWLL5ab", "GAL WL L1L5ab", ebMax, RGB(210, 210, 0) },
	{ Gnss::EWLL5b, (1575.42e6 - 1207.140e6), Gnss::GAL, "EWLL5b", "GAL WL L1L5b", ebMax, RGB(220, 220, 0) },
	{ Gnss::CWL0106, (1575.42e6 - 1268.52e6), Gnss::BDS, "CWLC01C06", "BDS WL C01C06", ebMax, RGB(80, 80, 0) },
	{ Gnss::CWL0206, (1561.098e6 - 1268.52e6), Gnss::BDS, "CWLC02C06", "BDS WL C02C06", ebMax, RGB(60, 60, 0) }
};

Gnss::BandProps Gnss::c_bands[Gnss::ebMax] =
{
	{ Gnss::ebL1, "L1", c_sigs[C02].f, c_sigs[R01].f, RGB(245, 245, 245) }, // RGB(253, 219, 225) },
	{ Gnss::ebL2, "L2", c_sigs[G02].f, c_sigs[R02].f, RGB(245, 245, 245) }, //RGB(220, 223, 252) },
	{ Gnss::ebL5, "L5", c_sigs[E05].f, c_sigs[E07].f, RGB(245, 245, 245) }, //RGB(227, 252, 253) },
	{ Gnss::ebB3E6, "B3E6", c_sigs[C06].f, c_sigs[E06].f, RGB(245, 245, 245) }, //RGB(230, 253, 227) }
	{ Gnss::ebS, "S", 2000e6, 4000e6, RGB(128, 128, 128) }
};
