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

#include "Tools.h"
#include "Gnss.h"
#include "VecMat.h"
#include "AntexAntenna.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

using namespace can2;

//////////////////////////////////////////////////////////////////////////////////////////////////
// AntexAntenna implementation

AntexAntenna& AntexAntenna::operator=(const AntexAntenna& a)
{
	*(Node*)this = *(Node*)&a;
	m_type = a.m_type;
	m_radome = a.m_radome;
	m_serial = a.m_serial;
	m_method = a.m_method;
	m_agency = a.m_agency;
	m_numIndAnt = a.m_numIndAnt;
	m_date = a.m_date;
	m_bDifference = a.m_bDifference;
	m_grid = a.m_grid;
	m_sigs = a.m_sigs;
	std::for_each(m_sigs.begin(), m_sigs.end(), [=](std::pair<const Gnss::Signal, SignalData>& a) { a.second.ant = this; });
	return *this;
}

double AntexAntenna::SignalData::getPcv(double z) const
{
	int n0 = (int)floor(z / ant->m_grid.step.zen);
	int n1 = n0 + 1;
	if (n1 == (int)naPcv.size())
		n1 = naPcv.size() - 1;
	if (n0 < 0 || (int)naPcv.size() - 1 < n1)
		throw std::range_error("zenith angle is out of range");

	double y0 = naPcv[n0];
	double y1 = naPcv[n1];

	return (y1 * (z - n0 * ant->m_grid.step.zen) + y0 * (n1 * ant->m_grid.step.zen - z)) / ant->m_grid.step.zen;
}

double AntexAntenna::SignalData::getPcv(double z, double a) const
{
	if (ant->m_grid.za1.zen < z && z < ant->m_grid.za1.zen + 0.001)
		z = ant->m_grid.za1.zen;
	int z0 = (int)floor(z / ant->m_grid.step.zen);
	int z1 = z0 + 1;
	if (z == ant->m_grid.za1.zen)
		z1 = z0;

	if (ant->m_grid.step.az <= 0)
		return getPcv(z);

	if (z0 < 0 || ant->m_grid.nz - 1 < z1)
		throw std::range_error("zenith angle is out of range");
	int a0 = (int)floor(a / ant->m_grid.step.az);
	int a1 = a0 + 1;
	if (a == ant->m_grid.za1.az)
		a1 = a0;
	if (a0 < 0 || ant->m_grid.na - 1 < a1)
		throw std::range_error("azimuth angle is out of range");

	double pcv0 = NAN;
	double pcv1 = NAN;

	if (z1 != z0)
	{
		double y00 = aPcv[a0 * ant->m_grid.nz + z0];
		double y01 = aPcv[a0 * ant->m_grid.nz + z1];
		double y10 = aPcv[a1 * ant->m_grid.nz + z0];
		double y11 = aPcv[a1 * ant->m_grid.nz + z1];

		pcv0 = (y01 * (z - z0 * ant->m_grid.step.zen) + y00 * (z1 * ant->m_grid.step.zen - z)) / ant->m_grid.step.zen;
		pcv1 = (y11 * (z - z0 * ant->m_grid.step.zen) + y10 * (z1 * ant->m_grid.step.zen - z)) / ant->m_grid.step.zen;
	}
	else // Case when z = m_grid.za1.zen
	{
		pcv0 = aPcv[a0 * ant->m_grid.nz + z0];
		pcv1 = aPcv[a1 * ant->m_grid.nz + z0];
	}

	if (a1 != a0)
		return (pcv1 * (a - a0 * ant->m_grid.step.az) + pcv0 * (a1 * ant->m_grid.step.az - a)) / ant->m_grid.step.az;
	else // Case when a = m_grid.za1.az
		return pcv0;
}

///////////////////////////////
// Operations:
void AntexAntenna::normalizePcv()
{
	for (auto its = m_sigs.begin(); its != m_sigs.end(); its++)
	{
		SignalData& s = its->second;
		double val0 = NAN;
		for (int i=0; i<(int)s.naPcv.size(); i++)
		{
			if (isnan(val0))
				val0 = s.getPcv(0);
			s.naPcv[i] -= val0;
		}

		val0 = NAN;
		for (int na = 0; na < m_grid.na; na++)
		{
			for (int nz = 0; nz < m_grid.nz; nz++)
			{
				if (isnan(val0))
					val0 = s.getPcv(0, 0);
				s.pcv(nz, na) -= val0;
			}
		}
	}
}

