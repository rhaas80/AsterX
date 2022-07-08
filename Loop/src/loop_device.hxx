#ifndef LOOP_DEVICE_HXX
#define LOOP_DEVICE_HXX

#include "loop.hxx"

#include <cctk.h>

#include <AMReX.H>
#include <AMReX_Box.H>
#include <AMReX_IndexType.H>
#include <AMReX_IntVect.H>
#ifdef AMREX_USE_GPU
#include <AMReX_Gpu.H>
#endif

#include <cassert>

#ifndef HAVE_CAPABILITY_Loop
#error                                                                         \
    "Using #include <loop_device.hxx> requires the capability 'Loop' in the calling thorn. Add this to the thorn's 'configuration.ccl' file."
#endif

namespace Loop {

// The struct GridDescBaseDevice is very similar to GridDescBase, except that it
// requires AMReX include files, and thus "REQUIRES CarpetX" in the using
// thorns' configuration.ccl file.
struct GridDescBaseDevice : GridDescBase {

protected:
  GridDescBaseDevice() : GridDescBase() {}

public:
  GridDescBaseDevice(const cGH *cctkGH) : GridDescBase(cctkGH) {}

  // Loop over a given box
  template <int CI, int CJ, int CK, int VS = 1, typename F>
  // inline CCTK_ATTRIBUTE_ALWAYS_INLINE
  CCTK_ATTRIBUTE_NOINLINE void
  loop_box_device(const F &f, const std::array<int, dim> &restrict imin,
                  const std::array<int, dim> &restrict imax,
                  const std::array<int, dim> &restrict inormal) const {
#ifndef AMREX_USE_GPU
    return this->template loop_box<CI, CJ, CK, VS>(f, imin, imax, inormal);
#else

    static_assert(CI == 0 || CI == 1, "");
    static_assert(CJ == 0 || CJ == 1, "");
    static_assert(CK == 0 || CK == 1, "");
    static_assert(VS > 0, "");

    for (int d = 0; d < dim; ++d)
      if (imin[d] >= imax[d])
        return;

    // std::array<bool, dim> bforward;
    // for (int d = 0; d < dim; ++d)
    //   bforward[d] = inormal[d] >= 0;
    // bool all_forward = true;
    // for (int d = 0; d < dim; ++d)
    //   all_forward &= bforward[d];

    // Run on GPU
    static_assert(VS == 1, "Only vector size of 1 is supported on GPUs");

    // For some reason, the arguments imin, imax, and inormal cannot
    // be captured correctly in CUDA, but copies of them can
    const auto imin1 = imin;
    const auto imax1 = imax;
    const auto inormal1 = inormal;

    // Convert to AMReX box
    const amrex::Box box(
        amrex::IntVect(imin[0], imin[1], imin[2]),
        amrex::IntVect(imax[0] - 1, imax[1] - 1, imax[2] - 1),
        amrex::IntVect(CI ? amrex::IndexType::CELL : amrex::IndexType::NODE,
                       CJ ? amrex::IndexType::CELL : amrex::IndexType::NODE,
                       CK ? amrex::IndexType::CELL : amrex::IndexType::NODE));

    bool has_normal = false;
    for (int d = 0; d < dim; ++d)
      has_normal |= inormal[d] != 0;

    if (!has_normal) {
      // Loop over an interior box

      amrex::launch(box, [=, *this] CCTK_DEVICE(const amrex::Box &box)
                             CCTK_ATTRIBUTE_ALWAYS_INLINE {
#ifdef CCTK_DEBUG
                               assert(box.bigEnd()[0] == box.smallEnd()[0] &&
                                      box.bigEnd()[1] == box.smallEnd()[1] &&
                                      box.bigEnd()[2] == box.smallEnd()[2]);
#endif
                               const int i = box.smallEnd()[0];
                               const int j = box.smallEnd()[1];
                               const int k = box.smallEnd()[2];
                               constexpr std::array<int, dim> izero = {0, 0, 0};
                               f(point_desc<CI, CJ, CK>(izero, izero, imin1[0],
                                                        imax1[0], i, j, k));
                             });

    } else {
      // Loop over a box at a boundary

      amrex::launch(box, [=, *this] CCTK_DEVICE(const amrex::Box &box)
                             CCTK_ATTRIBUTE_ALWAYS_INLINE {
#ifdef CCTK_DEBUG
                               assert(box.bigEnd()[0] == box.smallEnd()[0] &&
                                      box.bigEnd()[1] == box.smallEnd()[1] &&
                                      box.bigEnd()[2] == box.smallEnd()[2]);
#endif
                               const int i = box.smallEnd()[0];
                               const int j = box.smallEnd()[1];
                               const int k = box.smallEnd()[2];

                               const std::array<int, dim> I = {i, j, k};
                               std::array<int, dim> I0;
                               for (int d = 0; d < dim; ++d) {
                                 if (inormal1[d] == 0)
                                   // interior
                                   I0[d] = I[d];
                                 else if (inormal1[d] < 0)
                                   // left boundary
                                   I0[d] = imax1[d];
                                 else
                                   // right boundary
                                   I0[d] = imin1[d] - 1;
                               }
                               f(point_desc<CI, CJ, CK>(inormal1, I0, imin1[0],
                                                        imax1[0], i, j, k));
                             });
    }

#endif

#ifdef AMREX_USE_GPU
    static bool have_gpu_sync_after_every_kernel = false;
    static bool gpu_sync_after_every_kernel;
    if (!have_gpu_sync_after_every_kernel) {
      int type;
      const void *const gpu_sync_after_every_kernel_ptr =
          CCTK_ParameterGet("gpu_sync_after_every_kernel", "CarpetX", &type);
      assert(gpu_sync_after_every_kernel_ptr);
      assert(type == PARAMETER_BOOLEAN);
      gpu_sync_after_every_kernel =
          *static_cast<const CCTK_INT *>(gpu_sync_after_every_kernel_ptr);
    }
    if (gpu_sync_after_every_kernel) {
      amrex::Gpu::synchronize();
      AMREX_GPU_ERROR_CHECK();
    }
#endif
  }

