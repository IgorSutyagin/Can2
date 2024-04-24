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
/////////////////////////////////////////////////////////////////////////////////////////
// TreeParser.h - interface to text files parser which uses 
// a binary tree of key values to increase parsing speed
//

#pragma once

namespace can2
{

	class TreeParser
	{
	// Construction:
	public:
		TreeParser() {
			root = new TpLetter();
		}
		virtual ~TreeParser() {
			delete root;
		}

	// Operations:
	public:
		void addKey(const char* key, int nKey) {
			root->create(key, nKey);
		}

		int find(const char* s, int * pos=nullptr) {
			int keyFound = 0;
			if (pos != nullptr)
				*pos = -1;
			for (int i = 0; s[i]; i++) {
				if (root->compare(s + i, keyFound))
				{
					if (pos != nullptr)
						*pos = i;
					return keyFound;
				}
			}
			return 0;
		}

	// Implementation:
	protected:
		class TpLetter
		{
			// Construction:
		public:
			TpLetter() : c(0), key(0), left(nullptr), right(nullptr) {
			}
			~TpLetter() {
				if (left != nullptr)
					delete left;
				if (right != nullptr)
					delete right;
			}

			void create(const char* szKey, int nKey);

			// Attributes:
		public:
			char c;
			int key;
			TpLetter* left;
			TpLetter* right;

			// Operations:
		public:
			bool compare(const char* p, int& keyFound);
		} *root;
	};
}