////////////////////////////////
// Implementation
Node* AntexAntenna::subtract(const Node* pMinus) const
{
	if (pMinus->isAntenna())
	{
		can2::AntexAntenna* pam = (can2::AntexAntenna*)pMinus;
		AntexAntenna* par = new can2::AntexAntenna();
		par->m_type = stringFormat("%s - %s", getName().c_str(), pMinus->getName().c_str());
		par->m_bDifference = true;
		par->m_grid = m_grid;

		for (int nf = Gnss::G01; nf < Gnss::esigInvalid; nf++)
		{
			if (!pMinus->hasPcc(nf) || !hasPcc(nf))
				continue;

			Gnss::Signal es = (Gnss::Signal)nf;

			double wl = Gnss::getSysWl(nf);
			const SignalData& sp = m_sigs.at(es);
			const SignalData& sm = pam->m_sigs.at(es);

			SignalData& s = par->m_sigs.emplace(es, SignalData(es, par, Point3d(0, 0, 0))).first->second;

			Point3d pcod = sp.pco - sm.pco;
			s.pco = Point3d(0, 0, 0); // pcod;
			double val0 = NAN;
			int na = 0;
			for (double az = m_grid.za0.az; az < m_grid.za1.az + m_grid.step.az/2; az += m_grid.step.az, na++)
			{
				int nz = 0;
				for (double zen = m_grid.za0.zen; zen < m_grid.za1.zen + m_grid.step.zen/2; zen += m_grid.step.zen, nz++)
				{
					ZenAz za(zen, az);
					s.pcv(nz, na) = sp.pcv(nz, na) - sm.getPcv(zen, az) + pcod * za.dirCos();
					if (isnan(val0))
						val0 = s.pcv(nz, na);
					s.pcv(nz, na) -= val0;
					s.pcv(nz) += s.pcv(nz, na);
				}
			}

			for (int nz = 0; nz < m_grid.nz; nz++)
				s.pcv(nz) /= m_grid.na;

		}

		return par;
	}

	return nullptr;
}

double AntexAntenna::getPcv(can2::Gnss::Signal es, double zen, double az) const
{
	// The sign of PCV in SignalData is correct, so do not change sign here. PCV are in meters
	return m_sigs.at(es).getPcv(zen, az);
}

double AntexAntenna::getPcc(can2::Gnss::Signal es, double zen, double az) const
{
	double pcv = getPcv(es, zen, az);
	double pco = m_sigs.at(es).pco * ZenAz(zen, az).dirCos();
	return pco + pcv;
}