  // Loop over all points
  template <int CI, int CJ, int CK, int VS = 1, typename F>
  inline CCTK_ATTRIBUTE_ALWAYS_INLINE void
  loop_all_device(const std::array<int, dim> &group_nghostzones,
                  const F &f) const {
    const std::array<int, dim> offset{CI, CJ, CK};
    std::array<int, dim> imin, imax;
    for (int d = 0; d < dim; ++d) {
      int ghost_offset = nghostzones[d] - group_nghostzones[d];
      imin[d] = std::max(tmin[d], ghost_offset);
      imax[d] = std::min(tmax[d], lsh[d] - offset[d] - ghost_offset);
    }
    const std::array<int, dim> inormal{0, 0, 0};

    loop_box_device<CI, CJ, CK, VS>(f, imin, imax, inormal);
  }

  // Loop over all interior points
  template <int CI, int CJ, int CK, int VS = 1, typename F>
  inline CCTK_ATTRIBUTE_ALWAYS_INLINE void
  loop_int_device(const std::array<int, dim> &group_nghostzones,
                  const F &f) const {
    const std::array<int, dim> offset{CI, CJ, CK};
    std::array<int, dim> imin, imax;
    for (int d = 0; d < dim; ++d) {
      imin[d] = std::max(tmin[d], nghostzones[d]);
      imax[d] = std::min(tmax[d], lsh[d] - offset[d] - nghostzones[d]);
    }
    const std::array<int, dim> inormal{0, 0, 0};

    loop_box_device<CI, CJ, CK, VS>(f, imin, imax, inormal);
  }

  // Loop over a part of the domain. Loop over the interior first,
  // then faces, then edges, then corners.
  template <int CI, int CJ, int CK, int VS = 1, typename F>
  inline CCTK_ATTRIBUTE_ALWAYS_INLINE void loop_there_device(
      const std::array<int, dim> &group_nghostzones,
      const std::array<std::array<std::array<bool, dim>, dim>, dim> &there,
      const F &f) const {
    const std::array<int, dim> offset{CI, CJ, CK};

    for (int rank = dim; rank >= 0; --rank) {

      for (int nk = -1; nk <= +1; ++nk) {
        for (int nj = -1; nj <= +1; ++nj) {
          for (int ni = -1; ni <= +1; ++ni) {
            if ((ni == 0) + (nj == 0) + (nk == 0) == rank) {

              if (there[ni + 1][nj + 1][nk + 1]) {

                const std::array<int, dim> inormal{ni, nj, nk};

                std::array<int, dim> imin, imax;
                for (int d = 0; d < dim; ++d) {
                  const int ghost_offset =
                      nghostzones[d] - group_nghostzones[d];
                  const int begin_bnd = ghost_offset;
                  const int begin_int = nghostzones[d];
                  const int end_int = lsh[d] - offset[d] - nghostzones[d];
                  const int end_bnd = lsh[d] - offset[d] - ghost_offset;
                  switch (inormal[d]) {
                  case -1: // lower boundary
                    imin[d] = begin_bnd;
                    imax[d] = begin_int;
                    break;
                  case 0: // interior
                    imin[d] = begin_int;
                    imax[d] = end_int;
                    break;
                  case +1: // upper boundary
                    imin[d] = end_int;
                    imax[d] = end_bnd;
                    break;
                  default:
                    assert(0);
                  }

                  imin[d] = std::max(tmin[d], imin[d]);
                  imax[d] = std::min(tmax[d], imax[d]);
                }

                loop_box_device<CI, CJ, CK, VS>(f, imin, imax, inormal);
              }
            } // if rank
          }
        }
      }

    } // for rank
  }

