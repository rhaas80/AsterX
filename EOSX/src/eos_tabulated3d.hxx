#ifndef EOS_TABULATED3D_HXX
#define EOS_TABULATED3D_HXX

#include <cmath>
#include <hdf5.h>
#include <mpi.h>
#include "eos.hxx"
#include <string>

using namespace std;

namespace EOSX {

class eos_tabulated3d : public eos {
public:
  CCTK_REAL gamma;  // FIXME: get rid of this
  range rgeps;

  // Constructor
  CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline  eos_tabulated3d(
    const range &rgeps_, const range &rgrho_, const range &rgye_)
  {
    set_range_rho(rgrho_);
    set_range_ye(rgye_);
    // TODO: first compute temp as a function of rho, ye, and eps, and then initialize its range
    // For now, as dummy, we pass range of eps as range of temp
    set_range_temp(rgeps_);
  }


  // Destructor
  //CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline ~eos_tabulated3d();


  // Routine reading the EOS table and filling the corresponding object
  CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline void read_eos_table(const string &filename) {
    auto fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl_id >= 0);
    hid_t file_id = 0;
    int rank_id;
    CHECK_ERROR(MPI_Comm_rank(MPI_COMM_WORLD, &rank_id));

    #ifdef H5_HAVE_PARALLEL
    CHECK_ERROR(H5Pset_fapl_mpio(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL));
    CHECK_ERROR(H5Pset_all_coll_metadata_ops(fapl_id, true));
    file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, fapl_id);
    #else
    fapl_id = H5P_DEFAULT;
    if (rank_id == 0) {
      file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, fapl_id);
    }
    // TODO: bcast table info to all ranks
    #endif

    assert(file_id >= 0);

    // TODO: actually read the EOS table and fill the EOS object

    CHECK_ERROR(H5Pclose(fapl_id));

    #ifdef H5_HAVE_PARALLEL
    CHECK_ERROR(H5Fclose(file_id));
    #else
    if (rank_id == 0) {
      CHECK_ERROR(H5Fclose(file_id));
    }
    #endif

    return;
  }


  CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL press_from_valid_rho_eps_ye(const CCTK_REAL rho, const CCTK_REAL eps, const CCTK_REAL ye) const
{
  CCTK_REAL press = 0.0; //tab3d_press(rho, eps, ye, &ierr);
  return press;
}

CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL csnd_from_valid_rho_eps_ye(const CCTK_REAL rho, const CCTK_REAL eps, const CCTK_REAL ye) const
{
  CCTK_REAL csnd2 = 0.0; //tab3d_csnd2(rho, eps, ye, &ierr);
  assert(csnd2 >= 0); //Soundspeed^2 should never ever be negative
  return sqrt(csnd2);
}

CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL temp_from_valid_rho_eps_ye(const CCTK_REAL rho, const CCTK_REAL eps, const CCTK_REAL ye) const
{
  CCTK_REAL temp = 0.0; //tab3d_temp(rho, eps, ye, &ierr);
  return temp;
}

CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline void press_derivs_from_valid_rho_eps_ye(CCTK_REAL& press, CCTK_REAL& dpdrho, CCTK_REAL& dpdeps,
           const CCTK_REAL rho, const CCTK_REAL eps, const CCTK_REAL ye) const
{
  printf("press_derivs_from_valid_rho_eps_ye is not supported anymore!");
  exit(EXIT_FAILURE);
}

CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL press_from_valid_rho_temp_ye(const CCTK_REAL rho, const CCTK_REAL temp, const CCTK_REAL ye) const
{
    return 0.0; //tab3d_press_from_temp(rho, temp, ye);
}

CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL csnd_from_valid_rho_temp_ye(const CCTK_REAL rho, const CCTK_REAL temp, const CCTK_REAL ye) const
{
  CCTK_REAL csnd2 = 0.0; //tab3d_csnd2_from_temp(rho, temp, ye);
  assert(csnd2 >= 0); //Soundspeed^2 should never ever be negative
  return sqrt(csnd2);
}

CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL entropy_from_valid_rho_temp_ye(const CCTK_REAL rho, const CCTK_REAL temp, const CCTK_REAL ye) const
{
    return 0.0; //tab3d_entropy_from_temp(rho, temp, ye);
}

  CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline range
 range_eps_from_valid_rho_ye(const CCTK_REAL rho,
                                          const CCTK_REAL ye) const {
  return rgeps;
}

CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
eps_from_valid_rho_press_ye(const CCTK_REAL rho,
                                          const CCTK_REAL press,
                                          const CCTK_REAL ye) const {
  return 0.0; //press / (rho * gm1);
}

};
} // namespace EOSX

#endif