Point3d AntexAntenna::calcOffset(can2::Gnss::Signal es, double eleMask, OffsetMode em, double * pro) const
{
	if (!hasPcc(es))
		return Point3d(NAN, NAN, NAN);

	if (90 - m_grid.za1.zen > eleMask)
		eleMask = 90 - m_grid.za1.zen;

	double azStep = 1;
	double zenStep = m_grid.step.zen;
	double wl = Gnss::getSysWl(es);
	Point3d oldOffset = getOffset(es);

	auto intCosSinEN = [](double t0, double t1, double p0, double p1) {
		double c30 = cos(3 * t0);
		double s30 = sin(3 * t0);
		double c31 = cos(3 * t1);
		double s31 = sin(3 * t1);
		double c0 = cos(t0);
		double c1 = cos(t1);
		double s0 = sin(t0);
		double s1 = sin(t1);
		double ans =
			(
				+(p0 - p1) * c30
				- (p0 - p1) * c31
				- 9 * (p0 - p1) * c0
				+ 9 * (p0 - p1) * c1
				- 9 * p0 * (t0 - t1) * s0
				+ 9 * p1 * (t0 - t1) * s1
				+ 3 * p0 * (t0 - t1) * s30
				- 3 * p1 * (t0 - t1) * s31
				) / (36 * (t0 - t1));
		return ans;
	};
	auto intSinEN = [](double t0, double t1, double p0, double p1) {
		double c20 = cos(2 * t0);
		double s20 = sin(2 * t0);
		double c21 = cos(2 * t1);
		double s21 = sin(2 * t1);
		double dt = t1 - t0;
		double ans =
			(
				+(p1 - p0) * c20
				- (p1 - p0) * c21
				+ 2 * (p0 + p1) * dt * dt
				+ 2 * p0 * dt * s20
				- 2 * p1 * dt * s21
				) / (8 * dt);
		return ans;
	};

	auto intNoneEN = [](double t0, double t1, double p0, double p1) {
		double c0 = cos(t0);
		double s0 = sin(t0);
		double c1 = cos(t1);
		double s1 = sin(t1);
		double dt = t1 - t0;
		double ans =
			(
				+(p0 - p1) * s0
				- (p0 - p1) * s1
				+ p0 * dt * c0
				- p1 * dt * c1
				) / dt;
		return ans;
	};

	auto intCosSinU = [](double t0, double t1, double p0, double p1) {
		double s0 = sin(t0);
		double c0 = cos(t0);
		double s1 = sin(t1);
		double c1 = cos(t1);
		double ans =
			-(
				+2 * (p0 - p1) * s0
				- 2 * (p0 - p1) * s1
				- 3 * p0 * (t0 - t1) * c0 * c0 * c0
				+ 3 * p1 * (t0 - t1) * c1 * c1 * c1
				+ (p0 - p1) * c0 * c0 * s0
				- (p0 - p1) * c1 * c1 * s1
				) / (9 * (t0 - t1));
		return ans;
	};

	auto intSinU = [](double t0, double t1, double p0, double p1) {
		double s20 = sin(2 * t0);
		double c20 = cos(2 * t0);
		double s21 = sin(2 * t1);
		double c21 = cos(2 * t1);
		double dt = t1 - t0;
		double ans =
			(
				+(p0 - p1) * s20
				- (p0 - p1) * s21
				+ 2 * p0 * dt * c20
				- 2 * p1 * dt * c21
				) / (8 * dt);
		return ans;
	};

	auto intNoneU = [](double t0, double t1, double p0, double p1) {
		double s0 = sin(t0);
		double c0 = cos(t0);
		double s1 = sin(t1);
		double c1 = cos(t1);
		double dt = t1 - t0;
		double ans =
			(
				+(p0 - p1) * c0
				- (p0 - p1) * c1
				- p0 * dt * s0
				+ p1 * dt * s1
				) / dt;
		return ans;
	};

	auto intCosSinRo = [](double t0, double t1, double p0, double p1) {
		double c0 = cos(2 * t0);
		double s0 = sin(2 * t0);
		double c1 = cos(2 * t1);
		double s1 = sin(2 * t1);
		double dt = t1 - t0;
		double ans =
			(
				+(p0 - p1) * (s0 - s1)
				+ 2 * p0 * dt * c0
				- 2 * p1 * dt * c1
				) / (8 * dt);
		return ans;
	};

	auto intSinRo = [](double t0, double t1, double p0, double p1) {
		double c0 = cos(t0);
		double s0 = sin(t0);
		double c1 = cos(t1);
		double s1 = sin(t1);
		double dt = t1 - t0;
		double ans =
			(
				+(p0 - p1) * (s0 - s1)
				+ p0 * dt * c0
				- p1 * dt * c1
				) / dt;
		return ans;
	};

	auto intNoneRo = [](double t0, double t1, double p0, double p1) {
		double ans =
			-((p0 + p1) * (t0 - t1)) / 2;
		return ans;
	};

	auto intTheta = [=](double az, double dt, double intEle[4], double eleMaskDeg) {
		double ca = cos(az * PI / 180);
		double sa = sin(az * PI / 180);
		double pcv0 = getPcv(es, 0, az) / wl;
		double zenMaskDeg = 90 - eleMaskDeg;
		for (double zenDeg = zenStep; zenDeg < 90 + zenStep / 2; zenDeg += zenStep)
		{
			double t0 = (zenDeg - zenStep) * PI / 180;
			double zen = zenDeg < zenMaskDeg ? zenDeg : zenMaskDeg;
			double t1 = zen * PI / 180;
			if (t1 == t0)
				break;
			double ele0 = 90 - t0 * 180 / PI;
			double ele = 90 - zen;
			if (ele < 0)
				ele = 0;
			double ce = cos(ele * PI / 180);
			double se = sin(ele * PI / 180);
			double pcv1 = getPcv(es, zen, az) / wl;
			intEle[0] += em == eSinAndCos ? intCosSinEN(t0, t1, pcv0, pcv1) :
				em == eOnlyCos ? intSinEN(t0, t1, pcv0, pcv1) :
				intNoneEN(t0, t1, pcv0, pcv1);
			intEle[2] += em == eSinAndCos ? intCosSinU(t0, t1, pcv0, pcv1) :
				em == eOnlyCos ? intSinU(t0, t1, pcv0, pcv1) :
				intNoneU(t0, t1, pcv0, pcv1);
			intEle[3] += em == eSinAndCos ? intCosSinRo(t0, t1, pcv0, pcv1) :
				em == eOnlyCos ? intSinRo(t0, t1, pcv0, pcv1) :
				intNoneRo(t0, t1, pcv0, pcv1);
			pcv0 = pcv1;

			if (zen != zenDeg)
				break;
		}
		intEle[1] = intEle[0];
	};

	double intAz[4] = { 0, 0, 0, 0 };
	double dt = zenStep * PI / 180;
	double dfi = azStep * PI / 180;

	double sq3 = sqrt(3);
	for (double az = 0; az < 360 - azStep / 2; az += azStep)
	{
		double az0 = az + azStep / 2 - azStep / (sq3 * 2);
		double az1 = az + azStep / 2 + azStep / (sq3 * 2);
		double ca0 = cos(az0 * PI / 180);
		double sa0 = sin(az0 * PI / 180);
		double ca1 = cos(az1 * PI / 180);
		double sa1 = sin(az1 * PI / 180);

		double intEle0[4] = { 0, 0, 0, 0 };
		double intEle1[4] = { 0, 0, 0, 0 };
		intTheta(az0, dt, intEle0, eleMask);
		intTheta(az1, dt, intEle1, eleMask);

		intAz[0] += (intEle0[0] * sa0 + intEle1[0] * sa1) / 2;
		intAz[1] += (intEle0[1] * ca0 + intEle1[1] * ca1) / 2;
		intAz[2] += (intEle0[2] + intEle1[2]) / 2;
		intAz[3] += (intEle0[3] + intEle1[3]) / 2;
	}

	for (int i = 0; i < 4; i++)
		intAz[i] *= dfi;


	Matrix a(4, 4);

	double cm = cos(eleMask*PI / 180); // 1.0; // 
	double sm = sin(eleMask*PI / 180); // 0.0; // 
	if (em == eSinAndCos)
	{
		a(0, 0) = PI * cm * cm * cm * cm / 4;
		a(1, 1) = PI * cm * cm * cm * cm / 4;
		a(2, 2) = PI * (1 - sm * sm * sm * sm) / 2;
		a(2, 3) = 2 * PI * (1 - sm * sm * sm) / 3;
		a(3, 3) = PI * cm * cm;
		a(3, 2) = 2 * PI * (1 - sm * sm * sm) / 3;
	}
	else if (em == eOnlyCos) // bWeightSin = false && bNoWeight = false
	{
		a(0, 0) = PI * (sm - 1) * (sm - 1) * (sm + 2) / 3;
		a(1, 1) = PI * (sm - 1) * (sm - 1) * (sm + 2) / 3;
		a(2, 2) = 2 * PI * (1 - sm * sm * sm) / 3;
		a(2, 3) = PI * cm * cm;
		a(3, 3) = 2 * PI * (1 - sm);
		a(3, 2) = PI * cm * cm;
	}
	else if (em == eNoWeight) // bNoWeight = true
	{
		//double eleMask = 0;
		a(0, 0) = -(PI * (2 * eleMask * PI / 180 - PI + sin(2 * eleMask * PI / 180))) / 4;
		a(1, 1) = -(PI * (2 * eleMask * PI / 180 - PI + sin(2 * eleMask * PI / 180))) / 4;
		a(2, 2) = (PI * (PI - 2 * eleMask * PI / 180 + sin(2 * eleMask * PI / 180))) / 2;
		a(2, 3) = 2 * PI * cm;
		a(3, 2) = 2 * PI * cm;
		a(3, 3) = PI * PI - 2 * PI * eleMask * PI / 180;
	}
	else
	{
		return Point3d(NAN, NAN, NAN);
	}

	// Add to computed integrals of PCV integrals of old PCO
	{
		Vector oldPco(4);
		oldPco(0) = oldOffset.e / wl;
		oldPco(1) = oldOffset.n / wl;
		oldPco(2) = oldOffset.u / wl;
		oldPco(3) = 0;
		Vector pcoCor = a * oldPco;

		for (int i = 0; i < 4; i++)
		{
			intAz[i] += pcoCor(i);
		}
	}

	// Solve equations
	Vector b(4);
	for (int i = 0; i < 4; i++)
		b(i) = intAz[i];

	Vector x = a.solve(b);

	//{
	//	CRVector y(4);
	//	a.VMult(y, x);
	//	CRVector res = b;
	//	res -= y;
	//	std::cout << res[0] << "," << res[1] << "," << res[2] << "," << res[3] << std::endl;
	//}

	double r = x(3) * wl;
	if (pro != nullptr)
		*pro = r;
	return Point3d(x(0) * wl, x(1) * wl, x(2) * wl);
}

