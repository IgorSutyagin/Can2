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

namespace can2
{
	class Gnss
	{
	public:

		enum System
		{
			GPS = 0,
			GLO = 1,
			GAL = 2,
			BDS = 3,
			QZSS = 4,
			SBAS = 5,
			IRN = 6,
			esysInvalid
		};

		// Signal
		enum Signal
		{
			G01 = 0,// L1 GPS 1575.42
			G02,	// L2 GPS 1227.6
			G05,	// L5 GPS 1176.45

			R01,	// L1 GLONASS 1602
			R02,	// L2 GLONASS 1246
			R03,	// GLO 1202.025
			R04,	// GLO 1600.995
			R06,	// GLO 1248.06

			E01,	// E1  Galileo  1575.42
			E05,	// E5a Galileo  1176.45
			E07,	// E5b Galileo 1207.14
			E08,	// E5 (E5a+E5b) Galileo 1191.795
			E06,	// E6 Galileo 1278.75

			C01,	// B1C 1575.42	 // IVS 2020.08.29 Added BeiDou (Compass) and other systems
			C02,	// B1I 1561.098
			C05,	// 1176.45
			C06,	// B3 1268.52
			C07,	// B2 1207.14
			C08,	// 1191.795

			J01,	// L1 1575.42
			J02,	// L2 1227.6
			J05,	// L5 1176.45
			J06,	// LEX 1278.75

			S01,	// L1 1575.42
			S05,	// L5 1176.45

			I05,	// 1176.45
			esigInvalid
		};

		struct SignalProps
		{
			Signal es;	// Signal
			double f;	// Frequency [MHz]
			System sys; // System (GPS, GLO, GAL, BDS, ...)
			const char* code; // ANTEX code
			const char* name; // Full name
		};
		static SignalProps c_sigs[esigInvalid];
		static constexpr double C = 299792458.;

		static const char* getSignalName(int es) {
			if (G01 <= es && es < esigInvalid)
				return c_sigs[es].name;
			return "";
		}

		static double getSysWl(int es) {
			if (G01 <= es && es < esigInvalid)
				return C / c_sigs[es].f;
			return NAN;
		}

		static double getSysFreq(int es) {
			if (G01 <= es && es < esigInvalid)
				return c_sigs[es].f;
			return NAN;
		}

		static System getSystem(int es) {
			if (G01 <= es && es < esigInvalid)
				return c_sigs[es].sys;
			return esysInvalid;
		}
	};
}
