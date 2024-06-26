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
#include "Sph.h"

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


//////////////////////////////
// Operations:
void RingNode::compDists()
{
	compDists2();
	return;

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

void RingNode::compDists2()
{
	// Precompute distances between antennas from m_ants
	std::map<std::pair<std::string, std::string>, double> antDists;
	for (size_t i = 0; i < m_ants.size(); i++)
	{
		RingAntenna* pi = m_ants[i].get();
		for (size_t j = i + 1; j < m_ants.size(); j++)
		{
			RingAntenna* pj = m_ants[j].get();
			AntexAntenna* padif = (AntexAntenna*)pi->subtract(pj);
			double d = norm(*padif, m_metrics);
			std::pair<std::string, std::string> key(pi->getName(), pj->getName());
			antDists[key] = d;
			delete padif;
		}
	}

	auto clustDist = [&](const Cluster& c1, const Cluster& c2) {
		double dist = 0;
		for (size_t i = 0; i < c1.ants.size(); i++)
		{
			RingAntenna* pi = c1.ants[i].get();
			for (size_t j = 0; j < c2.ants.size(); j++)
			{
				RingAntenna* pj = c2.ants[j].get();
				std::pair<std::string, std::string> key1(pi->getName(), pj->getName());
				std::pair<std::string, std::string> key2(pj->getName(), pi->getName());
				double d = antDists.find(key1) != antDists.end() ? antDists[key1] : 
					antDists.find(key2) != antDists.end() ? antDists[key2] : NAN;
				ASSERT(!isnan(d));
				if (fabs(d) > dist)
					dist = fabs(d);
			}
		}
		return dist;
	};
	m_dists.clear();
	for (size_t i = 0; i < m_cls.size(); i++)
	{
		Cluster& ci = m_cls[i];
		for (size_t j = i + 1; j < m_cls.size(); j++)
		{
			Cluster& cj = m_cls[j];

			double d = clustDist(ci, cj);
			m_dists[std::pair<size_t, size_t>(i, j)] = d;
		}
	}
}

double RingNode::norm(const AntexAntenna& a, Metrics& metrics)
{
	double sumSigma = 0;
	int countSigma = 0;
	double maxDist = 0;
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

		double sigma = NAN;
		if (metrics.et == Metrics::etPcoPcv)
		{
			sigma = a.calcNorm(es, metrics.em, metrics.bSimpleMode);
		}
		else if (metrics.et == Metrics::etPco)
		{
			Point3d pco = a.calcOffset(es, metrics.eleMask, metrics.em);
			sigma = pco.rad();
		}
		else if (metrics.et == Metrics::etVPco)
		{
			Point3d pco = a.calcOffset(es, metrics.eleMask, metrics.em);
			sigma = fabs(pco.u);
		}
		else if (metrics.et == Metrics::etHPco)
		{
			Point3d pco = a.calcOffset(es, metrics.eleMask, metrics.em);
			sigma = sqrt(pco.x * pco.x + pco.y * pco.y);
		}
		else if (metrics.et == Metrics::etE)
		{
			Point3d pco = a.calcOffset(es, metrics.eleMask, metrics.em);
			sigma = fabs(pco.x);
		}
		else if (metrics.et == Metrics::etN)
		{
			Point3d pco = a.calcOffset(es, metrics.eleMask, metrics.em);
			sigma = fabs(pco.y);
		}
		sigma *= 1000;
		if (fabs(sigma) > maxDist)
			maxDist = fabs(sigma);

		double sigma2 = sigma * sigma;

		sumSigma += sigma2;
		countSigma++;
	}

	if (metrics.esp == Metrics::espRms)
		return sqrt(sumSigma / countSigma);
	else if (metrics.esp == Metrics::espMax)
		return maxDist;
	else
	{
		ASSERT(FALSE);
		return NAN;
	}
}