  // Loop over all outer boundary points. This excludes ghost faces, but
  // includes ghost edges/corners on non-ghost faces. Loop over faces first,
  // then edges, then corners.
  template <int CI, int CJ, int CK, int VS = 1, typename F>
  inline CCTK_ATTRIBUTE_ALWAYS_INLINE void
  loop_bnd_device(const std::array<int, dim> &group_nghostzones,
                  const F &f) const {
    const std::array<int, dim> offset{CI, CJ, CK};

    for (int rank = dim - 1; rank >= 0; --rank) {

      for (int nk = -1; nk <= +1; ++nk) {
        for (int nj = -1; nj <= +1; ++nj) {
          for (int ni = -1; ni <= +1; ++ni) {
            if ((ni == 0) + (nj == 0) + (nk == 0) == rank) {

              if ((ni != 0 && bbox[0 + (ni == -1 ? 0 : 1)]) ||
                  (nj != 0 && bbox[2 + (nj == -1 ? 0 : 1)]) ||
                  (nk != 0 && bbox[4 + (nk == -1 ? 0 : 1)])) {

                const std::array<int, dim> inormal{ni, nj, nk};

                std::array<int, dim> imin, imax;
                for (int d = 0; d < dim; ++d) {
                  const int ghost_offset =
                      nghostzones[d] - group_nghostzones[d];
                  const int begin_bnd = ghost_offset;
                  const int begin_int = nghostzones[d];
                  const int end_int = lsh[d] - offset[d] - nghostzones[d];
                  const int end_bnd = lsh[d] - offset[d] - ghost_offset;
                  switch (inormal[d]) {
                  case -1: // lower boundary
                    imin[d] = begin_bnd;
                    imax[d] = begin_int;
                    break;
                  case 0: // interior
                    imin[d] = begin_int;
                    imax[d] = end_int;
                    break;
                  case +1: // upper boundary
                    imin[d] = end_int;
                    imax[d] = end_bnd;
                    break;
                  default:
                    assert(0);
                  }

                  imin[d] = std::max(tmin[d], imin[d]);
                  imax[d] = std::min(tmax[d], imax[d]);
                }

                loop_box_device<CI, CJ, CK, VS>(f, imin, imax, inormal);
              }
            } // if rank
          }
        }
      }

    } // for rank
  }

  // Loop over all outer ghost points. This excludes ghost edges/corners on
  // non-ghost faces. Loop over faces first, then edges, then corners.
  template <int CI, int CJ, int CK, int VS = 1, typename F>
  inline CCTK_ATTRIBUTE_ALWAYS_INLINE void
  loop_ghosts_device(const std::array<int, dim> &group_nghostzones,
                     const F &f) const {
    const std::array<int, dim> offset{CI, CJ, CK};

    for (int rank = dim - 1; rank >= 0; --rank) {

      for (int nk = -1; nk <= +1; ++nk) {
        for (int nj = -1; nj <= +1; ++nj) {
          for (int ni = -1; ni <= +1; ++ni) {
            if ((ni == 0) + (nj == 0) + (nk == 0) == rank) {

              if ((ni == 0 || !bbox[0 + (ni == -1 ? 0 : 1)]) &&
                  (nj == 0 || !bbox[2 + (nj == -1 ? 0 : 1)]) &&
                  (nk == 0 || !bbox[4 + (nk == -1 ? 0 : 1)])) {

                const std::array<int, dim> inormal{ni, nj, nk};

                std::array<int, dim> imin, imax;
                for (int d = 0; d < dim; ++d) {
                  const int ghost_offset =
                      nghostzones[d] - group_nghostzones[d];
                  const int begin_bnd = ghost_offset;
                  const int begin_int = nghostzones[d];
                  const int end_int = lsh[d] - offset[d] - nghostzones[d];
                  const int end_bnd = lsh[d] - offset[d] - ghost_offset;
                  switch (inormal[d]) {
                  case -1: // lower boundary
                    imin[d] = begin_bnd;
                    imax[d] = begin_int;
                    break;
                  case 0: // interior
                    imin[d] = begin_int;
                    imax[d] = end_int;
                    break;
                  case +1: // upper boundary
                    imin[d] = end_int;
                    imax[d] = end_bnd;
                    break;
                  default:
                    assert(0);
                  }

                  imin[d] = std::max(tmin[d], imin[d]);
                  imax[d] = std::min(tmax[d], imax[d]);
                }

                loop_box_device<CI, CJ, CK, VS>(f, imin, imax, inormal);
              }
            } // if rank
          }
        }
      }
    } // for rank
  }

