#include <initial_setup.hxx>

namespace AsterX {

using namespace EOSX;

enum class eos_id {Polytropic, PWPolytropic}
enum class eos_evol { IdealGas, Hybrid, Tabulated };

// initial data EOS
eos_polytropic *eos_poly;

// evolution EOS
eos_idealgas *eos_ig;
eos_tabulated3d *eos_tab3d;

extern "C" void AsterX_Initialize_EOSID(CCTK_ARGUMENTS) {

eos_poly = new eos_polytropic{};

}

extern "C" void AsterX_Initialize_EOS(CCTK_ARGUMENTS) {

eos_ig =
eos_hybrid = 
eos_tab3d = 
}

} // namespace AsterX

#endif
