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
			m_ent = entRingAntenna;
			m_alias = alias;
			m_sourceFile = sourceFile;
			m_antNum = "1";
		}
		virtual ~RingAntenna() {}

	// Attributes:
	public:
		std::string m_alias; // A nickname used in Ring calibrations comparison. Initially empty
		std::string m_sourceFile; // An ANTEX file this calibration was loaded from
		std::string m_antNum; // Number of antennas used to create this one

	// Operations:
	public:
		RingAntenna& operator=(const RingAntenna& a) {
			*(AntexAntenna*)this = *(AntexAntenna*)&a;
			m_alias = a.m_alias;
			m_sourceFile = a.m_sourceFile;
			m_antNum = a.m_antNum;
			return *this;
		}

	// Overrides:
	public:
		virtual std::string getName() const { return m_alias; }


	// Serialization:
	public:
		virtual void serialize(Archive& ar) {
			AntexAntenna::serialize(ar);
			DWORD dwVer = 2;
			if (ar.isStoring())
			{
				ar << dwVer;
				ar << m_alias;
				ar << m_sourceFile;
				ar << m_antNum;
			}
			else
			{
				ar >> dwVer;
				ar >> m_alias;
				ar >> m_sourceFile;
				if (dwVer < 2)
					return;
				ar >> m_antNum;
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

			void getMinMaxOff(Point3d& ptMin, Point3d& ptMax, double& roMin, double& roMax, Gnss::Signal es, double eleMask, AntexAntenna::OffsetMode em) const {
				if (ants.size() < 2)
					return;

				ptMin = Point3d(DBL_MAX, DBL_MAX, DBL_MAX);
				ptMax = Point3d(-DBL_MAX, -DBL_MAX, -DBL_MAX);
				roMin = DBL_MAX;
				roMax = -DBL_MIN;
				for (int i = 0; i < (int)ants.size(); i++) {
					double ro = 0;
					Point3d pco = ants[i]->calcOffset(es, eleMask, em, &ro);
					if (!pco.isValid())
						continue;
					if (pco.x < ptMin.x)
						ptMin.x = pco.x;
					if (pco.y < ptMin.y)
						ptMin.y = pco.y;
					if (pco.z < ptMin.z)
						ptMin.z = pco.z;
					if (pco.x > ptMax.x)
						ptMax.x = pco.x;
					if (pco.y > ptMax.y)
						ptMax.y = pco.y;
					if (pco.z > ptMax.z)
						ptMax.z = pco.z;
					if (ro < roMin)
						roMin = ro;
					if (ro > roMax)
						roMax = ro;
				}
				if (ptMin.x > ptMax.x)
					ptMin = ptMax = Point3d(NAN, NAN, NAN);
				if (roMin > roMax)
					roMin = roMax = NAN;
			}
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
				esp = espMax;
				et = etPcoPcv;
				maxClust = 1;
				ecd = ecdMax;
			}
			double eleMask; // Not really used yet. May be of use if the metrics changes

			// Frequency types to use when computing the norm
			std::map<Gnss::Signal, bool> fts;

			//
			AntexAntenna::OffsetMode em;
			bool bSimpleMode;

			enum SigProc
			{
				espRms = 0, // Root mean square among all available signals
				espMax = 1  // Maximum distance among all available signals
			} esp;

			enum Type
			{
				etPcoPcv = 0,
				etPco = 1,
				etVPco = 2,
				etHPco = 3,
				etE = 4,
				etN = 5
			} et;

			int maxClust; // Maximum number of clusters

			enum ClustDist
			{
				ecdMax = 0,
				ecdMean = 1,
				ecdMin = 2
			} ecd;

			bool isUsed(Gnss::Signal f) const {
				return fts.find(f) != fts.end() && fts.at(f);
			}
			void use(Gnss::Signal eft, bool use) {
				fts[eft] = use;
			}
			void setMultisignalMode(int nMode) {
				if (espRms <= nMode && nMode <= espMax)
					esp = (SigProc)nMode;
				else
					esp = espMax;
			}
			void setType(int nType) {
				if (etPcoPcv <= nType && nType <= etN)
					et = (Type)nType;
				else
					et = etPcoPcv;
			}

			void serialize(Archive& ar);
		};

		struct ClusterMode
		{
			ClusterMode() : meanType(Arithmetic), useSignal(Any) {}
			ClusterMode(int nMeanType, int nUseSignal) {
				meanType = (0 <= nMeanType && nMeanType <= Shp) ? (MeanType)nMeanType : Arithmetic;
				useSignal = (0 <= nUseSignal && nUseSignal <= OnlyCommon) ? (UseSignal)nUseSignal : Any;
			}
			enum MeanType
			{
				Arithmetic = 0,
				Shp = 1
			} meanType;
			enum UseSignal
			{
				Any = 0, // Use any available signal
				AtLeast2 = 1, // Use signals which are at least in 2 cluster calibrations
				OnlyCommon = 2 // Use only signals which are in all cluster calibrations
			} useSignal;
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

		std::string m_title; // Used in graph titles

		// Operations:
	public:
		void compDists();
		void compDists2(); // Compute distance not between cluster mean but between all cluster members
		static double norm(const AntexAntenna& a, Metrics& metrics);
		void clusterize(int level, ClusterMode cm, std::map<std::string, bool>* useAnts = nullptr); // , int maxClust = 0);
		void clusterizeManual(std::vector<std::vector<RingAntenna*>>& cls, std::vector<std::string>& names, ClusterMode cm);
		Cluster unite(ClusterMode cm, const Cluster& c1, const Cluster * pc2=nullptr);
		Cluster uniteSh(ClusterMode cm, const Cluster& c1, const Cluster * pc2=nullptr);
		void addAntennas(const std::vector<std::shared_ptr<RingAntenna>>& ras);
		void removeAnt(RingAntenna* pa);
		void swapAntennas(RingAntenna* pa0, RingAntenna* pa1);

		bool allChildsHavePcc(int es) const {
			auto it = std::find_if(m_ants.begin(), m_ants.end(), [&](const std::shared_ptr<RingAntenna>& a) {
				return !a->hasPcc(es);
			});
			return it == m_ants.end();
		}

		bool hasCluster() const {
			for (int i = 0; i < (int)m_cls.size(); i++)
			{
				if (m_cls[i].ants.size() > 1)
					return true;
			}
			return false;
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