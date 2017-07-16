var list_across0 = [
'_contents.htm',
'_reference.htm',
'_index.htm',
'_search.htm',
'_external.htm'
];
var list_up0 = [
'cppad.htm',
'example.htm',
'general.htm',
'cppad_eigen.hpp.htm',
'eigen_array.cpp.htm'
];
var list_down3 = [
'get_started.cpp.htm',
'general.htm',
'exampleutility.htm',
'listallexamples.htm',
'testvector.htm'
];
var list_down2 = [
'ad_fun.cpp.htm',
'ad_in_c.cpp.htm',
'conj_grad.cpp.htm',
'cppad_eigen.hpp.htm',
'hes_minor_det.cpp.htm',
'hes_lu_det.cpp.htm',
'interface2c.cpp.htm',
'jac_minor_det.cpp.htm',
'jac_lu_det.cpp.htm',
'mul_level.htm',
'ode_stiff.cpp.htm',
'mul_level_ode.cpp.htm',
'mul_level_adolc_ode.cpp.htm',
'ode_taylor.cpp.htm',
'stack_machine.cpp.htm'
];
var list_down1 = [
'eigen_array.cpp.htm',
'eigen_det.cpp.htm'
];
function choose_across0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_across0[index-1];
}
function choose_up0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_up0[index-1];
}
function choose_down3(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_down3[index-1];
}
function choose_down2(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_down2[index-1];
}
function choose_down1(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_down1[index-1];
}
function choose_down0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_down0[index-1];
}
function choose_current0(item)
{	var index          = item.selectedIndex;
	item.selectedIndex = 0;
	if(index > 0)
		document.location = list_current0[index-1];
}