void RingNode::clusterize(int level, ClusterMode cm, std::map<std::string, bool> * useAnts, int maxClust)
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

	if (false)
	{
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
			//Cluster c = unite(c1, c2);
			Cluster c = (cm.meanType == ClusterMode::Arithmetic) ? unite(cm, c1, &c2) : uniteSh(cm, c1, &c2);

			m_cls[m] = c;
			m_cls.erase(m_cls.begin() + n);
			compDists();
		}
	}
	else
	{
		for (int lev = 0; lev < level; lev++)
		{
			std::map<std::pair<size_t, size_t>, double>::iterator itMin = m_dists.end();
			if (useAnts == nullptr) // Use all antennas
			{
				itMin = std::min_element(m_dists.begin(), m_dists.end(), [&](const std::pair<std::pair<int, int>, double>& l, const std::pair<std::pair<int, int>, double>& r) {
					return l.second < r.second;
				});
				if (itMin == m_dists.end())
					return;
			}
			else
			{
				int nClustNum = 0;
				for (size_t nc = 0; nc < m_cls.size(); nc++)
				{
					if (m_cls[nc].ants.size() > 1)
						nClustNum++;
				}
				
				double minVal = DBL_MAX;
				for (std::map<std::pair<size_t, size_t>, double>::iterator it = m_dists.begin(); it != m_dists.end(); it++)
				{
					Cluster& c1 = m_cls[it->first.first];
					Cluster& c2 = m_cls[it->first.second];
					if (maxClust > 0 && nClustNum >= maxClust) // No more clusters, so do not create a new one
					{
						if (c1.ants.size() <= 1 && c2.ants.size() <= 1)
							continue;
					}

					bool bUse = true;
					std::for_each(c1.ants.begin(), c1.ants.end(), [&](const std::shared_ptr<RingAntenna>& a) {
						if (!(*useAnts)[a->getName()])
							bUse = false;
					});
					if (!bUse)
						continue;
					std::for_each(c2.ants.begin(), c2.ants.end(), [&](const std::shared_ptr<RingAntenna>& a) {
						if (!(*useAnts)[a->getName()])
							bUse = false;
					});
					if (!bUse)
						continue;
					if (it->second < minVal)
					{
						itMin = it;
						minVal = it->second;
					}
				}
			}

			if (itMin == m_dists.end())
				return;

			int m = itMin->first.first;
			int n = itMin->first.second;

			Cluster c;
			Cluster& c1 = m_cls[m];
			Cluster& c2 = m_cls[n];
			for (size_t i = 0; i < c1.ants.size(); i++)
			{
				c.ants.push_back(c1.ants[i]);
			}
			for (size_t i = 0; i < c2.ants.size(); i++)
			{
				c.ants.push_back(c2.ants[i]);
			}

			//Cluster c = unite(c1, c2);
			//Cluster c = (cm.meanType == ClusterMode::Arithmetic) ? unite(cm, c1, &c2) : uniteSh(cm, c1, &c2);

			m_cls[m] = c;
			m_cls.erase(m_cls.begin() + n);
			compDists();
		}

		for (size_t nc = 0; nc < m_cls.size(); nc++)
		{
			Cluster& c = m_cls[nc];
			if (c.ants.size() <= 1)
				continue;

			Cluster cNew = (cm.meanType == ClusterMode::Arithmetic) ? unite(cm, c) : uniteSh(cm, c);
			m_cls[nc] = cNew;
		}

		std::sort(m_cls.begin(), m_cls.end(), [&](const Cluster& c1, const Cluster& c2) {
			return c1.ants.size() < c2.ants.size();
		});

		compDists();
	}
}

