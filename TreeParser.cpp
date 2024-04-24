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
////////////////////////////////////////////////////////////////////////////////
// TreeParser.cpp - implementation of a binary tree parser

#include "pch.h"
#include "TreeParser.h"

using namespace can2;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TreeParser implementation

void TreeParser::TpLetter::create(const char* pKey, int nKey)
{
	if (*pKey == 0)
		return;

	if (c == *pKey || c == 0)
	{
		c = *pKey;
		pKey++;
		if (*pKey == 0)
		{
			key = nKey;
			return;
		}
	}

	if (*pKey < c)
	{
		if (left == nullptr)
			left = new TpLetter();
		left->create(pKey, nKey);
	}
	else
	{
		if (right == nullptr)
			right = new TpLetter();
		right->create(pKey, nKey);
	}
}

bool TreeParser::TpLetter::compare(const char* p, int& keyFound)
{
	if (p == nullptr || *p == 0)
		return false;

	if (*p == c)
	{
		p++;
		if (*p < c && left != nullptr)
		{
			if (left->compare(p, keyFound))
				return true;
		}
		if (*p >= c && right != nullptr)
		{
			if (right->compare(p, keyFound))
				return true;
		}

		if (key != 0)
		{
			keyFound = key;
			return true;
		}
	}
	else if (*p < c && left != nullptr)
	{
		return left->compare(p, keyFound);
	}
	else if (*p > c && right != nullptr)
	{
		return right->compare(p, keyFound);
	}

	return false;
}

// End of TpLetter implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
