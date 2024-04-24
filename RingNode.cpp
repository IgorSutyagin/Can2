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

#include "RingNode.h"

using namespace can2;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RingNode implementation

RingNode::RingNode()
{
	m_ent = entRing;
}

RingNode::~RingNode()
{
	reset();
}

void RingNode::reset()
{
	m_dists.clear();
	m_cls.clear();
	for (auto p : m_ants)
	{
		p = nullptr;
	}
	m_ants.clear();
}

/*
RingNode* RingNode::create(const std::map<std::string, RingAntenna*>& ants)
{
	RingNode* prn = new RingNode();
	prn->m_metrics.use(Gnss::G01, true);
	prn->m_metrics.use(Gnss::G02, true);
	prn->m_metrics.use(Gnss::R01, true);
	prn->m_metrics.use(Gnss::R02, true);

	for (auto& p : ants)
	{
		RingAntenna* pa = new RingAntenna();
		*pa = *p.second;
		pa->m_parent = prn;
		std::shared_ptr<RingAntenna> a(pa);
		pa->m_alias = p.first;
		prn->m_ants.push_back(a);

		Cluster cl;
		cl.ants.push_back(a);
		cl.atm = a;
		prn->m_cls.push_back(cl);
	}

	prn->compDists();

	return prn;
}
*/

//////////////////////////////
// Operations:
void RingNode::compDists()
{
	m_dists.clear();
	for (size_t i = 0; i < m_cls.size(); i++)
	{
		Cluster& ci = m_cls[i];
		for (size_t j = i + 1; j < m_cls.size(); j++)
		{
			Cluster& cj = m_cls[j];

			AntexAntenna* padif = (AntexAntenna *)ci.atm->subtract(cj.atm.get());
			ASSERT(padif->isAntexAntenna());
			double d = norm(*padif, m_metrics);
			m_dists[std::pair<size_t, size_t>(i, j)] = d;
			delete padif;
		}
	}
}

double RingNode::norm(const AntexAntenna& a, Metrics& metrics)
{
	double sumSigma = 0;
	int countSigma = 0;
	for (int ns = Gnss::G01; ns < Gnss::esigInvalid; ns++)
	{
		Gnss::Signal es = (Gnss::Signal)ns;
		if (!metrics.isUsed(es))
			continue;
		if (!a.hasPcc(es))
		{
			// Process the absense of the calibration
			continue;
		}

		double sigma = a.calcNorm(es, metrics.em, metrics.bSimpleMode);
		sigma *= 1000;
		double sigma2 = sigma * sigma;

		sumSigma += sigma2;
		countSigma++;
	}

	return sqrt(sumSigma / countSigma);
}

void RingNode::clusterize(int level)
{
	// Set 0 clustering level
	m_cls.clear();
	for (auto& p : m_ants)
	{
		Cluster cl;
		cl.ants.push_back(p);
		cl.atm = p;
		m_cls.push_back(cl);
	}

	compDists();

	for (int lev = 0; lev < level; lev++)
	{
		std::map<std::pair<size_t, size_t>, double>::iterator it = std::min_element(m_dists.begin(), m_dists.end(), [](const std::pair<std::pair<int, int>, double>& l, const std::pair<std::pair<int, int>, double>& r) {
			return l.second < r.second;
		});
		if (it == m_dists.end())
			return;

		int m = it->first.first;
		int n = it->first.second;

		Cluster& c1 = m_cls[m];
		Cluster& c2 = m_cls[n];
		Cluster c = unite(c1, c2);

		m_cls[m] = c;
		m_cls.erase(m_cls.begin() + n);
		compDists();
	}
}

