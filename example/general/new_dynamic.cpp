/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-18 Bradley M. Bell

CppAD is distributed under multiple licenses. This distribution is under
the terms of the
                    Eclipse Public License Version 1.0.

A copy of this license is included in the COPYING file of this distribution.
Please visit http://www.coin-or.org/CppAD/ for information on other licenses.
-------------------------------------------------------------------------- */

/*
$begin new_dynamic.cpp$$
$spell
	Cpp
$$

$section Dynamic Parameters: Example and Test$$

$code
$srcfile%example/general/new_dynamic.cpp%0%// BEGIN C++%// END C++%1%$$
$$

$end
*/
// BEGIN C++
# include <limits>
# include <cppad/cppad.hpp>

bool new_dynamic(void)
{	bool ok = true;
	using CppAD::AD;
	using CppAD::NearEqual;
	double eps = 10. * std::numeric_limits<double>::epsilon();

	// dynamic parameter vector
	size_t nd = 3;
	CPPAD_TESTVECTOR(AD<double>) adynamic(nd);
	adynamic[0] = 1.0;
	adynamic[1] = 1.0;
	adynamic[2] = 1.0;

	// domain space vector
	size_t nx = 2;
	CPPAD_TESTVECTOR(AD<double>) ax(nx);
	ax[0] = 0.;
	ax[1] = 1.;

	// declare independent variables, dynamic parammeters, starting recording
	size_t abort_op_index = 0;
	bool   record_compare = true;
	CppAD::Independent(ax, abort_op_index, record_compare, adynamic);

	// range space vector
	size_t ny = 1;
	CPPAD_TESTVECTOR(AD<double>) ay(ny);
	ay[0]  = adynamic[0] + ax[0];
	ay[0] *= adynamic[1] + ax[0];
	ay[0] *= adynamic[2] + ax[1];

	// create f: x -> y and stop tape recording
	CppAD::ADFun<double> f(ax, ay);

	// zero order forward mode
	CPPAD_TESTVECTOR(double) x(nx), y(ny);
	x[0] = 3.;
	x[1] = 4.;
	y    = f.Forward(0, x);
	double check;
	check  = Value( adynamic[0] ) + x[0];
	check *= Value( adynamic[1] ) + x[0];
	check *= Value( adynamic[2] ) + x[1];
	ok  &= NearEqual(y[0] , check, eps, eps);

	// change the dynamic parameter values
	CPPAD_TESTVECTOR(double) dynamic(nd);
	dynamic[0] = 2.0;
	dynamic[1] = 3.0;
	dynamic[2] = 4.0;
	f.new_dynamic(dynamic);
	//
	y    = f.Forward(0, x);
	check  = dynamic[0] + x[0];
	check *= dynamic[1] + x[0];
	check *= dynamic[2] + x[1];
	ok  &= NearEqual(y[0] , check, eps, eps);

	// use first order foward mode to compute partial of f w.r.t x[0]
	CPPAD_TESTVECTOR(double) dx(nx), dy(ny);
	dx[0] = 1.0;
	dx[1] = 0.0;
	dy    = f.Forward(1, dx);
	check = (dynamic[2] +  x[1]) * (dynamic[0] + x[0] + dynamic[1] + x[0]);
	ok  &= NearEqual(dy[0] , check, eps, eps);
	//
	return ok;
}
// END C++