Point3d AntexAntenna::getOffset(Gnss::Signal es, double* pro) const
{
	if (!hasPcc(es))
		return Point3d(NAN, NAN, NAN);

	if (pro == nullptr)
		return m_sigs.at(es).pco;

	Point3d pco = m_sigs.at(es).pco;

	*pro = 0;
	return pco;
}

double AntexAntenna::calcNorm(Gnss::Signal es, OffsetMode em, bool bSimple, int root) const
{
	double ro = 0;
	Point3d off = calcOffset(es, 0.0, em, &ro);
	double wl = Gnss::getSysWl(es);

	double r = 0;
	double a = 0;
	double pcvBypcc = 0;

	if (root < 0)
	{
		root = (ro * off.u >= 0) ? 1 : 0;
	}

	if (em == eSinAndCos)
	{
		r = root == 0 ? (4 + sqrt(7.0)) / 6.0 : (4 - sqrt(7.0)) / 6.0;
		a = 2 / sqrt(PI);
		pcvBypcc = 2 * PI * sqrt(7) / 6 * off.u * ro;
		if (root == 0)
			pcvBypcc *= -1;
	}
	else if (em == eOnlyCos)
	{
		r = root == 0 ? 1 : 0;
		a = sqrt(3 / (2 * PI));
		pcvBypcc = 2 * PI * off.u * ro;
		if (root == 0)
			pcvBypcc *= -1;
	}
	else if (em == eNoWeight)
	{
		r = root == 0 ? (4 + sqrt(16 - PI * PI)) / (2 * PI) : (4 - sqrt(16 - PI * PI)) / (2 * PI);
		a = 2 / PI;
		pcvBypcc = 2 * PI * sqrt(16 - PI * PI) / 2 * ro * off.u;
		if (root == 0)
			pcvBypcc *= -1;
	}

	if (bSimple)
		pcvBypcc = 0;

	auto intCosSin = [](double t0, double t1, double p0, double p1) {
		double c20 = cos(2 * t0);
		double c21 = cos(2 * t1);
		double s20 = sin(2 * t0);
		double s21 = sin(2 * t1);
		double dp = p1 - p0;
		double dt = t1 - t0;
		double ans =
			-(
				+(dp * dp - 2 * p0 * p0 * dt * dt) * c20
				- (dp * dp - 2 * p1 * p1 * dt * dt) * c21

				+ 2 * p0 * dp * dt * s20
				- 2 * dp * p1 * dt * s21
				) / (8 * dt * dt);
		return ans;
	};

	auto intSin = [](double t0, double t1, double p0, double p1) {
		double dt = t1 - t0;
		double dp = p1 - p0;
		double c0 = cos(t0);
		double s0 = sin(t0);
		double c1 = cos(t1);
		double s1 = sin(t1);
		double ans =
			+p1 * p1 * c0
			- p1 * p1 * c1
			+ (2 * c1 * dp * dp) / (dt * dt)
			- (dp * dp * (c0 * (2 - dt * dt) - 2 * dt * s0)) / (dt * dt)
			- (2 * p1 * dp * (s0 + c0 * dt)) / dt
			+ (2 * p1 * s1 * dp) / dt;
		return ans;
	};

	auto intNone = [](double t0, double t1, double p0, double p1) {
		return	(t1 - t0) * (p0 * p0 + p0 * p1 + p1 * p1) / 3;
	};

	auto intEle = [=](double az, double dt) {
		double ca = cos(az * PI / 180);
		double sa = sin(az * PI / 180);
		double pcc = Point3d(0, 0, sin(PI / 2)) * off;
		double p0 = 0;
		double pcv0 = getPcc(es, 0, az) - pcc; // -r*off.u;
		double sum = 0;
		std::vector<Point2d> debug;
		if (az == 0)
			debug.push_back(Point2d(90, p0));
		for (double t = dt; t < PI / 2 + dt / 2; t += dt)
		{
			double zen = t * 180 / PI;
			double ele = 90 - zen;
			double pcc = Point3d(sa * sin(t), ca * sin(t), cos(t)) * off;
			double p1 = getPcc(es, zen, az) - pcc - pcv0; //  - r*off.u
			if (az == 0)
				debug.push_back(Point2d(ele, p1));
			double t0 = t - dt;
			double t1 = t;
			double d = em == eSinAndCos ? intCosSin(t0, t1, p0, p1) :
				em == eOnlyCos ? intSin(t0, t1, p0, p1) : intNone(t0, t1, p0, p1);
			sum += d;
			p0 = p1;
		}
		if (az == 0)
		{
			std::sort(debug.begin(), debug.end(), [](const Point2d& a, const Point2d& b) {
				return a.x < b.x;
			});
		}
		return sum;
	};

	double daz = 2;
	double sumPcv2 = 0;
	double dt = m_grid.step.zen * PI / 180;
	double sq3 = sqrt(3);
	//intEle(0, dt);
	for (double az = 0; az < 360 - daz / 2; az += daz)
	{
		double az0 = az + daz / 2 - daz / (sq3 * 2);
		double az1 = az + daz / 2 + daz / (sq3 * 2);
		double v0 = intEle(az0, dt);
		double v1 = intEle(az1, dt);
		sumPcv2 += (v0 + v1) / 2;
	}

	sumPcv2 *= daz * PI / 180;

	double pco2 = off * off / (a * a);

	double sigma = a * sqrt(pco2 + pcvBypcc + sumPcv2);
	return sigma;

}

