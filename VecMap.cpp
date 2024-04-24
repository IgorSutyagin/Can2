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


// End of Matrix implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////
