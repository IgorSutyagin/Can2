# Can2: An Open Source Program for GNSS Absolute Antenna Calibration Analysis 

OVERIEW

Can2 software was developed while working on The IGS Antenna Ring Campaign project and 
is designed to view, analyze, compare absolute calibrations of GNSS antennas

The results of absolute antenna calibrations are loaded into the program from files in 
the ANTEX format. The program provides tools for viewing phase center variations (PCV), 
calculates phase center offsets (PCO) using the standard procedure described in 
the documentation, and allows you to analyze their frequency dependencies. 

Calibration data loaded into the program can be subtracted from each other and the 
difference phase characteristics can be analyzed.

To compare several calibrations of the same antenna, or antennas of the same type, 
Can2 software allows you to create special data sets, save them in internal binary format 
for subsequent analysis, calculate scalar metrics of difference phase characteristics, 
and cluster several calibrations into one. Algorithms, formulas and expressions used in 
the calculation process are strictly defined and described in the documentation.

SYSTEM REQUIREMENTS

The GUI components of the software requires Miscrosoft Windows environment. It was developped 
using C++ (ISO C++17 Standard), Microsoft Foundation Class Library (MFC) and OpenGL library.
No other libraries required.

LICENSE

The Can2 software is distributed under the following BSD 2-clause
license (http://opensource.org/licenses/BSD-2-Clause) and additional two
exclusive clauses. Users are permitted to develop, produce or sell their own
non-commercial or commercial products utilizing, linking or including Can2 as
long as they comply with the license.

          Copyright (c) 2024, Topcon, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

- The software package includes some companion executive binaries or shared
  libraries necessary to execute APs on Windows. These licenses succeed to the
  original ones of these software. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