void RingNode::clusterizeManual(std::vector<std::vector<RingAntenna*>>& cls, std::vector<std::string>& names, ClusterMode cm)
{
	m_cls.clear();
	m_dists.clear();

	for (int nc = 0; nc < (int)cls.size(); nc++)
	{
		std::vector<RingAntenna*>& as = cls[nc];

		if (as.size() == 0)
			continue;
		else if (as.size() == 1)
		{
			std::vector<std::shared_ptr<RingAntenna>>::iterator it = std::find_if(m_ants.begin(), m_ants.end(), [&](const std::shared_ptr<RingAntenna>& a) {
				return as[0] == a.get();
			});
			if (it != m_ants.end())
			{
				Cluster c;
				c.ants.push_back(*it);
				c.atm = *it;
				m_cls.push_back(c);
			}

		}
		else
		{
			Cluster c;
			for (int i = 0; i < (int)as.size(); i++)
			{
				std::vector<std::shared_ptr<RingAntenna>>::iterator it = std::find_if(m_ants.begin(), m_ants.end(), [&](const std::shared_ptr<RingAntenna>& a) {
					return as[i] == a.get();
				});
				if (it != m_ants.end())
				{
					c.ants.push_back(*it);
				}
			}

			if (cm.meanType == ClusterMode::Arithmetic)
			{
				Cluster cNew = unite(cm, c);
				if (!names[nc].empty())
					cNew.atm->m_alias = names[nc];
				m_cls.push_back(cNew);
			}
			else
			{
				Cluster cNew = uniteSh(cm, c);
				if (!names[nc].empty())
					cNew.atm->m_alias = names[nc];
				m_cls.push_back(cNew);
			}
		}
	}
	compDists();
}

RingNode::Cluster RingNode::unite(ClusterMode cm, const Cluster& c1, const Cluster * pc2)
{
	Cluster c;
	c.atm = std::shared_ptr<RingAntenna>(new RingAntenna());
	std::map<Gnss::Signal, AntexAntenna::SignalData> fsigs;
	int useSigs[Gnss::esigInvalid];
	for (int i = 0; i < Gnss::esigInvalid; i++)
		useSigs[i] = 0;

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

		RingAntenna& ra = **it;
		for (auto it = ra.m_sigs.begin(); it != ra.m_sigs.end(); it++)
		{
			useSigs[it->first]++;
		}
	}

	if (pc2 != nullptr)
	{
		for (std::vector<std::shared_ptr<RingAntenna>>::const_iterator it = pc2->ants.begin(); it != pc2->ants.end(); it++)
		{
			c.ants.push_back(*it);

			RingAntenna& ra = **it;
			for (auto it = ra.m_sigs.begin(); it != ra.m_sigs.end(); it++)
			{
				useSigs[it->first]++;
			}
		}
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
			if (useSigs[eft] == 0)
				continue;
			if (useSigs[eft] < 2 && cm.useSignal >= ClusterMode::AtLeast2)
				continue;
			if (useSigs[eft] != c.ants.size() && cm.useSignal == ClusterMode::OnlyCommon)
				continue;

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
					{
						for (int nz = 0; nz < c.atm->m_grid.nz; nz++)
						{
							double zen = c.atm->m_grid.za0.zen + c.atm->m_grid.step.zen * nz;
							noaPcvs[nz] = (noaPcvs[nz] * freqs[eft] + sigA.getPcv(zen)) / (freqs[eft] + 1);
						}
					}
					else
					{
						for (int i = 0; i < (int)noaPcvs.size(); i++)
						{
							noaPcvs[i] = (noaPcvs[i] * freqs[eft] + sigA.naPcv[i]) / (freqs[eft] + 1);
						}
					}

					// Azimuth dependent PCVs
					std::vector<double>& pcvs = fsigs[eft].aPcv;
					if (pcvs.size() != sigA.aPcv.size())
					{
						for (int na = 0; na < c.atm->m_grid.na; na++)
						{
							double az = c.atm->m_grid.za0.az + c.atm->m_grid.step.az * na;
							for (int nz = 0; nz < c.atm->m_grid.nz; nz++)
							{
								double zen = c.atm->m_grid.za0.zen + c.atm->m_grid.step.zen * nz;
								fsigs[eft].pcv(nz, na) = (fsigs[eft].pcv(nz, na) * freqs[eft] + sigA.getPcv(zen, az)) / (freqs[eft] + 1);
							}
						}
					}
					else
					{
						for (int i = 0; i < (int)pcvs.size(); i++)
						{
							pcvs[i] = (pcvs[i] * freqs[eft] + sigA.aPcv[i]) / (freqs[eft] + 1);
						}
					}

					freqs[eft] += 1.0;
				}
				else
				{
					if (c.atm->m_grid == a.m_grid)
					{
						fsigs[eft] = sigA;
					}
					else
					{
						fsigs[eft] = AntexAntenna::SignalData(eft, c.atm.get(), Point3d(0, 0, 0));
						AntexAntenna::SignalData& sig = fsigs[eft];
						sig.pco = sigA.pco;
						for (int nz = 0; nz < c.atm->m_grid.nz; nz++)
						{
							double zen = c.atm->m_grid.za0.zen + c.atm->m_grid.step.zen * nz;
							sig.naPcv[nz] = sigA.getPcv(zen);
						}
						for (int na = 0; na < c.atm->m_grid.na; na++)
						{
							double az = c.atm->m_grid.za0.az + c.atm->m_grid.step.az * na;
							for (int nz = 0; nz < c.atm->m_grid.nz; nz++)
							{
								double zen = c.atm->m_grid.za0.zen + c.atm->m_grid.step.zen * nz;
								sig.pcv(nz, na) = sigA.getPcv(zen, az);
							}
						}
					}
					freqs[eft] = 1.0;
				}
			}
		}
	}
	c.atm->m_alias += ")";
	c.atm->m_sigs = fsigs;
	return c;
}

