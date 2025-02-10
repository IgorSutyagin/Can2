#include "pch.h"
#include "Can2.h"

#include "Tex2Csv.h"
#include "RingIFView.h"

using namespace can2;

bool Tex2Csv::convert(const char * szAnt, const char* szTex, std::vector<CRingIFView::PppItem>& pppItems)
{
	std::ifstream ifs(szTex);
	if (!ifs.good())
		return nullptr;

	const char* szSeps = " \t\r\nGREC$&\\pm";
	const char* labs[11] = { "IGS", "DLR", "ETH", "GPP4", "GPP8", "GSA", "IFE", "IGG", "NGS", "TPS", "WHU" };
	std::string line;
	while (std::getline(ifs, line))
	{
		const char* pLine = line.c_str();
		std::vector<int> nLabs;
		pLine += strspn(pLine, "\\ &");
		for (int i = 0; i < 11; i++)
		{
			if (strstr(pLine, labs[i]) == pLine)
			{
				nLabs.push_back(i);
				break;
			}
		}

		if (nLabs.size() == 0 || nLabs.size() > 2)
			continue;

		pLine += strcspn(pLine, "\\ &");
		pLine += strspn(pLine, "\\ &");
		for (int i = 0; i < 11; i++)
		{
			if (strstr(pLine, labs[i]) == pLine)
			{
				nLabs.push_back(i);
				break;
			}
		}

		while (std::getline(ifs, line))
		{
			if (line.find("\\cmidrule", 0) != std::string::npos)
				continue;

			const char* szSysLabels[4] = { "G", "R", "E", "C" };
			int sys = -1;
			for (int i = 0; i < 4; i++)
			{
				if (line.find(szSysLabels[i], 0) == 0)
				{
					sys = i;
					break;
				}
			}

			ASSERT(sys >= 0);

			const char* p = line.c_str();
			p += strspn(p, szSeps);
			Point3d pco;
			pco.n = *p == 'n' ? NAN : atof(p);
			p += strcspn(p, szSeps);
			p += strspn(p, szSeps);
			p += strcspn(p, szSeps);
			p += strspn(p, szSeps);
			pco.e = *p == 'n' ? NAN : atof(p);
			p += strcspn(p, szSeps);
			p += strspn(p, szSeps);
			p += strcspn(p, szSeps);
			p += strspn(p, szSeps);
			pco.u = *p == 'n' ? NAN : atof(p);

			CRingIFView::PppItem pppItem;
			pppItem.ant = szAnt;
			pppItem.lab = labs[nLabs[0]];
			pppItem.sys = szSysLabels[sys];
			pppItem.enu = pco;

			pppItems.push_back(pppItem);

			if (nLabs.size() < 2)
			{
				if (sys == 3)
					break;
				continue;
			}

			p += strcspn(p, szSeps);
			p += strspn(p, szSeps);
			p += strcspn(p, szSeps);
			p += strspn(p, szSeps);

			pco.n = *p == 'n' ? NAN : atof(p);
			p += strcspn(p, szSeps);
			p += strspn(p, szSeps);
			p += strcspn(p, szSeps);
			p += strspn(p, szSeps);
			pco.e = *p == 'n' ? NAN : atof(p);
			p += strcspn(p, szSeps);
			p += strspn(p, szSeps);
			p += strcspn(p, szSeps);
			p += strspn(p, szSeps);
			pco.u = *p == 'n' ? NAN : atof(p);

			pppItem.ant = szAnt;
			pppItem.lab = labs[nLabs[1]];
			pppItem.sys = szSysLabels[sys];
			pppItem.enu = pco;
			pppItems.push_back(pppItem);

			if (sys == 3)
				break;
		}
	}

	return pppItems.size() > 0;
}