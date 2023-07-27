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
#include "eos_hybrid.hxx"
#include "eos_tabulated3d.hxx"


namespace EOSX {

// initial data EOS
extern AMREX_GPU_MANAGED eos_polytrope *eos_poly;

// evolution EOS
extern AMREX_GPU_MANAGED eos_idealgas    *eos_ig;
extern AMREX_GPU_MANAGED eos_hybrid      *eos_hyb;
extern AMREX_GPU_MANAGED eos_tabulated3d *eos_tab3d;

} // namespace EOSX

#endif
