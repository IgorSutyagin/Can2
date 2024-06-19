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
#include "Angles.h"
#include "Node.h"
#include "Tools.h"

namespace can2
{
	class AntexAntenna : public Node
	{
	// Constructon
	public:
		AntexAntenna() : m_bDifference(false) {
			m_ent = entAntexAntenna;
		}
		AntexAntenna(const AntexAntenna& a) {
			*this = a;
		}
		virtual ~AntexAntenna() {
			m_sigs.clear();
		}

		AntexAntenna& operator=(const AntexAntenna& a);

		std::string m_type; // IGS Antenna type
		std::string m_radome; // Antenna radome
		std::string m_serial; // Antenna serial numner
		std::string m_method; // Calibration method
		std::string m_agency; // Who calibrate this antenna
		std::string m_numIndAnt; // Number of individual antennas calibrated
		std::string m_date; // Calibration date
		bool m_bDifference; // If this is the difference of two calibrations

		std::string getFullName() const {
			std::string str = m_type + " " + m_radome + " " + m_serial;
			rtrim(str);
			return str;
		}

		struct Grid
		{
			ZenAz step; // Steps (5, 5)
			ZenAz za0; // Minimum values of anges (0, 0)
			ZenAz za1; // Maximum vlues of angles (90, 360)
			int na; // Number of azimuth angles
			int nz; // Number of zenith angles

			bool isValid() const {
				return nz > 0;
			}

			bool operator==(const Grid& a) const {
				return step == a.step && za0 == a.za0 && za1 == a.za1 && na == a.na && nz == a.nz;
			}

			void serialize(Archive& ar)
			{
				DWORD dwVer = 1;
				if (ar.isStoring())
				{
					ar << dwVer;
					ar << step;
					ar << za0;
					ar << za1;
					ar << na;
					ar << nz;
				}
				else
				{
					ar >> dwVer;
					ar >> step;
					ar >> za0;
					ar >> za1;
					ar >> na;
					ar >> nz;
				}
			}
		} m_grid;

		struct SignalData
		{
			SignalData() : es(Gnss::esigInvalid), pco(NAN, NAN, NAN), ant(nullptr) {}
			SignalData(Gnss::Signal es_, const AntexAntenna* ant_, Point3d pco_) : es(es_), ant(ant_), pco(pco_) {
				setGrid(ant->m_grid);
			}
			const AntexAntenna* ant;
			Gnss::Signal es;
			Point3d pco;
			Point3d pcoRms;
			std::vector<double> naPcv; // No azimuth PCV 0, dz, 2*dz, ... 90 (m). The sign is correct.
			std::vector<double> aPcv; // PCV (m). The sign is correct
			std::vector<double> naRms; // No azimuth RMS 0, dz, 2*dz, ... 90 (m)
			std::vector<double> aRms; // RMS (m)

			void setGrid(const Grid& grid, bool bRms=false) {
				naPcv.resize(grid.nz, NAN);
				aPcv.resize(grid.na * grid.nz, NAN);
				if (bRms)
					aRms.resize(grid.na * grid.nz, NAN);
			}

			// z in degrees, return PCV from ANTEX files in meters. 
			double getPcv(double z) const;
			// a and z are in degrees
			double getPcv(double z, double a) const;
			double& pcv(int nz, int na) {
				return aPcv[na * ant->m_grid.nz + nz];
			}
			double pcv(int nz, int na) const {
				return aPcv[na * ant->m_grid.nz + nz];
			}
			double& pcv(int nz) {
				return naPcv[nz];
			}
			double pcv(int nz) const {
				return naPcv[nz];
			}

			bool hasRms() const {
				return aRms.size() > 0;
			}

			void serialize(Archive& ar)
			{
				DWORD dwVer = 1;
				if (ar.isStoring())
				{
					ar << dwVer;
					ar.write(&es, sizeof(es));
					ar << pco;
					ar << pcoRms;
					ar << naPcv;
					ar << aPcv;
					ar << naRms;
					ar << aRms;
				}
				else
				{
					ar >> dwVer;
					ar.read(&es, sizeof(es));
					ar >> pco;
					ar >> pcoRms;
					ar >> naPcv;
					ar >> aPcv;
					ar >> naRms;
					ar >> aRms;
				}
			}
		};
		std::map<Gnss::Signal, SignalData> m_sigs;

	// Operations:
	public:
		void normalizePcv(); // Set PCV to be 0 at zenith (subtract zenith PCV)

		// Overrides:
	public:
		virtual std::string getName() const { return getFullName(); }
		virtual bool hasPcc(int es) const {
			return m_sigs.find((Gnss::Signal)es) != m_sigs.end();
		}
		virtual bool hasPcvRms(int es) const {
			auto it = m_sigs.find((Gnss::Signal)es);
			if (it == m_sigs.end())
				return false;
			return it->second.hasRms();
		}
		virtual Node* subtract(const Node* pMinus) const;

		// Return PCV in meters. The sign of the returned value
		// is correct, i.e. it is value from ANTEX file multiplied by -1 and devided by 1000. 
		virtual double getPcv(can2::Gnss::Signal es, double zen, double az) const;

		// Return PCC in meters 
		virtual double getPcc(can2::Gnss::Signal es, double zen, double az) const;

		// Compute phase center offset from PCC. See the document "PCC definition.pdf"
		virtual Point3d calcOffset(can2::Gnss::Signal es, double eleMask, OffsetMode em, double * pro=nullptr) const;

		// Returns offset loaded from the ANTEX file
		virtual Point3d getOffset(can2::Gnss::Signal es) const { 
			if (!hasPcc(es))
				return Point3d(NAN, NAN, NAN);
			return m_sigs.at(es).pco;
		}

		double calcNorm(Gnss::Signal es, OffsetMode em, bool bSimple = false, int root = -1) const;
		double calcNormSimple(Gnss::Signal es, OffsetMode em) const;

		// Serialization:
	public:
		virtual void serialize(Archive& ar);

	};
}
