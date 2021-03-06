# ifndef CPPAD_LOCAL_PLAY_ATOM_OP_INFO_HPP
# define CPPAD_LOCAL_PLAY_ATOM_OP_INFO_HPP
/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-18 Bradley M. Bell

CppAD is distributed under the terms of the
             Eclipse Public License Version 2.0.

This Source Code may also be made available under the following
Secondary License when the conditions for such availability set forth
in the Eclipse Public License, Version 2.0 are satisfied:
      GNU General Public License, Version 2.0 or later.
---------------------------------------------------------------------------- */

// BEGIN_CPPAD_LOCAL_PLAY_NAMESPACE
namespace CppAD { namespace local { namespace play {

/*!
\file atom_op_info.hpp
*/

/*!
\brief
Unpack extra information corresponding to a AFunOp

\param op [in]
must be a AFunOp

\param op_arg [in]
is the arguments for this operator

\param atom_old [out]
is the extra information passed to the old style atomic functions.

\param atom_m   [out]
is the number of results for this user atmoic function.

\param atom_n   [out]
is the number of arguments for this user atmoic function.

\return
Is a pointer to this atomic function.
*/
template <class Base>
atomic_base<Base>* atom_op_info(
    const OpCode     op         ,
    const addr_t*    op_arg     ,
    size_t&          atom_old   ,
    size_t&          atom_m     ,
    size_t&          atom_n     )
{   atomic_base<Base>* atom_fun;
    //
    CPPAD_ASSERT_UNKNOWN( op == AFunOp );
    CPPAD_ASSERT_NARG_NRES(op, 4, 0);
    //
    atom_old = size_t(op_arg[1]);
    atom_n   = size_t(op_arg[2]);
    atom_m   = size_t(op_arg[3]);
    CPPAD_ASSERT_UNKNOWN( atom_n > 0 );
    //
    size_t user_index = size_t( op_arg[0] );
    atom_fun = atomic_base<Base>::class_object(user_index);
# ifndef NDEBUG
    if( atom_fun == CPPAD_NULL )
    {   // atom_fun is null so cannot use atom_fun->afun_name()
        std::string msg = atomic_base<Base>::class_name(user_index)
            + ": atomic_base function has been deleted";
        CPPAD_ASSERT_KNOWN(false, msg.c_str() );
    }
# endif
    // the atomic_base object corresponding to this atomic function
    atom_fun = atomic_base<Base>::class_object(user_index);
    CPPAD_ASSERT_UNKNOWN( atom_fun != CPPAD_NULL );
    return atom_fun;
}

} } } // END_CPPAD_LOCAL_PLAY_NAMESPACE

# endif
