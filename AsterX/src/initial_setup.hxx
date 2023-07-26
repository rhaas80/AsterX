#ifndef ASTERX_INITIAL_SETUP_HXX
#define ASTERX_INITIAL_SETUP_HXX

#include <cctk.h>
#include <cctk_Arguments.h>
#include <cctk_Parameters.h>
#include <AMReX.H>

#include <eos_1p.hxx>
#include <eos_polytropic.hxx>

#include <eos.hxx>
#include <eos_idealgas.hxx>
#include <eos_tabulated3d.hxx>

namespace AsterX {

using namespace Arith;
using namespace AMReX;

// initial data EOS
extern Vector<EOSX::eos_polytrope, 1> eos_poly;

// evolution EOS
extern Vector<EOSX::eos_idealgas, 1> eos_ig;
extern Vector<EOSX::eos_tabulated3d, 1> eos_tab3d;

} // namespace AsterX

#endif
