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

#include "Points.h"
#include "Archive.h"

namespace can2
{
	constexpr double PI = 3.141592653589793238462643383280;
	constexpr double DEG2RAD = PI / 180.0;

	//
	// ZenAz - zenith and azimuth angles in spherical coord system
	// 
	// Zenith angle is in [0, 180], counting from zenith direction
	// Azimuth angle is in [0, 360], counting clockwise from Y axis of the corresponding Decart coords
	//
	class ZenAz
	{
	public:
		ZenAz() : zen(0), az(0) {}
		ZenAz(const ZenAz& a) {
			*this = a;
		}
		ZenAz(double zen_, double az_) : zen(zen_), az(az_) {}

	public:
		double zen; // Zenith angle (deg)
		double az;	// Azimuth angle (deg)

	public:
		ZenAz& operator=(const ZenAz& a) {
			zen = a.zen;
			az = a.az;
			return *this;
		}

		// Direction cosines
		Point3d dirCos() const {
			double sinz = ::sin(zen * DEG2RAD);
			return Point3d(sinz * ::sin(az * DEG2RAD), sinz * ::cos(az * DEG2RAD), ::cos(zen * DEG2RAD));
		}

		friend Archive& operator<<(Archive& ar, const ZenAz& za) {
			ar << za.zen;
			ar << za.az;
			return ar;
		}
		friend Archive& operator>>(Archive& ar, ZenAz& za) {
			ar >> za.zen;
			ar >> za.az;
			return ar;
		}
	};
}
