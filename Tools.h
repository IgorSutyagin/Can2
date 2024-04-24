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

#include "Colors.h"

namespace can2
{

	template<typename ... Args>
	std::string stringFormat(const std::string& format, Args ... args) {
		int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
		if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
		auto size = static_cast<size_t>(size_s);
		std::unique_ptr<char[]> buf(new char[size]);
		std::snprintf(buf.get(), size, format.c_str(), args ...);
		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}

	extern const char* whiteSpace;

	// trim from end of string (right)
	inline std::string& rtrim(std::string& s, const char* t = whiteSpace)
	{
		s.erase(s.find_last_not_of(t) + 1);
		return s;
	}

	// trim from beginning of string (left)
	inline std::string& ltrim(std::string& s, const char* t = whiteSpace)
	{
		s.erase(0, s.find_first_not_of(t));
		return s;
	}

	// trim from both ends of string (right then left)
	inline std::string& trim(std::string& s, const char* t = whiteSpace)
	{
		return ltrim(rtrim(s, t), t);
	}

	// Get lowercase version of the string for standart ASCII characters
	inline std::string tolower(const char* source)
	{
		std::string r = source;
		for (auto & c : r)
		{
			c = ::tolower(c);
		}
		return r;
	}

	// Get Alpha (opacity) value from argb
	inline byte getAValue(DWORD dwArgb)
	{
		byte c = ((byte)((dwArgb) >> 24));
		return c;
	}

	// RGB to/from Humidity, Luma, Saturation conversion
	//void  rgb2hls(DWORD lRGBColor, unsigned short& H, unsigned short& L, unsigned short& S);
	//DWORD hls2rgb(unsigned short hue, unsigned short lum, unsigned short sat);

	CSize calcDialogSize(UINT nResourceId, HINSTANCE hInstance);

	struct ScreenParams
	{
		ScreenParams() {
			ppmm = NAN;
		}

		double ppmm; // Points in mm
	};

	inline CTime getDate(CTime t) {
		return CTime(t.GetYear(), t.GetMonth(), t.GetDay(), 0, 0, 0);
	}

	class CTimeEx : public CTime
	{
	// Construction:
	public:
		CTimeEx() {}
		CTimeEx(time_t t_) : CTime(t_) {}
		CTimeEx(const CTime& t_) : CTime(t_) {}

		enum TimeRegion
		{
			etrDef = 0,
			etrGmt = 1
		};

	// Operations:
	public:
		int getYearGmt() const;
		int getMonthGmt() const;
		int getDayGmt() const;
		int getHourGmt() const;
	};
}


