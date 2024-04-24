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


#include "AntexAntenna.h"
#include "Archive.h"

namespace can2
{
	//////////////////////////////////////////////////////////////////////
	// RingAntenna - item of RingNode antenna collection. 
	// Nearly the same as AntexAntenna except two properties
	class RingAntenna : public AntexAntenna
	{
	// Construction:
	public:
		RingAntenna() {	m_ent = entRingAntenna;	}
		RingAntenna(const RingAntenna& a) {
			*this = a;
		}
		RingAntenna(const AntexAntenna& a, const char* alias, const char* sourceFile) {
			*(AntexAntenna*)this = a;
			m_alias = alias;
			m_sourceFile = sourceFile;
		}
		virtual ~RingAntenna() {}

	// Attributes:
	public:
		std::string m_alias; // A nickname used in Ring calibrations comparison. Initially empty
		std::string m_sourceFile; // An ANTEX file this calibration was loaded from

	// Operations:
	public:
		RingAntenna& operator=(const RingAntenna& a) {
			*(AntexAntenna*)this = *(AntexAntenna*)&a;
			m_alias = a.m_alias;
			m_sourceFile = a.m_sourceFile;
			return *this;
		}

	// Overrides:
	public:
		virtual std::string getName() const { return m_alias; }


	// Serialization:
	public:
		virtual void serialize(Archive& ar) {
			AntexAntenna::serialize(ar);
			DWORD dwVer = 1;
			if (ar.isStoring())
			{
				ar << dwVer;
				ar << m_alias;
				ar << m_sourceFile;
			}
			else
			{
				ar >> dwVer;
				ar >> m_alias;
				ar >> m_sourceFile;
			}
		}

	};
	// End of RingAntenna

	//////////////////////////////////////////////////////////////////////
	// RingNode - a collection of different calibrations 

	class RingNode : public Node
	{
		// Construction:
	public:
		RingNode();
		virtual ~RingNode();
		void reset();

		//static RingNode* create(const std::map<std::string, RingAntenna*>& ants);

		struct Cluster
		{
			std::vector<std::shared_ptr<RingAntenna>> ants; // sources for the type-mean
			std::shared_ptr<RingAntenna> atm; // Type mean for clusters with many sources or soure if the cluster has only one calibration
		};

		struct Metrics
		{
			Metrics() {
				em = AntexAntenna::eSinAndCos;
				eleMask = 0;
				bSimpleMode = false;
				em = AntexAntenna::eSinAndCos;
				use(Gnss::G01, true);
				use(Gnss::G02, true);
				use(Gnss::R01, true);
				use(Gnss::R02, true);
			}
			double eleMask; // Not really used yet. May be of use if the metrics changes

			// Frequency types to use when computing the norm
			std::map<Gnss::Signal, bool> fts;

			//
			AntexAntenna::OffsetMode em;
			bool bSimpleMode;

			bool isUsed(Gnss::Signal f) const {
				return fts.find(f) != fts.end() && fts.at(f);
			}
			void use(Gnss::Signal eft, bool use) {
				fts[eft] = use;
			}

			void serialize(Archive& ar);
		};

		// Attributes:
	public:
		// Source calibrations
		std::vector<std::shared_ptr<RingAntenna>> m_ants;

		// Clusters built from source calibrations
		std::vector<Cluster> m_cls;

		// Distances between clusters. 
		// std::pair contains indeces of 
		// clusters in m_cls. index1 < index2
		std::map<std::pair<size_t, size_t>, double> m_dists;

		Metrics m_metrics;

		std::string m_strSourceFile;

		// Operations:
	public:
		void compDists();
		static double norm(const AntexAntenna& a, Metrics& metrics);
		void clusterize(int level);
		Cluster unite(const Cluster& c1, const Cluster& c2);
		void addAntennas(const std::vector<std::shared_ptr<RingAntenna>>& ras);
		void removeAnt(RingAntenna* pa);

		bool allChildsHavePcc(int es) const {
			auto it = std::find_if(m_ants.begin(), m_ants.end(), [&](const std::shared_ptr<RingAntenna>& a) {
				return !a->hasPcc(es);
			});
			return it == m_ants.end();
		}

		// Overrides:
	public:
		std::string getShortName() const;
		virtual std::string getName() const {
			return m_strSourceFile.empty() ? "Ring calibration" : m_strSourceFile;
		}
		virtual int childs() const { return m_ants.size(); }
		virtual Node* getChild(int index) const { return m_ants[index].get(); }

		// Serialization:
	public:
		virtual void serialize(Archive& ar);

	};
	// End of RingNode interface
	//////////////////////////////////////////////////////////////////////
}