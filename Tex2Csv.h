#pragma once

#include "RingIFView.h"

namespace can2
{
//////////////////////////////////////////////////////////////////////////////
// Tex to CSV converter
class Tex2Csv
{
// Construction:
public:
	Tex2Csv();
	~Tex2Csv();

// Operations:
public:
	static bool convert(const char * szAnt, const char* szTex, std::vector<CRingIFView::PppItem>& pppItems);
};
// End of Tex to CSV converter interface
//////////////////////////////////////////////////////////////////////////////
}