  // Loop over all outer ghost points. This includes ghost edges/corners on
  // non-ghost faces. Loop over faces first, then edges, then corners.
  template <int CI, int CJ, int CK, int VS = 1, typename F>
  inline CCTK_ATTRIBUTE_ALWAYS_INLINE void
  loop_ghosts_inclusive_device(const std::array<int, dim> &group_nghostzones,
                               const F &f) const {
    const std::array<int, dim> offset{CI, CJ, CK};

    for (int rank = dim - 1; rank >= 0; --rank) {

      for (int nk = -1; nk <= +1; ++nk) {
        for (int nj = -1; nj <= +1; ++nj) {
          for (int ni = -1; ni <= +1; ++ni) {
            if ((ni == 0) + (nj == 0) + (nk == 0) == rank) {

              if ((ni != 0 && !bbox[0 + (ni == -1 ? 0 : 1)]) ||
                  (nj != 0 && !bbox[2 + (nj == -1 ? 0 : 1)]) ||
                  (nk != 0 && !bbox[4 + (nk == -1 ? 0 : 1)])) {

                const std::array<int, dim> inormal{ni, nj, nk};

                std::array<int, dim> imin, imax;
                for (int d = 0; d < dim; ++d) {
                  const int ghost_offset =
                      nghostzones[d] - group_nghostzones[d];
                  const int begin_bnd = ghost_offset;
                  const int begin_int = nghostzones[d];
                  const int end_int = lsh[d] - offset[d] - nghostzones[d];
                  const int end_bnd = lsh[d] - offset[d] - ghost_offset;
                  switch (inormal[d]) {
                  case -1: // lower boundary
                    imin[d] = begin_bnd;
                    imax[d] = begin_int;
                    break;
                  case 0: // interior
                    imin[d] = begin_int;
                    imax[d] = end_int;
                    break;
                  case +1: // upper boundary
                    imin[d] = end_int;
                    imax[d] = end_bnd;
                    break;
                  default:
                    assert(0);
                  }

                  imin[d] = std::max(tmin[d], imin[d]);
                  imax[d] = std::min(tmax[d], imax[d]);
                }

                loop_box_device<CI, CJ, CK, VS>(f, imin, imax, inormal);
              }
            } // if rank
          }
        }
      }
    } // for rank
  }

  template <int CI, int CJ, int CK, where_t where, typename F>
  inline CCTK_ATTRIBUTE_ALWAYS_INLINE
      std::enable_if_t<(where == where_t::everywhere), void>
      loop_device(const std::array<int, dim> &group_nghostzones,
                  const F &f) const {
    loop_all_device<CI, CJ, CK>(group_nghostzones, f);
  }
  template <int CI, int CJ, int CK, where_t where, typename F>
  inline CCTK_ATTRIBUTE_ALWAYS_INLINE
      std::enable_if_t<(where == where_t::interior), void>
      loop_device(const std::array<int, dim> &group_nghostzones,
                  const F &f) const {
    loop_int_device<CI, CJ, CK>(group_nghostzones, f);
  }
  template <int CI, int CJ, int CK, where_t where, typename F>
  inline CCTK_ATTRIBUTE_ALWAYS_INLINE
      std::enable_if_t<(where == where_t::boundary), void>
      loop_device(const std::array<int, dim> &group_nghostzones,
                  const F &f) const {
    loop_bnd_device<CI, CJ, CK>(group_nghostzones, f);
  }
  template <int CI, int CJ, int CK, where_t where, typename F>
  inline CCTK_ATTRIBUTE_ALWAYS_INLINE
      std::enable_if_t<(where == where_t::ghosts_inclusive), void>
      loop_device(const std::array<int, dim> &group_nghostzones,
                  const F &f) const {
    loop_ghosts_inclusive_device<CI, CJ, CK>(group_nghostzones, f);
  }
  template <int CI, int CJ, int CK, where_t where, typename F>
  inline CCTK_ATTRIBUTE_ALWAYS_INLINE
      std::enable_if_t<(where == where_t::ghosts), void>
      loop_device(const std::array<int, dim> &group_nghostzones,
                  const F &f) const {
    loop_ghosts_device<CI, CJ, CK>(group_nghostzones, f);
  }

