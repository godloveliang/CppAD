/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-17 Bradley M. Bell

CppAD is distributed under the terms of the
             Eclipse Public License Version 2.0.

This Source Code may also be made available under the following
Secondary License when the conditions for such availability set forth
in the Eclipse Public License, Version 2.0 are satisfied:
      GNU General Public License, Version 2.0 or later.
---------------------------------------------------------------------------- */

/*
$begin acos.cpp$$
$spell
    cos
    acos
$$

$section The AD acos Function: Example and Test$$

$srcfile%example/general/acos.cpp%0%// BEGIN C++%// END C++%1%$$

$end
*/
// BEGIN C++

# include <cppad/cppad.hpp>

bool acos(void)
{   bool ok = true;

    using CppAD::AD;
    using CppAD::NearEqual;

    // 10 times machine epsilon
    double eps = 10. * std::numeric_limits<double>::epsilon();

    // domain space vector
    size_t n  = 1;
    double x0 = 0.5;
    CPPAD_TESTVECTOR(AD<double>) x(n);
    x[0]      = x0;

    // declare independent variables and start tape recording
    CppAD::Independent(x);

    // a temporary value
    AD<double> cos_of_x0 = CppAD::cos(x[0]);

    // range space vector
    size_t m = 1;
    CPPAD_TESTVECTOR(AD<double>) y(m);
    y[0] = CppAD::acos(cos_of_x0);

    // create f: x -> y and stop tape recording
    CppAD::ADFun<double> f(x, y);

    // check value
    ok &= NearEqual(y[0] , x0,  eps, eps);

    // forward computation of first partial w.r.t. x[0]
    CPPAD_TESTVECTOR(double) dx(n);
    CPPAD_TESTVECTOR(double) dy(m);
    dx[0] = 1.;
    dy    = f.Forward(1, dx);
    ok   &= NearEqual(dy[0], 1., eps, eps);

    // reverse computation of derivative of y[0]
    CPPAD_TESTVECTOR(double)  w(m);
    CPPAD_TESTVECTOR(double) dw(n);
    w[0]  = 1.;
    dw    = f.Reverse(1, w);
    ok   &= NearEqual(dw[0], 1., eps, eps);

    // use a VecAD<Base>::reference object with acos
    CppAD::VecAD<double> v(1);
    AD<double> zero(0);
    v[zero] = cos_of_x0;
    AD<double> result = CppAD::acos(v[zero]);
    ok     &= NearEqual(result, x0, eps, eps);

    return ok;
}

// END C++
