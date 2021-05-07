#include "physics.hxx"

#include <loop_device.hxx>
#include <mat.hxx>
#include <simd.hxx>
#include <vec.hxx>

#include <fixmath.hxx> // include this before <cctk.h>
#include <cctk.h>
#include <cctk_Arguments_Checked.h>

#include <cmath>

namespace Z4c {
using namespace Arith;
using namespace Loop;
using namespace std;

extern "C" void Z4c_Initial1(CCTK_ARGUMENTS) {
  DECLARE_CCTK_ARGUMENTS_Z4c_Initial1;

  const array<int, dim> indextype = {0, 0, 0};
  const GF3D2layout layout1(cctkGH, indextype);

  const smat<GF3D2<const CCTK_REAL>, 3, DN, DN> gf_g1{
      GF3D2<const CCTK_REAL>(layout1, gxx),
      GF3D2<const CCTK_REAL>(layout1, gxy),
      GF3D2<const CCTK_REAL>(layout1, gxz),
      GF3D2<const CCTK_REAL>(layout1, gyy),
      GF3D2<const CCTK_REAL>(layout1, gyz),
      GF3D2<const CCTK_REAL>(layout1, gzz)};

  const smat<GF3D2<const CCTK_REAL>, 3, DN, DN> gf_K1{
      GF3D2<const CCTK_REAL>(layout1, kxx),
      GF3D2<const CCTK_REAL>(layout1, kxy),
      GF3D2<const CCTK_REAL>(layout1, kxz),
      GF3D2<const CCTK_REAL>(layout1, kyy),
      GF3D2<const CCTK_REAL>(layout1, kyz),
      GF3D2<const CCTK_REAL>(layout1, kzz)};

  const GF3D2<const CCTK_REAL> gf_alp1(layout1, alp);

  const vec<GF3D2<const CCTK_REAL>, 3, UP> gf_beta1{
      GF3D2<const CCTK_REAL>(layout1, betax),
      GF3D2<const CCTK_REAL>(layout1, betay),
      GF3D2<const CCTK_REAL>(layout1, betaz)};

  const GF3D2<CCTK_REAL> gf_chi1(layout1, chi);

  const smat<GF3D2<CCTK_REAL>, 3, DN, DN> gf_gammat1{
      GF3D2<CCTK_REAL>(layout1, gammatxx), GF3D2<CCTK_REAL>(layout1, gammatxy),
      GF3D2<CCTK_REAL>(layout1, gammatxz), GF3D2<CCTK_REAL>(layout1, gammatyy),
      GF3D2<CCTK_REAL>(layout1, gammatyz), GF3D2<CCTK_REAL>(layout1, gammatzz)};

  const GF3D2<CCTK_REAL> gf_Kh1(layout1, Kh);

  const smat<GF3D2<CCTK_REAL>, 3, DN, DN> gf_At1{
      GF3D2<CCTK_REAL>(layout1, Atxx), GF3D2<CCTK_REAL>(layout1, Atxy),
      GF3D2<CCTK_REAL>(layout1, Atxz), GF3D2<CCTK_REAL>(layout1, Atyy),
      GF3D2<CCTK_REAL>(layout1, Atyz), GF3D2<CCTK_REAL>(layout1, Atzz)};

  const GF3D2<CCTK_REAL> gf_Theta1(layout1, Theta);

  const GF3D2<CCTK_REAL> gf_alphaG1(layout1, alphaG);

  const vec<GF3D2<CCTK_REAL>, 3, UP> gf_betaG1{
      GF3D2<CCTK_REAL>(layout1, betaGx), GF3D2<CCTK_REAL>(layout1, betaGy),
      GF3D2<CCTK_REAL>(layout1, betaGz)};

  typedef simd<CCTK_REAL> vreal;
  typedef simdl<CCTK_REAL> vbool;
  constexpr size_t vsize = tuple_size_v<vreal>;

  const Loop::GridDescBaseDevice grid(cctkGH);
  grid.loop_all_device<0, 0, 0, vsize>(
      grid.nghostzones,
      [=] ARITH_DEVICE ARITH_HOST(const PointDesc &p) ARITH_INLINE {
        const vbool mask = mask_for_loop_tail<vbool>(p.i, p.imax);
        const GF3D2index index1(layout1, p.I);

        // Load
        const smat<vreal, 3, DN, DN> g =
            gf_g1(mask, index1, one<smat<vreal, 3, DN, DN> >()());
        const smat<vreal, 3, DN, DN> K = gf_K1(mask, index1);
        const vreal alp = gf_alp1(mask, index1, 1);
        const vec<vreal, 3, UP> beta = gf_beta1(mask, index1);

        // Calculate Z4c variables (all except Gammat)
        const vreal detg = calc_det(g);
        const smat<vreal, 3, UP, UP> gu = calc_inv(g, detg);

        const vreal chi = 1 / cbrt(detg);

        const smat<vreal, 3, DN, DN> gammat(
            [&](int a, int b) ARITH_INLINE { return chi * g(a, b); });

        const vreal trK = sum_symm<3>(
            [&](int x, int y) ARITH_INLINE { return gu(x, y) * K(x, y); });

        const vreal Theta = 0;

        const vreal Kh = trK - 2 * Theta;

        const smat<vreal, 3, DN, DN> At([&](int a, int b) ARITH_INLINE {
          return chi * (K(a, b) - trK / 3 * g(a, b));
        });

        const vreal alphaG = alp;

        const vec<vreal, 3, UP> betaG([&](int a)
                                          ARITH_INLINE { return beta(a); });

        // Store
        gf_chi1.store(mask, index1, chi);
        gf_gammat1.store(mask, index1, gammat);
        gf_Kh1.store(mask, index1, Kh);
        gf_At1.store(mask, index1, At);
        gf_Theta1.store(mask, index1, Theta);
        gf_alphaG1.store(mask, index1, alphaG);
        gf_betaG1.store(mask, index1, betaG);
      });
}

} // namespace Z4c