RingNode::Cluster RingNode::uniteSh(ClusterMode cm, const Cluster& c1, const Cluster * pc2)
{
	Cluster c;
	c.atm = std::shared_ptr<RingAntenna>(new RingAntenna());
	std::map<Gnss::Signal, AntexAntenna::SignalData> fsigs;
	int useSigs[Gnss::esigInvalid];
	for (int i = 0; i < Gnss::esigInvalid; i++)
		useSigs[i] = 0;

	std::map<int, double> freqs;
	const AntexAntenna* paFirst = nullptr;
	for (std::vector<std::shared_ptr<RingAntenna>>::const_iterator it = c1.ants.begin(); it != c1.ants.end(); it++)
	{
		c.ants.push_back(*it);
		if (it == c1.ants.begin())
		{
			const AntexAntenna& a = **it;
			paFirst = &a;
			c.atm->m_grid = a.m_grid;
		}

		RingAntenna& ra = **it;
		for (auto it = ra.m_sigs.begin(); it != ra.m_sigs.end(); it++)
		{
			useSigs[it->first]++;
		}
	}
	if (pc2 != nullptr)
	{
		for (std::vector<std::shared_ptr<RingAntenna>>::const_iterator it = pc2->ants.begin(); it != pc2->ants.end(); it++)
		{
			c.ants.push_back(*it);

			RingAntenna& ra = **it;
			for (auto it = ra.m_sigs.begin(); it != ra.m_sigs.end(); it++)
			{
				useSigs[it->first]++;
			}
		}
	}

	ShFreqProjection shp[Gnss::ebMax];
	//ShProjection shp[Gnss::ebMax];
	for (int nb = 0; nb < Gnss::ebMax; nb++)
	{
		Gnss::Band eb = (Gnss::Band)nb;
		//if (eb > 0)
		//	continue;

		// Setup bands properties
		std::map<double, int> mapBandFreqs;
		std::map<Gnss::Signal, int> mapBandSigs;
		for (std::vector<std::shared_ptr<RingAntenna>>::const_iterator it = c.ants.begin(); it != c.ants.end(); it++)
		{
			const RingAntenna& a = **it;
			for (int nf = 0; nf < Gnss::esigInvalid; nf++)
			{
				Gnss::Signal eft = (Gnss::Signal)nf;
				if (useSigs[eft] == 0)
					continue;
				if (useSigs[eft] < 2 && cm.useSignal >= ClusterMode::AtLeast2)
					continue;
				if (useSigs[eft] != c.ants.size() && cm.useSignal == ClusterMode::OnlyCommon)
					continue;

				if (a.m_sigs.find(eft) == a.m_sigs.end())
					continue;
				if (Gnss::getBand(eft) != eb)
					continue;
				double f = Gnss::getSysFreq(eft);
				mapBandFreqs[f]++;
				mapBandSigs[eft]++;
			}
		}

		int nBandFreqs = mapBandFreqs.size();
		if (nBandFreqs == 0)
			continue;
		else if (nBandFreqs == 1)
			shp[nb].setBands(9, 9, 1, -80);
			//shp[nb].setBands(9, 9, -90);
		else
			shp[nb].setBands(9, 9, 2, -80);
			//shp[nb].setBands(9, 9, -90);

		int nMatrixSize = shp[nb].getMatrixSize();

		Matrix N;
		N.resize(nMatrixSize, nMatrixSize);
		N.init(0);
		Vector U;
		U.resize(nMatrixSize);
		U.init(0);

		//ShProjection shTest;
		//shTest.setBands(5, 5, -90);
		//shTest.m_coefs[4] = 1.0;
		//shTest.m_coefs[6] = 2.0;
		//shTest.m_coefs[8] = 3.0;

		for (std::vector<std::shared_ptr<RingAntenna>>::const_iterator it = c.ants.begin(); it != c.ants.end(); it++)
		{
			const RingAntenna& a = **it;
			//if (it != c.ants.begin())
			//	continue;

			for (int nf = 0; nf < Gnss::esigInvalid; nf++)
			{
				Gnss::Signal eft = (Gnss::Signal)nf;
				if (useSigs[eft] == 0)
					continue;
				if (useSigs[eft] < 2 && cm.useSignal >= ClusterMode::AtLeast2)
					continue;
				if (useSigs[eft] != c.ants.size() && cm.useSignal == ClusterMode::OnlyCommon)
					continue;
				if (a.m_sigs.find(eft) == a.m_sigs.end())
					continue;
				if (Gnss::getBand(eft) != eb)
					continue;

				double f = Gnss::getNormFreq(eb, eft);
				for (int na = 0; na < a.m_grid.na - 1; na++)
				{
					double az = na * a.m_grid.step.az;
					for (int nz = 0; nz < a.m_grid.nz; nz++)
					{
						double zen = nz * a.m_grid.step.zen;

						double ph = a.getPcc(eft, zen, az);
						//ph = shTest.pcc(ThetaPhi(ZenAz(zen, az)));
						double w = sin(zen * PI / 180);
						w = 1;
						//shp[nb].appendEqs(ZenAz(zen, az), ph, w, N, U);
						ZenAz za(zen, az);
						shp[nb].appendEqs(za, f, ph, w, N, U);
					}
				}
			}
		}

		N.copyUpperTriangle();

		//N.fileDump("test.txt", U, "N", TRUE);
		Vector X = N.solve(U);
		shp[eb].setCoefs(X);

		/*
		for (std::vector<std::shared_ptr<RingAntenna>>::const_iterator it = c.ants.begin(); it != c.ants.end(); it++)
		{
			const RingAntenna& a = **it;
			if (it != c.ants.begin())
				continue;

			for (int nf = 0; nf < Gnss::esigInvalid; nf++)
			{
				Gnss::Signal eft = (Gnss::Signal)nf;
				if (eft != Gnss::G01)
					continue;
				if (a.m_sigs.find(eft) == a.m_sigs.end())
					continue;
				if (Gnss::getBand(eft) != eb)
					continue;

				double f = Gnss::getNormFreq(eb, eft);
				ShProjection s = shp[eb].getProjection(Gnss::getNormFreq(eb, f));
				double az = 0;
				std::vector<double> dif;
				for (int nz = 0; nz < a.m_grid.nz; nz++)
				{
					double zen = nz * a.m_grid.step.zen;

					double ph = a.getPcc(eft, zen, az);
					double ph0 = s.pcc(ThetaPhi(ZenAz(zen, az)));
					dif.push_back(ph - ph0);
				}

				Sleep(0);
			}
		}
		*/


		for (auto it = mapBandFreqs.begin(); it != mapBandFreqs.end(); it++)
		{
			double f = it->first;
			ShProjection s = shp[eb].getProjection(Gnss::getNormFreq(eb, f));
			Point3d pco = s.calcOffset(0, c.atm->m_grid.step);
			for (auto it = mapBandSigs.begin(); it != mapBandSigs.end(); it++)
			{
				Gnss::Signal es = it->first;
				if (useSigs[es] == 0)
					continue;
				if (useSigs[es] < 2 && cm.useSignal >= ClusterMode::AtLeast2)
					continue;
				if (useSigs[es] != c.ants.size() && cm.useSignal == ClusterMode::OnlyCommon)
					continue;
				if (Gnss::getSysFreq(es) != f)
					continue;

				if (c.atm->m_sigs.find(es) == c.atm->m_sigs.end())
				{
					c.atm->m_sigs[es] = AntexAntenna::SignalData(es, paFirst, Point3d(0, 0, 0));
					c.atm->m_sigs[es].setGrid(c.atm->m_grid);
				}
				AntexAntenna::SignalData& sig = c.atm->m_sigs[es];
				sig.pco = pco;
				double fNorm = Gnss::getNormFreq(eb, es);
				ShProjection sh = shp[eb].getProjection(fNorm);
				double pcv0 = NAN;
				for (int nz = 0; nz < c.atm->m_grid.nz; nz++)
				{
					double zen = c.atm->m_grid.za0.zen + c.atm->m_grid.step.zen * nz;
					for (int na=0; na < c.atm->m_grid.na; na++)
					{
						double az = c.atm->m_grid.za0.az + c.atm->m_grid.step.az * na;
						ThetaPhi tp(ZenAz(zen, az));
						double pcv = sh.pcv(tp, pco);
						if (isnan(pcv0))
							pcv0 = pcv;
						sig.pcv(nz, na) = pcv - pcv0;
					}
				}
			}
		}

		/*
		for (std::vector<std::shared_ptr<RingAntenna>>::const_iterator it = c.ants.begin(); it != c.ants.end(); it++)
		{
			const RingAntenna& a = **it;
			if (it != c.ants.begin())
				continue;

			for (int nf = 0; nf < Gnss::esigInvalid; nf++)
			{
				Gnss::Signal eft = (Gnss::Signal)nf;
				if (eft != Gnss::G01)
					continue;
				if (a.m_sigs.find(eft) == a.m_sigs.end())
					continue;
				if (Gnss::getBand(eft) != eb)
					continue;

				double f = Gnss::getNormFreq(eb, eft);
				ShProjection s = shp[eb].getProjection(Gnss::getNormFreq(eb, f));
				double az = 0;
				std::vector<double> dif;
				for (int nz = 0; nz < a.m_grid.nz; nz++)
				{
					double zen = nz * a.m_grid.step.zen;

					double ph = a.getPcc(eft, zen, az);
					double ph0 = c.atm->getPcc(eft, zen, az);
					dif.push_back(ph - ph0);
					if (nz > 0)
						dif[dif.size() - 1] -= dif[0];
				}

				dif[0] = 0;

				Sleep(0);
			}
		}
		*/
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
	}

	c.atm->m_alias += ")";
	//c.atm->m_sigs = fsigs;
	return c;

}