RingNode::Cluster RingNode::unite(const Cluster& c1, const Cluster& c2)
{
	Cluster c;
	c.atm = std::shared_ptr<RingAntenna>(new RingAntenna());
	std::map<Gnss::Signal, AntexAntenna::SignalData> fsigs;

	std::map<int, double> freqs;
	for (std::vector<std::shared_ptr<RingAntenna>>::const_iterator it = c1.ants.begin(); it != c1.ants.end(); it++)
	{
		c.ants.push_back(*it);
		if (it == c1.ants.begin())
		{
			const AntexAntenna& a = **it;
			c.atm->m_grid = a.m_grid;
			c.atm->m_sigs = a.m_sigs;
		}
	}
	for (std::vector<std::shared_ptr<RingAntenna>>::const_iterator it = c2.ants.begin(); it != c2.ants.end(); it++)
	{
		c.ants.push_back(*it);
	}


	for (std::vector<std::shared_ptr<RingAntenna>>::const_iterator it = c.ants.begin(); it != c.ants.end(); it++)
	{
		const RingAntenna& a = **it;
		if (c.atm->m_alias.empty())
		{
			c.atm->m_alias = "(";
		}
		else
		{
			c.atm->m_alias += ",";
		}
		c.atm->m_alias += a.m_alias;

		std::map<Gnss::Signal, AntexAntenna::SignalData> sigs = a.m_sigs;

		for (int nf = 0; nf < Gnss::esigInvalid; nf++)
		{
			Gnss::Signal eft = (Gnss::Signal)nf;
			if (sigs.find(eft) != sigs.end())
			{
				Point3d pcoa = a.getOffset(eft);
				AntexAntenna::SignalData& sigA = sigs[eft];
				if (fsigs.find(eft) != fsigs.end())
				{
					// Offset
					Point3d pco = c.atm->getOffset(eft);
					pco = (pco * freqs[eft] + pcoa) * (1.0 / (freqs[eft] + 1));
					c.atm->m_sigs[eft].pco = pco;

					// NoAzi pcvs
					std::vector<double>& noaPcvs = fsigs[eft].naPcv;
					if (noaPcvs.size() != sigA.naPcv.size())
						AfxThrowUserException();
					for (int i = 0; i < (int)noaPcvs.size(); i++)
					{
						noaPcvs[i] = (noaPcvs[i] * freqs[eft] + sigA.naPcv[i]) / (freqs[eft] + 1);
					}

					// Azimuth dependent PCVs
					std::vector<double>& pcvs = fsigs[eft].aPcv;
					if (pcvs.size() != sigA.aPcv.size())
						AfxThrowUserException();
					for (int i= 0; i<(int)pcvs.size(); i++)
					{
						pcvs[i] = (pcvs[i] * freqs[eft] + sigA.aPcv[i]) / (freqs[eft] + 1);
					}

					freqs[eft] += 1.0;
				}
				else
				{
					fsigs[eft] = sigA;
					freqs[eft] = 1.0;
				}
			}
		}
	}
	c.atm->m_alias += ")";
	c.atm->m_sigs = fsigs;
	return c;
}

void RingNode::addAntennas(const std::vector<std::shared_ptr<RingAntenna>>& ras)
{
	m_ants.insert(m_ants.end(), ras.begin(), ras.end());
	clusterize(0);
}

void RingNode::removeAnt(RingAntenna* pa)
{
	for (int i = 0; i < (int)m_ants.size(); i++)
	{
		if (m_ants[i].get() == pa)
		{
			m_ants.erase(m_ants.begin() + i);
			break;
		}
	}

	clusterize(0);
}

///////////////////////////
// Overrides

std::string RingNode::getShortName() const
{
	if (m_strSourceFile.empty())
		return "Ring calibration";

	std::filesystem::path path(m_strSourceFile);
	return path.stem().u8string();
}

////////////////////////////////
// Serialization
void RingNode::Metrics::serialize(Archive& ar)
{
	DWORD dwVer = 2;
	if (ar.isStoring())
	{
		ar << dwVer;
		ar << eleMask;
		int size = fts.size();
		ar << size;
		for (std::map<Gnss::Signal, bool>::iterator it = fts.begin(); it != fts.end(); it++)
		{
			Gnss::Signal eft = it->first;
			bool b = it->second;
			ar.write(&eft, sizeof(eft));
			ar.write(&b, sizeof(b));
		}

		ar << (short)em;
		ar << bSimpleMode;
	}
	else
	{
		ar >> dwVer;
		ar >> eleMask;
		int size = 0;
		ar >> size;
		fts.clear();
		for (int i = 0; i < size; i++)
		{
			Gnss::Signal eft;
			bool b;
			ar.read(&eft, sizeof(eft));
			ar.read(&b, sizeof(b));
			fts[eft] = b;
		}

		if (dwVer >= 2)
		{
			short n = 0;
			ar >> n;
			em = (AntexAntenna::OffsetMode)n;
			ar >> bSimpleMode;
		}
	}
}

void RingNode::serialize(Archive& ar)
{
	DWORD dwVer = 1001;
	if (ar.isStoring())
	{
		ar << dwVer;

		ar << m_strSourceFile;

		int nAnts = m_ants.size();
		ar << nAnts;
		for (int i = 0; i < nAnts; i++)
		{
			m_ants[i]->serialize(ar);
		}
		m_metrics.serialize(ar);
	}
	else
	{
		ar >> dwVer;
		if (dwVer < 1001)
			throw std::runtime_error("RingAnt files from CAN are not supported yet");

		ar >> m_strSourceFile;

		int nAnts = 0;
		ar >> nAnts;
		m_ants.clear();
		for (int i = 0; i < nAnts; i++)
		{
			std::shared_ptr<RingAntenna> a(new RingAntenna);
			a->serialize(ar);
			a->m_parent = this;
			m_ants.push_back(a);
		}
		m_metrics.serialize(ar);
		clusterize(0);
	}
}

// End of RingNode implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
