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

  eos_id eos_id_type;

  if (CCTK_EQUALS(id_eos_name, "Polytropic")) {
    eos_id_type = eos_id::Polytropic;
  } else if (CCTK_EQUALS(id_eos_name, "PWPolytropic")) {
    eos_id_type = eos_id::PWPolytropic;
  } else {
    CCTK_ERROR("Unknown value for parameter \"initial_data_eos\"");
  }

  switch (eos_id_type) {
    case eos_id::Polytropic: {
      eos_poly = new eos_polytropic{};
    }
    case eos_id::PWPolytropic: {
    }
    default:
      assert(0);
  }
}

extern "C" void AsterX_Initialize_EOS(CCTK_ARGUMENTS) {

  eos_evol eos_evol_type;
  if (CCTK_EQUALS(evol_eos_name, "IdealGas")) {
    eos_evol_type = eos_evol::IdealGas;
  } else if (CCTK_EQUALS(evol_eos_name, "Hybrid")) {
    eos_evol_type = eos_evol::Hybrid;
  } else if (CCTK_EQUALS(evol_eos_name, "Tabulated3d")) {
    eos_evol_type = eos_evol::Tabulated;
  } else {
    CCTK_ERROR("Unknown value for parameter \"evolution_eos\"");
  }

  switch (eos_evol_type) {
  case eos_evol::IdealGas: {
    eos_ig=
    break;
  }
  case eos_evol::Hybrid: {
    CCTK_ERROR("Hybrid EOS is not yet supported");
    break;
  }
  case eos_evol::Tabulated: {
    eos_tab3d =
    break;
  }
  default:
    assert(0);
  }

}

} // namespace AsterX

#endif