void RingNode::addAntennas(const std::vector<std::shared_ptr<RingAntenna>>& ras)
{
	m_ants.insert(m_ants.end(), ras.begin(), ras.end());
	clusterize(0, ClusterMode());
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

	clusterize(0, ClusterMode());
}

void RingNode::swapAntennas(RingAntenna* pa0, RingAntenna* pa1)
{
	int index0 = -1;
	int index1 = -1;
	for (int i = 0; i < (int)m_ants.size(); i++)
	{
		if (m_ants[i].get() == pa0)
		{
			index0 = i;
		}
		if (m_ants[i].get() == pa1)
		{
			index1 = i;
		}
	}

	if (index0 < 0 || index1 < 0)
		return;

	std::shared_ptr<RingAntenna> a0 = m_ants[index0];
	m_ants[index0] = m_ants[index1];
	m_ants[index1] = a0;
	clusterize(0, ClusterMode());
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
	DWORD dwVer = 5;
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
		ar << (short)esp;
		ar << (short)et;
		ar << maxClust;
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

		if (dwVer >= 3)
		{
			short n = 0;
			ar >> n;
			esp = (SigProc)n;
		}

		if (dwVer >= 4)
		{
			short n = 0;
			ar >> n;
			et = (Type)n;
		}

		if (dwVer >= 5)
		{
			ar >> maxClust;
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
		clusterize(0, ClusterMode());
	}
}

// End of RingNode implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
