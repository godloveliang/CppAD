# ifndef CPPAD_CORE_CHECKPOINT_CTOR_HPP
# define CPPAD_CORE_CHECKPOINT_CTOR_HPP

/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-18 Bradley M. Bell

CppAD is distributed under multiple licenses. This distribution is under
the terms of the
                    Eclipse Public License Version 1.0.

A copy of this license is included in the COPYING file of this distribution.
Please visit http://www.coin-or.org/CppAD/ for information on other licenses.
-------------------------------------------------------------------------- */

namespace CppAD { // BEGIN_CPPAD_NAMESPACE

template <class Base>
template <class Algo, class ADVector>
checkpoint<Base>::checkpoint(
	const char*                    name            ,
	Algo&                          algo            ,
	const ADVector&                ax              ,
	ADVector&                      ay              ,
	option_enum                    sparsity        ,
	bool                           optimize
) : atomic_base<Base>(name, sparsity)
{
# ifndef NDEBUG
	if( thread_alloc::in_parallel() )
	{	std::string msg = name;
		msg += ": checkpoint constructor called in parallel mode.";
		CPPAD_ASSERT_KNOWN(false, msg.c_str() );
	}
# endif
	for(size_t thread = 0; thread < CPPAD_MAX_NUM_THREADS; ++thread)
		member_[thread] = CPPAD_NULL;
	//
	CheckSimpleVector< CppAD::AD<Base> , ADVector>();
	//
	// make a copy of ax because Independent modifies AD information
	ADVector x_tmp(ax);
	// delcare x_tmp as the independent variables
	Independent(x_tmp);
	// record mapping from x_tmp to ay
	algo(x_tmp, ay);
	// create function f_ : x -> y
	const_member_.f_.Dependent(ay);
	if( optimize )
	{	// suppress checking for nan in f_ results
		// (see optimize documentation for atomic functions)
		const_member_.f_.check_for_nan(false);
		//
		// now optimize
		const_member_.f_.optimize();
	}
	// now disable checking of comparison operations
	// 2DO: add a debugging mode that checks for changes and aborts
	const_member_.f_.compare_change_count(0);
}

} // END_CPPAD_NAMESPACE
# endif
