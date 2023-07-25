#ifndef EOSPOLYTROPIC_HXX
#define EOSPOLYTROPIC_HXX

#include "eos_1p.hxx"
#include <cmath>

namespace EOSX {

/// Polytropic EOS
/**
The polytropic EOS is written (using units with \f$ c=1 \f$) as
\f[ P = \rho_p \left(\frac{\rho}{\rho_p}\right)^\Gamma ,\qquad
\Gamma= 1+\frac{1}{n}  \f]
Here we use a <em>polytropic density scale</em> \f$ \rho_p \f$ to specify
the EOS instead of the usual form \f$ P = K \rho^\Gamma \f$
because it has simpler units than
\f$ K = \rho_p^{-1/n} \f$.

See eos_cold for notation used and eos_cold_api for a description of the member
functions.
*/

class eos_polytrope : public eos_1p {
  CCTK_REAL n;     ///< Polytropic index \f$ n \f$
  CCTK_REAL rho_p; ///< Polytropic density scale \f$ \rho_p \f$
  CCTK_REAL np1;   ///< \f$ n+1 \f$
  CCTK_REAL gamma; ///< Polytropic exponent \f$ \Gamma \f$
  CCTK_REAL invn;  ///< \f$ \frac{1}{n} \f$

  CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline void
  init(CCTK_REAL n_,      ///< Adiabatic index \f$ n \f$
       CCTK_REAL rho_p_,  ///< Density scale \f$ \rho_p \f$
       CCTK_REAL rho_max_ ///< Max valid density
  );

public:
  /// Constructor
  CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline eos_polytrope(
      CCTK_REAL n_,      ///< Adiabatic index \f$ n \f$
      CCTK_REAL rho_p_,  ///< Density scale \f$ \rho_p \f$
      CCTK_REAL rho_max_ ///< Max valid density
  );

  CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
  gm1_from_valid_rho(const CCTK_REAL rho) const;
  CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
  gm1_from_valid_p(const CCTK_REAL p) const;
  CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
  sed_from_valid_gm1(const CCTK_REAL gm1) const;
  CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
  ied_from_valid_gm1(const CCTK_REAL gm1) const;
  CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
  p_from_valid_gm1(const CCTK_REAL gm1) const;
  CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
  rho_from_valid_gm1(const CCTK_REAL gm1) const;
  CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
  hm1_from_valid_gm1(const CCTK_REAL gm1) const;
  CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
  csnd2_from_valid_gm1(const CCTK_REAL gm1) const;
};

CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline void
eos_polytrope::init(CCTK_REAL n_, CCTK_REAL rho_p_, CCTK_REAL rho_max_) {
  n = n_;
  rho_p = rho_p_;
  np1 = n + 1;
  gamma = 1.0 + 1.0 / n;
  invn = 1.0 / n;

  // set_ranges(range(0, rho_max_));
}

CCTK_DEVICE
CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline eos_polytrope::eos_polytrope(
    CCTK_REAL n_, CCTK_REAL rho_p_, CCTK_REAL rho_max_) {
  init(n_, rho_p_, rho_max_);
}

CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
eos_polytrope::gm1_from_valid_rho(const CCTK_REAL rho) const {
  return np1 * pow(rho / rho_p, invn);
}

/**
\return \f$ g-1 = h-1 = (n+1) \left(\frac{P}{\rho_p} \right)^\frac{1}{n+1} \f$
*/
CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
eos_polytrope::gm1_from_valid_p(const CCTK_REAL p) const {
  return np1 * pow(p / rho_p, 1.0 / np1);
}

/**
\return Specific internal energy \f$ \epsilon = \frac{g-1}{\Gamma} \f$
*/
CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
eos_polytrope::sed_from_valid_gm1(const CCTK_REAL gm1 ///< \f$ g-1 \f$
) const {
  return gm1 / gamma;
}

/**
\return Internal energy density
\f$ \rho_I = n \rho_p \left(\frac{g-1}{n+1}\right)^{n+1} \f$
*/
CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
eos_polytrope::ied_from_valid_gm1(const CCTK_REAL gm1 ///< \f$ g-1 \f$
) const {
  // return sed_from_gm1(gm1)*rho_from_gm1(gm1);
  return n * rho_p * pow(gm1 / np1, np1);
}

/**
\return Pressure \f$ P = \rho_p \left( \frac{g-1}{1+n} \right)^{1+n} \f$
*/
CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
eos_polytrope::p_from_valid_gm1(const CCTK_REAL gm1 ///< \f$ g-1 \f$
) const {
  return rho_p * pow(gm1 / np1, np1);
}

/**
\return Rest mass density \f$ \rho = \rho_p \left( \frac{g-1}{1+n} \right)^n \f$
*/
CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
eos_polytrope::rho_from_valid_gm1(const CCTK_REAL gm1 ///< \f$ g-1 \f$
) const {
  return rho_p * pow(gm1 / np1, n);
}

/**
\return specific enthalpy \f$ h-1 = g-1 \f$
*/
CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
eos_polytrope::hm1_from_valid_gm1(const CCTK_REAL gm1 ///< \f$ g-1 \f$
) const {
  return gm1;
}

/**
\return Soundspeed squared \f$ c_s^2 = \frac{g-1}{ng} \f$
*/
CCTK_DEVICE CCTK_HOST CCTK_ATTRIBUTE_ALWAYS_INLINE inline CCTK_REAL
eos_polytrope::csnd2_from_valid_gm1(const CCTK_REAL gm1) const {
  return gm1 / (n * (gm1 + 1));
}

} // namespace EOSX

#endif
