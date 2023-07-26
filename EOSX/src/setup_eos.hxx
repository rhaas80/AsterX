#ifndef EOSX_SETUP_EOS_HXX
#define EOSX_SETUP_EOS_HXX

#include <cctk.h>
#include <cctk_Arguments.h>
#include <cctk_Parameters.h>
#include <AMReX.H>

#include "eos_1p.hxx"
#include "eos_polytropic.hxx"

#include "eos.hxx"
#include "eos_idealgas.hxx"
#include "eos_tabulated3d.hxx"

namespace EOSX {

// initial data EOS
extern amrex::AsyncArray<eos_polytrope> eos_poly_aa;
auto &eos_poly = eos_poly_aa.data()[0];

// evolution EOS
extern amrex::AsyncArray<eos_idealgas> eos_ig_aa;
auto &eos_ig = eos_ig_aa.data()[0];

extern amrex::AsyncArray<eos_tabulated3d> eos_tab3d_aa;
auto &eos_tab3d = eos_tab3d_aa.data()[0];

} // namespace EOSX

#endif