void AntexAntenna::serialize(Archive& ar)
{
	DWORD dwVer = 1;
	if (ar.isStoring())
	{
		ar << dwVer;
		ar << m_type;
		ar << m_serial;
		ar << m_method;
		ar << m_agency;
		ar << m_numIndAnt;
		ar << m_date;
		ar << m_bDifference;
		m_grid.serialize(ar);

		size_t ns = m_sigs.size();
		ar << ns;
		for (auto it = m_sigs.begin(); it != m_sigs.end(); it++)
		{
			ar.write(&(it->first), sizeof(it->first));
			it->second.serialize(ar);
		}
	}
	else
	{
		ar >> dwVer;
		ar >> m_type;
		ar >> m_serial;
		ar >> m_method;
		ar >> m_agency;
		ar >> m_numIndAnt;
		ar >> m_date;
		ar >> m_bDifference;
		m_grid.serialize(ar);

		size_t ns = 0;
		ar >> ns;
		m_sigs.clear();
		for (size_t i=0; i<ns; i++)
		{
			Gnss::Signal es;
			ar.read(&es, sizeof(es));
			SignalData s(es, this, Point3d(NAN, NAN, NAN));
			s.serialize(ar);
			m_sigs[es] = s;
		}
	}
}
// End of AntexAntenna implementation
//////////////////////////////////////////////////////////////////////////////////////////////////
