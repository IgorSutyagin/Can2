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

#include "Gnss.h"
#include "Points.h"
#include "Archive.h"

namespace can2
{
	// Node - Abstract base class for calibrations and other data
	class Node
	{
	public:
		Node() : m_ent(entUnk), m_parent(nullptr) {
		}
		Node(const Node& a) {
			*this = a;
		}
		virtual ~Node() {
		}

		Node& operator=(const Node& a)	{
			m_ent = a.m_ent;
			m_parent = a.m_parent;
			return *this;
		}

		enum NodeType
		{
			entUnk = 0,
			entAntexFile = 1,
			entAntexAntenna = 2,
			entRing = 3,
			entRingAntenna = 4
		};

		// OffsetMode is the type of weight function used for PCO computation
		// eNoWeight specifies weight function 1/sin(Z)
		// eOnlyCos specifies weight function 1
		// eSinAndCos specifies weight function cos(Z)
		// For more information see the document "PCC definition"
		enum OffsetMode
		{
			eNoWeight = 0,
			eOnlyCos = 1,
			eSinAndCos = 2
		};

		// Attributes:
	public:
		NodeType m_ent;
		Node* m_parent;

		bool isAntexFile() const { return m_ent == entAntexFile; }
		bool isAntexAntenna() const { return m_ent == entAntexAntenna; }
		bool isRing() const { return m_ent == entRing; }
		bool isRingAntenna() const { return m_ent == entRingAntenna; }
		bool isAntenna() const {
			return m_ent == entRingAntenna || m_ent == entAntexAntenna;
		}

		// Overrides:
	public:
		virtual std::string getName() const { return std::string(); }
		virtual bool hasPcc(int es) const { return false; }
		virtual bool hasPcvRms(int es) const { return false; }
		virtual Node* subtract(const Node* pMinus) const { return nullptr; }
		virtual int childs() const { return 0; }
		virtual Node* getChild(int index) const { assert(false); return nullptr; }
		virtual double getPcv(can2::Gnss::Signal es, double zen, double az) const { return NAN; }
		virtual double getPcc(can2::Gnss::Signal es, double zen, double az) const { return NAN; }

		// Compute phase center offset from PCC. See the document "PCC definition.pdf"
		virtual Point3d calcOffset(can2::Gnss::Signal es, double eleMask, OffsetMode em, double * pro) const { return Point3d(NAN, NAN, NAN); }

		// Returns offset loaded from the ANTEX file
		virtual Point3d getOffset(can2::Gnss::Signal es) const { return Point3d(NAN, NAN, NAN); }

		virtual void serialize(Archive& ar) {
			if (ar.isStoring())
			{
				ar.write(&m_ent, sizeof(m_ent));
			}
			else
			{
				ar.read(&m_ent, sizeof(m_ent));
			}
		}
	};
}