  template <typename F>
  inline CCTK_ATTRIBUTE_ALWAYS_INLINE void loop_there_device_idx(
      const std::array<int, dim> &indextype,
      const std::array<std::array<std::array<bool, dim>, dim>, dim> &there,
      const std::array<int, dim> &group_nghostzones, const F &f) const {
    switch (indextype[0] + 2 * indextype[1] + 4 * indextype[2]) {
    case 0b000:
      return noinline([&] {
        return loop_there_device<0, 0, 0>(group_nghostzones, there, f);
      });
    case 0b001:
      return noinline([&] {
        return loop_there_device<1, 0, 0>(group_nghostzones, there, f);
      });
    case 0b010:
      return noinline([&] {
        return loop_there_device<0, 1, 0>(group_nghostzones, there, f);
      });
    case 0b011:
      return noinline([&] {
        return loop_there_device<1, 1, 0>(group_nghostzones, there, f);
      });
    case 0b100:
      return noinline([&] {
        return loop_there_device<0, 0, 1>(group_nghostzones, there, f);
      });
    case 0b101:
      return noinline([&] {
        return loop_there_device<1, 0, 1>(group_nghostzones, there, f);
      });
    case 0b110:
      return noinline([&] {
        return loop_there_device<0, 1, 1>(group_nghostzones, there, f);
      });
    case 0b111:
      return noinline([&] {
        return loop_there_device<1, 1, 1>(group_nghostzones, there, f);
      });
    default:
      assert(0);
    }
  }

  template <where_t where, typename F>
  inline CCTK_ATTRIBUTE_ALWAYS_INLINE void
  loop_device_idx(const std::array<int, dim> &indextype,
                  const std::array<int, dim> &group_nghostzones,
                  const F &f) const {
    switch (indextype[0] + 2 * indextype[1] + 4 * indextype[2]) {
    case 0b000:
      return noinline(
          [&] { return loop_device<0, 0, 0, where>(group_nghostzones, f); });
    case 0b001:
      return noinline(
          [&] { return loop_device<1, 0, 0, where>(group_nghostzones, f); });
    case 0b010:
      return noinline(
          [&] { return loop_device<0, 1, 0, where>(group_nghostzones, f); });
    case 0b011:
      return noinline(
          [&] { return loop_device<1, 1, 0, where>(group_nghostzones, f); });
    case 0b100:
      return noinline(
          [&] { return loop_device<0, 0, 1, where>(group_nghostzones, f); });
    case 0b101:
      return noinline(
          [&] { return loop_device<1, 0, 1, where>(group_nghostzones, f); });
    case 0b110:
      return noinline(
          [&] { return loop_device<0, 1, 1, where>(group_nghostzones, f); });
    case 0b111:
      return noinline(
          [&] { return loop_device<1, 1, 1, where>(group_nghostzones, f); });
    default:
      assert(0);
    }
  }
};

template <where_t where, typename F>
inline CCTK_ATTRIBUTE_ALWAYS_INLINE void
loop_device_idx(const cGH *cctkGH, const std::array<int, dim> &indextype,
                const std::array<int, dim> &nghostzones, const F &f) {
  GridDescBaseDevice(cctkGH).loop_device_idx<where>(indextype, nghostzones, f);
}

} // namespace Loop

#undef CCTK_CENTERING_GRID
#define CCTK_CENTERING_GRID const Loop::GridDescBaseDevice cctk_grid(cctkGH)

#endif // #ifndef LOOP_DEVICE_HXX
