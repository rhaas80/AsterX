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
  CCTK_REAL gamma;
  range rgeps;

  // constructor
  CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline eos_tabulated3d(
      const string &filename, const bool &read_EOSTable_parallel) {
    auto fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl_id >= 0);
    hid_t file_id = 0;
    int mpi_rank_id;
    CHECK_ERROR(MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank_id));

    if(read_EOSTable_parallel){
      // Open the input file collectively
      CHECK_ERROR(H5Pset_fapl_mpio(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL));
      CHECK_ERROR(H5Pset_all_coll_metadata_ops(fapl_id, true));
      file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, fapl_id);
    } 
    else {
      fapl_id = H5P_DEFAULT;

      if (!mpi_rank_id) {
        file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, fapl_id);
      }
    }
  
    assert(file_id >= 0);
    CHECK_ERROR(H5Pclose(fapl_id));
    if(read_EOSTable_parallel || !mpi_rank_id) {
      CHECK_ERROR(H5Fclose(file_id));
    }
  }
};

  CCTK_HOST CCTK_DEVICE CCTK_ATTRIBUTE_ALWAYS_INLINE inline range
 range_eps_from_valid_rho_ye(const CCTK_REAL rho,
                                          const CCTK_REAL ye) const {
  return rgeps;
}

} // namespace EOSX

#endif
