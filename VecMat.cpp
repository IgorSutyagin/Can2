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

#include "VecMat.h"

using namespace can2;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vector implementation 

void Vector::getSubVector (Vector& v, int n0, int n1) const
{
	v.resize(n1 - n0);

	for (int i = n0; i < n1; i++)
	{
		v[i - n0] = (*this)[i];
	}
}

// End of Vector implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Matrix implementation

Vector Matrix::solve(const Vector& b) const
{
	assert(rows() == cols());
	assert(b.len() == rows());

	Matrix t(*this);
	Vector v(b);
	Vector x(cols());

	// Excluding Method
	for (int k = 0; k < cols() - 1; k++)
	{
		if (fabs(t(k, k)) == 0.0)
		{
			throw new MatrixException("Matrix::solve: Matrix is singular");
		}
		for (int i = k + 1; i < rows(); i++)
		{
			double d = t(i, k) / t(k, k);
			t(i, k) = 0.0;
			for (int j = k + 1; j < cols(); j++)
			{
				t(i, j) = t(i, j) - d * t(k, j);
			}
			v(i) = v(i) - d * v(k);
		}
	}

	// Back ...
	x(rows() - 1) = v(rows() - 1) / t(rows() - 1, cols() - 1);

	for (int i = rows() - 2; i >= 0; i--)
	{
		double s = 0.0;
		for (int j = i + 1; j < rows(); j++)
		{
			s += t(i, j) * x(j);
		}
		x(i) = (v(i) - s) / t(i, i);
	}

	return x;
}

void Matrix::inverse()
{
	if (rs != cs || rs == 0)
		throw new MatrixException("Matrix::inverse: Matrix is not square or matrix is empty");

	int M = rs;
	int i, j;

	Matrix E (M, M);

	// E is a E-matrix at the beginning
	for (i = 0; i < M; i++)
		for (j = 0; j < M; j++)
			if (i == j)
				E(i, j) = 1.0;
			else
				E(i, j) = 0.0;

	int* nOrgCol = NULL;
	nOrgCol = new int[M];
	if (nOrgCol == NULL)
		// Never get here because of
		// throwing MemoryException
		return;

	// i-th item contains original column number
	for (i = 0; i < M; i++)
		nOrgCol[i] = i;

	double xNull = 0.0;
	double xDiag = 0.0;
	// Convert matrix to a E-matrix
	// and get inverse matrix in Tmp.
	double* pMatrix = mat.data();
	for (i = 0; i < M; i++)
	{
		double* pRow = pMatrix + M * i;
		// Get diagonal item in current row:
		double* pDiag = pMatrix + M * i + i;
		if (*pDiag == xNull)
		{// Exchange columns:

			// Find appropriate column
			for (j = i + 1; j < M; j++)
				if (*(pRow + j) != xNull)
					break;

			if (j == M) { // Nothing could be done.
				delete[] nOrgCol;
				throw new MatrixException("Matrix::inverse: Matrix is singular");
			}

			exchCol(i, j);
			E.exchCol(i, j);
			int nTmp = nOrgCol[i];
			nOrgCol[i] = nOrgCol[j];
			nOrgCol[j] = nTmp;

		}

		// Normalize i-th row
		xDiag = 1.0 / *pDiag;
		multRowByVal(i, xDiag);
		E.multRowByVal(i, xDiag);

		// Process all rows bellow this one
		// to make them like E-matrix rows:
		for (j = i + 1; j < M; j++)
		{
			double* px = pMatrix + M * j + i;
			double xMult = -*px;
			addRows(j, i, xMult);
			E.addRows(j, i, xMult);
		}

		// The same action above this row:
		for (j = i - 1; j >= 0; j--)
		{
			double* px = pMatrix + M * j + i;
			double xMult = -*px;
			addRows(j, i, xMult);
			E.addRows(j, i, xMult);
		}
	}

	for (i = 0; i < M; i++)
	{
		if (nOrgCol[i] == i)
			continue;
		for (j = i + 1; j < M; j++)
		{
			if (i == nOrgCol[j])
			{
				E.exchCol(i, j);
				nOrgCol[j] = nOrgCol[i];
				nOrgCol[i] = i;
				break;
			}
		}
		ASSERT(j < M);
	}

	*this = E;
	delete[] nOrgCol;
}

void Matrix::exchRow(int nRow1, int nRow2)
{
	if (nRow1 == nRow2)
		return;

	for (int i = 0; i < cs; i++)
	{
		std::swap((*this)(nRow1, i), (*this)(nRow2, i));
	}
}

void Matrix::exchCol(int nCol1, int nCol2)
{
	if (nCol1 == nCol2)
		return;

	for (int i = 0; i < rs; i++)
	{
		double t = (*this)(i, nCol1);
		(*this)(i, nCol1) = (*this)(i, nCol2);
		(*this)(i, nCol2) = t;
	}
}

void Matrix::multRowByVal(int nDst, double xMult)
{
	for (int i = 0; i < cs; i++)
		(*this)(nDst, i) *= xMult;
}

void Matrix::addRows(int nDst, int nSrc, double mult)
{
	double* pMatrix = mat.data();
	double* pSrcRow = pMatrix + cs * nSrc;
	double* pDstRow = pMatrix + cs * nDst;

	for (int i = 0; i < cs; i++)
		*(pDstRow + i) += *(pSrcRow + i) * mult;
}

void Matrix::copyUpperTriangle()
{
	for (int i = 0; i < rows(); i++)
	{
		for (int j = 0; j < i; j++)
		{
			(*this)(i, j) = (*this)(j, i);
		}
	}
}



// End of Matrix implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////
