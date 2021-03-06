# ifndef CPPAD_LOCAL_SWEEP_FOR_HES_HPP
# define CPPAD_LOCAL_SWEEP_FOR_HES_HPP
/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-18 Bradley M. Bell

CppAD is distributed under the terms of the
             Eclipse Public License Version 2.0.

This Source Code may also be made available under the following
Secondary License when the conditions for such availability set forth
in the Eclipse Public License, Version 2.0 are satisfied:
      GNU General Public License, Version 2.0 or later.
---------------------------------------------------------------------------- */

# include <cppad/local/play/atom_op_info.hpp>

// BEGIN_CPPAD_LOCAL_SWEEP_NAMESPACE
namespace CppAD { namespace local { namespace sweep {
/*!
\file sweep/for_hes.hpp
Compute Forward mode Hessian sparsity patterns.
*/

/*!
\def CPPAD_FOR_HES_TRACE
This value is either zero or one.
Zero is the normal operational value.
If it is one, a trace of every rev_hes_sweep computation is printed.
*/
# define CPPAD_FOR_HES_TRACE 0

/*!
Given the forward Jacobian sparsity pattern for all the variables,
and the reverse Jacobian sparsity pattern for the dependent variables,
ForHesSweep computes the Hessian sparsity pattern for all the independent
variables.

\tparam Base
this operation sequence was recorded using AD<Base>.

\tparam Vector_set
is the type used for vectors of sets. It can be either
sparse_pack or sparse_list.

\param n
is the number of independent variables on the tape.

\param numvar
is the total number of variables on the tape; i.e.,
 play->num_var_rec().
This is also the number of rows in the entire sparsity pattern
 for_hes_sparse.

\param play
The information stored in play
is a recording of the operations corresponding to a function
\f[
    F : {\bf R}^n \rightarrow {\bf R}^m
\f]
where \f$ n \f$ is the number of independent variables
and \f$ m \f$ is the number of dependent variables.

\param for_jac_sparse
For i = 0 , ... , numvar - 1,
(for all the variables on the tape),
the forward Jacobian sparsity pattern for the variable with index i
corresponds to the set with index i in for_jac_sparse.

\param rev_jac_sparse
\b Input:
For i = 0, ... , numvar - 1
the if the function we are computing the Hessian for has a non-zero
derivative w.r.t. variable with index i,
the set with index i has element zero.
Otherwise it has no elements.

\param for_hes_sparse
The forward Hessian sparsity pattern for the variable with index i
corresponds to the set with index i in for_hes_sparse.
The number of rows in this sparsity patter is n+1 and the row
with index zero is not used.
\n
\n
\b Input: For i = 1 , ... , n
the forward Hessian sparsity pattern for the variable with index i is empty.
\n
\n
\b Output: For j = 1 , ... , n,
the forward Hessian sparsity pattern for the independent dependent variable
with index (j-1) is given by the set with index j
in for_hes_sparse.

\param not_used_rec_base
Specifies RecBase for this call.
*/

template <class Addr, class Base, class Vector_set, class RecBase>
void for_hes(
    const local::player<Base>* play,
    size_t                     n,
    size_t                     numvar,
    const Vector_set&          for_jac_sparse,
    const Vector_set&          rev_jac_sparse,
    Vector_set&                for_hes_sparse,
    const RecBase&             not_used_rec_base
)
{
    // length of the parameter vector (used by CppAD assert macros)
    const size_t num_par = play->num_par_rec();

    size_t             i, j, k;

    // check numvar argument
    size_t limit = n+1;
    CPPAD_ASSERT_UNKNOWN( play->num_var_rec()    == numvar );
    CPPAD_ASSERT_UNKNOWN( for_jac_sparse.n_set() == numvar );
    CPPAD_ASSERT_UNKNOWN( for_hes_sparse.n_set() == limit );
    CPPAD_ASSERT_UNKNOWN( numvar > 0 );

    // upper limit exclusive for set elements
    CPPAD_ASSERT_UNKNOWN( for_jac_sparse.end() == limit );
    CPPAD_ASSERT_UNKNOWN( for_hes_sparse.end() == limit );

    // vecad_sparsity contains a sparsity pattern for each VecAD object.
    // vecad_ind maps a VecAD index (beginning of the VecAD object)
    // to the index for the corresponding set in vecad_sparsity.
    size_t num_vecad_ind   = play->num_vec_ind_rec();
    size_t num_vecad_vec   = play->num_vecad_vec_rec();
    Vector_set vecad_sparse;
    pod_vector<size_t> vecad_ind;
    pod_vector<bool>   vecad_jac;
    if( num_vecad_vec > 0 )
    {   size_t length;
        vecad_sparse.resize(num_vecad_vec, limit);
        vecad_ind.extend(num_vecad_ind);
        vecad_jac.extend(num_vecad_vec);
        j             = 0;
        for(i = 0; i < num_vecad_vec; i++)
        {   // length of this VecAD
            length   = play->GetVecInd(j);
            // set vecad_ind to proper index for this VecAD
            vecad_ind[j] = i;
            // make all other values for this vector invalid
            for(k = 1; k <= length; k++)
                vecad_ind[j+k] = num_vecad_vec;
            // start of next VecAD
            j       += length + 1;
            // initialize this vector's reverse jacobian value
            vecad_jac[i] = false;
        }
        CPPAD_ASSERT_UNKNOWN( j == play->num_vec_ind_rec() );
    }
    // ------------------------------------------------------------------------
    // atomic function
    atomic_base<RecBase>* atom_fun = CPPAD_NULL; // atomic function
    //
    // work space used by AFunOp.
    vector<Base>       atom_x;   // value of parameter arguments to function
    pod_vector<size_t> atom_ix;  // variable index (on tape) for each argument
    pod_vector<size_t> atom_iy;  // variable index (on tape) for each result
    //
    // information set by atomic forward (initialization to avoid warnings)
    size_t atom_old=0, atom_m=0, atom_n=0, atom_i=0, atom_j=0;
    // information set by atomic forward (necessary initialization)
    enum_atom_state atom_state = start_atom;
    // -------------------------------------------------------------------------
    //
    // pointer to the beginning of the parameter vector
    // (used by atomic functions)
    const Base* parameter = CPPAD_NULL;
    if( num_par > 0 )
        parameter = play->GetPar();

    // skip the BeginOp at the beginning of the recording
    play::const_sequential_iterator itr = play->begin();
    // op_info
    OpCode op;
    size_t i_var;
    const Addr*   arg;
    itr.op_info(op, arg, i_var);
    CPPAD_ASSERT_UNKNOWN( op == BeginOp );
# if CPPAD_FOR_HES_TRACE
    vector<size_t> atom_funrp; // parameter index for FunrpOp operators
    std::cout << std::endl;
    CppAD::vectorBool zf_value(limit);
    CppAD::vectorBool zh_value(limit * limit);
# endif
    bool flag; // temporary for use in switch cases below
    bool more_operators = true;
    while(more_operators)
    {
        // next op
        (++itr).op_info(op, arg, i_var);

        // does the Hessian in question have a non-zero derivative
        // with respect to this variable
        bool include = NumRes(op) > 0;
        if( include )
            include = rev_jac_sparse.is_element(i_var, 0);
        //
        // operators to include even if derivative is zero
        include |= op == EndOp;
        include |= op == CSkipOp;
        include |= op == CSumOp;
        include |= op == AFunOp;
        include |= op == FunapOp;
        include |= op == FunavOp;
        include |= op == FunrpOp;
        include |= op == FunrvOp;
        //
        if( include ) switch( op )
        {   // operators that should not occurr
            // case BeginOp
            // -------------------------------------------------

            // operators that do not affect hessian
            case AbsOp:
            case AddvvOp:
            case AddpvOp:
            case CExpOp:
            case DisOp:
            case DivvpOp:
            case InvOp:
            case LdpOp:
            case LdvOp:
            case MulpvOp:
            case ParOp:
            case PriOp:
            case SignOp:
            case StppOp:
            case StpvOp:
            case StvpOp:
            case StvvOp:
            case SubvvOp:
            case SubpvOp:
            case SubvpOp:
            case ZmulpvOp:
            case ZmulvpOp:
            break;
            // -------------------------------------------------

            // nonlinear unary operators
            case AcosOp:
            case AsinOp:
            case AtanOp:
            case CosOp:
            case CoshOp:
            case ExpOp:
            case LogOp:
            case SinOp:
            case SinhOp:
            case SqrtOp:
            case TanOp:
            case TanhOp:
# if CPPAD_USE_CPLUSPLUS_2011
            case AcoshOp:
            case AsinhOp:
            case AtanhOp:
            case Expm1Op:
            case Log1pOp:
# endif
            CPPAD_ASSERT_UNKNOWN( NumArg(op) == 1 )
            forward_sparse_hessian_nonlinear_unary_op(
                size_t(arg[0]), for_jac_sparse, for_hes_sparse
            );
            break;
            // -------------------------------------------------

            case CSkipOp:
            itr.correct_before_increment();
            break;
            // -------------------------------------------------

            case CSumOp:
            itr.correct_before_increment();
            break;
            // -------------------------------------------------

            case DivvvOp:
            CPPAD_ASSERT_NARG_NRES(op, 2, 1)
            forward_sparse_hessian_div_op(
                arg, for_jac_sparse, for_hes_sparse
            );
            break;
            // -------------------------------------------------

            case DivpvOp:
            CPPAD_ASSERT_NARG_NRES(op, 2, 1)
            forward_sparse_hessian_nonlinear_unary_op(
                size_t(arg[1]), for_jac_sparse, for_hes_sparse
            );
            break;
            // -------------------------------------------------

            case EndOp:
            CPPAD_ASSERT_NARG_NRES(op, 0, 0);
            more_operators = false;
            break;
            // -------------------------------------------------

            case ErfOp:
            // arg[1] is always the parameter 0
            // arg[2] is always the parameter 2 / sqrt(pi)
            CPPAD_ASSERT_NARG_NRES(op, 3, 5);
            forward_sparse_hessian_nonlinear_unary_op(
                size_t(arg[0]), for_jac_sparse, for_hes_sparse
            );
            break;
            // -------------------------------------------------

            // -------------------------------------------------
            // logical comparision operators
            case EqppOp:
            case EqpvOp:
            case EqvvOp:
            case LtppOp:
            case LtpvOp:
            case LtvpOp:
            case LtvvOp:
            case LeppOp:
            case LepvOp:
            case LevpOp:
            case LevvOp:
            case NepvOp:
            case NeppOp:
            case NevvOp:
            CPPAD_ASSERT_NARG_NRES(op, 2, 0);
            break;
            // -------------------------------------------------

            case MulvvOp:
            CPPAD_ASSERT_NARG_NRES(op, 2, 1)
            forward_sparse_hessian_mul_op(
                arg, for_jac_sparse, for_hes_sparse
            );
            break;
            // -------------------------------------------------

            case PowpvOp:
            CPPAD_ASSERT_NARG_NRES(op, 2, 3)
            forward_sparse_hessian_nonlinear_unary_op(
                size_t(arg[1]), for_jac_sparse, for_hes_sparse
            );
            break;
            // -------------------------------------------------

            case PowvpOp:
            CPPAD_ASSERT_NARG_NRES(op, 2, 3)
            forward_sparse_hessian_nonlinear_unary_op(
                size_t(arg[0]), for_jac_sparse, for_hes_sparse
            );
            break;
            // -------------------------------------------------

            case PowvvOp:
            CPPAD_ASSERT_NARG_NRES(op, 2, 3)
            forward_sparse_hessian_pow_op(
                arg, for_jac_sparse, for_hes_sparse
            );
            break;
            // -------------------------------------------------

            case AFunOp:
            // start or end an atomic function call
            CPPAD_ASSERT_UNKNOWN(
                atom_state == start_atom || atom_state == end_atom
            );
            flag = atom_state == start_atom;
            atom_fun = play::atom_op_info<RecBase>(
                op, arg, atom_old, atom_m, atom_n
            );
            if( flag )
            {   atom_state = arg_atom;
                atom_i     = 0;
                atom_j     = 0;
                //
                atom_x.resize( atom_n );
                atom_ix.resize( atom_n );
                atom_iy.resize( atom_m );
# if CPPAD_FOR_HES_TRACE
                atom_funrp.resize( atom_m );
# endif
            }
            else
            {   atom_state = start_atom;
                //
                atom_fun->set_old(atom_old);
                atom_fun->for_sparse_hes(
                    atom_x, atom_ix, atom_iy,
                    for_jac_sparse, rev_jac_sparse, for_hes_sparse
                );
            }
            break;

            case FunapOp:
            // parameter argument for a atomic function
            CPPAD_ASSERT_UNKNOWN( NumArg(op) == 1 );
            CPPAD_ASSERT_UNKNOWN( atom_state == arg_atom );
            CPPAD_ASSERT_UNKNOWN( atom_i == 0 );
            CPPAD_ASSERT_UNKNOWN( atom_j < atom_n );
            CPPAD_ASSERT_UNKNOWN( size_t( arg[0] ) < num_par );
            //
            atom_x[atom_j] = parameter[arg[0]];
            atom_ix[atom_j] = 0; // special variable used for parameters
            //
            ++atom_j;
            if( atom_j == atom_n )
                atom_state = ret_atom;
            break;

            case FunavOp:
            // variable argument for a atomic function
            CPPAD_ASSERT_UNKNOWN( NumArg(op) == 1 );
            CPPAD_ASSERT_UNKNOWN( atom_state == arg_atom );
            CPPAD_ASSERT_UNKNOWN( atom_i == 0 );
            CPPAD_ASSERT_UNKNOWN( atom_j < atom_n );
            //
            // arguemnt variables not avaialbe during sparisty calculations
            atom_x[atom_j] = CppAD::numeric_limits<Base>::quiet_NaN();
            atom_ix[atom_j] = size_t(arg[0]); // variable for this argument
            //
            ++atom_j;
            if( atom_j == atom_n )
                atom_state = ret_atom;
            break;

            case FunrpOp:
            // parameter result for a atomic function
            CPPAD_ASSERT_NARG_NRES(op, 1, 0);
            CPPAD_ASSERT_UNKNOWN( atom_state == ret_atom );
            CPPAD_ASSERT_UNKNOWN( atom_i < atom_m );
            CPPAD_ASSERT_UNKNOWN( atom_j == atom_n );
            CPPAD_ASSERT_UNKNOWN( size_t( arg[0] ) < num_par );
            //
            atom_iy[atom_i] = 0; // special variable used for parameters
# if CPPAD_FOR_HES_TRACE
            // remember argument for delayed tracing
            atom_funrp[atom_i] = arg[0];
# endif
            ++atom_i;
            if( atom_i == atom_m )
                atom_state = end_atom;
            break;

            case FunrvOp:
            // variable result for a atomic function
            CPPAD_ASSERT_NARG_NRES(op, 0, 1);
            CPPAD_ASSERT_UNKNOWN( atom_state == ret_atom );
            CPPAD_ASSERT_UNKNOWN( atom_i < atom_m );
            CPPAD_ASSERT_UNKNOWN( atom_j == atom_n );
            //
            atom_iy[atom_i] = i_var; // variable index for this result
            //
            ++atom_i;
            if( atom_i == atom_m )
                atom_state = end_atom;
            break;
            // -------------------------------------------------

            case ZmulvvOp:
            CPPAD_ASSERT_NARG_NRES(op, 2, 1)
            forward_sparse_hessian_mul_op(
                arg, for_jac_sparse, for_hes_sparse
            );
            break;

            // -------------------------------------------------

            default:
            CPPAD_ASSERT_UNKNOWN(0);
        }
# if CPPAD_FOR_HES_TRACE
        typedef typename Vector_set::const_iterator const_iterator;
        if( op == AFunOp && atom_state == start_atom )
        {   // print operators that have been delayed
            CPPAD_ASSERT_UNKNOWN( atom_m == atom_iy.size() );
            CPPAD_ASSERT_UNKNOWN( itr.op_index() > atom_m );
            CPPAD_ASSERT_NARG_NRES(FunrpOp, 1, 0);
            CPPAD_ASSERT_NARG_NRES(FunrvOp, 0, 1);
            addr_t arg_tmp[1];
            for(k = 0; k < atom_m; k++)
            {   size_t k_var = atom_iy[k];
                // value for this variable
                for(i = 0; i < limit; i++)
                {   zf_value[i] = false;
                    for(j = 0; j < limit; j++)
                        zh_value[i * limit + j] = false;
                }
                const_iterator itr_1(for_jac_sparse, i_var);
                j = *itr_1;
                while( j < limit )
                {   zf_value[j] = true;
                    j = *(++itr_1);
                }
                for(i = 0; i < limit; i++)
                {   const_iterator itr_2(for_hes_sparse, i);
                    j = *itr_2;
                    while( j < limit )
                    {   zh_value[i * limit + j] = true;
                        j = *(++itr_2);
                    }
                }
                OpCode op_tmp = FunrvOp;
                if( k_var == 0 )
                {   op_tmp     = FunrpOp;
                    arg_tmp[0] = atom_funrp[k];
                }
                // k_var is zero when there is no result
                printOp(
                    std::cout,
                    play,
                    itr.op_index() - atom_m + k,
                    k_var,
                    op_tmp,
                    arg_tmp
                );
                if( k_var > 0 ) printOpResult(
                    std::cout,
                    1,
                    &zf_value,
                    1,
                    &zh_value
                );
                std::cout << std::endl;
            }
        }
        for(i = 0; i < limit; i++)
        {   zf_value[i] = false;
            for(j = 0; j < limit; j++)
                zh_value[i * limit + j] = false;
        }
        const_iterator itr_1(for_jac_sparse, i_var);
        j = *itr_1;
        while( j < limit )
        {   zf_value[j] = true;
            j = *(++itr_1);
        }
        for(i = 0; i < limit; i++)
        {   const_iterator itr_2(for_hes_sparse, i);
            j = *itr_2;
            while( j < limit )
            {   zh_value[i * limit + j] = true;
                j = *(++itr_2);
            }
        }
        // must delay print for these cases till after atomic function call
        bool delay_print = op == FunrpOp;
        delay_print     |= op == FunrvOp;
        if( ! delay_print )
        {    printOp(
                std::cout,
                play,
                itr.op_index(),
                i_var,
                op,
                arg
            );
            if( NumRes(op) > 0 && (! delay_print) ) printOpResult(
                std::cout,
                1,
                &zf_value,
                1,
                &zh_value
            );
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
# else
    }
# endif

    return;
}
} } } // END_CPPAD_LOCAL_SWEEP_NAMESPACE

// preprocessor symbols that are local to this file
# undef CPPAD_FOR_HES_TRACE

# endif
