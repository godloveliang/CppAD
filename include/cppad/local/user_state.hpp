# ifndef CPPAD_LOCAL_USER_STATE_HPP
# define CPPAD_LOCAL_USER_STATE_HPP
/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-16 Bradley M. Bell

CppAD is distributed under the terms of the
             Eclipse Public License Version 2.0.

This Source Code may also be made available under the following
Secondary License when the conditions for such availability set forth
in the Eclipse Public License, Version 2.0 are satisfied:
           GNU General Public License, Version 3.0.
---------------------------------------------------------------------------- */

namespace CppAD { namespace local { // BEGIN_CPPAD_LOCAL_NAMESPACE

enum enum_user_state {
	/// next AFunOp marks beginning of a atomic function call
	start_user,

	/// next FunapOp (FunavOp) is a parameter (variable) argument
	arg_user,

	/// next FunrpOp (FunrvOp) is a parameter (variable) result
	ret_user,

	/// next AFunOp marks end of a atomic function call
	end_user
};

} } // END_CPPAD_LOCAL_NAMESPACE
# endif