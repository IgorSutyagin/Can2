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

#include "Can2Exception.h"

namespace can2
{
	class Vector
	{
		// Construction:
	public:
		Vector() {}
		Vector(const Vector& a) : data(a.data) {}
		Vector(Vector&& a) noexcept : data(a.data) {}
		Vector(int len) : data(len, 0) {}
		virtual ~Vector() {}
		void clear() {
			data.clear();
		}

	// Operations:
	public:
		int len() const {
			return data.size();
		}
		Vector& operator=(const Vector& a) {
			*this = a;
			return *this;
		}
		Vector& operator=(Vector&& a) noexcept {
			*this = a;
			return *this;
		}
		double operator()(int i) const { return data[i]; }
		double& operator()(int i) {	return data[i];	}

	// Implementation:
	protected:
		std::vector<double> data;
	};

	class Matrix
	{
	// Construction:
	public:
		Matrix() : rs(0), cs(0) {}
		Matrix(const Matrix& a) : rs(a.rs), cs(a.cs), mat(a.mat) {}
		Matrix(Matrix&& a) noexcept : rs(a.rs), cs(a.cs), mat(a.mat) {}
		Matrix(int rows_, int cols_) : rs(rows_), cs(cols_), mat(rows_* cols_, 0) {}
		~Matrix() {}
		void clear() {
			mat.clear();
			cs = rs = 0;
		}

	// Operations:
	public:
		int cols() const {
			return cs;
		}
		int rows() const {
			return rs;
		}
		Matrix& operator=(const Matrix& a) {
			cs = a.cs; rs = a.rs; mat = mat;
			return *this;
		}
		Matrix& operator=(Matrix&& a) noexcept {
			cs = a.cs; a.cs = 0;
			rs = a.rs; a.rs = 0;
			mat = a.mat;
			return *this;
		}
		double& operator()(int row, int col) {	return mat[cs* row + col];	}
		double operator()(int row, int col) const { return mat[cs * row + col]; }
		friend Vector operator*(const Matrix& m, const Vector& v) {
			assert(m.cols() == v.len());
			Vector r(v.len());
			for (int i = 0; i < r.len(); i++) {
				double& d = r(i);
				for (int j = 0; j < m.cols(); j++) {
					d += m(i, j) * v(j);
				}
			}
			return r;
		}
		friend Matrix operator*(const Matrix& a, const Matrix& b) {
			assert(a.cols() == b.rows());
			Matrix r(a.rows(), b.cols());
			for (int m = 0; m < r.rows(); m++) {
				for (int n = 0; n < r.cols(); n++) {
					double& d = r(m, n);
					for (int k = 0; k < a.cols(); k++) {
						d += a(m, k) * b(k, n);
					}
				}
			}
			return r;
		}

		// b = A*x
		Vector solve(const Vector& b) const;

	// Implementation:
	protected:
		std::vector<double> mat;
		int rs;
		int cs;
	};

}