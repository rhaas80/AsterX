#ifndef ASTERX_INITIAL_SETUP_HXX
#define ASTERX_INITIAL_SETUP_HXX

#include <cctk.h>
#include <cctk_Arguments.h>
#include <cctk_Parameters.h>

#include <eos_1p.hxx>
#include <eos_polytropic.hxx>

#include <eos.hxx>
#include <eos_idealgas.hxx>
#include <eos_tabulated3d.hxx>

namespace AsterX {

// initial data EOS
extern EOSX::eos_polytrope *eos_poly;

// evolution EOS
extern EOSX::eos_idealgas *eos_ig;
extern EOSX::eos_tabulated3d *eos_tab3d;

} // namespace AsterX

#endif
