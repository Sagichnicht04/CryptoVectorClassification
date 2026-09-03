// Copyright Eric Chauvin, 2021 -2023.




// This is licensed under the GNU General
// Public License (GPL).  It is the
// same license that Linux has.
// https://www.gnu.org/licenses/gpl-3.0.html


#pragma once



#include "../CppBase/BasicTypes.h"
#include "../CppInt/Integer.h"
#include "../CppInt/IntegerMath.h"
#include "../CppInt/Mod.h"
// #include "../CppInt/Exponents.h"
#include "FindFacSm.h"
// #include "FindFacQr.h"
#include "../Crt/MultInv.h"
#include "../Crt/CrtMath.h"
#include "../Crt/GarnerCrt.h"


// RFC 8017.



class Rsa
  {
  private:
  bool testForCopy = false;

  // 65537 is a prime.
  // It is a commonly used exponent for RSA.
  //  It is 2^16 + 1.
  // Int32 PubKeyExponentL = 65537;

  Integer primeP;
  Integer primeQ;
  Integer primePMinus1;
  Integer primeQMinus1;
  Integer pubKeyN;
  Integer pubKeyExponent;
  Integer privKInverseExponent;
  Integer privKInverseExponentDP;
  Integer privKInverseExponentDQ;
  Integer qInv;
  Integer phiN;
  // Exponents expTest;


  public:
  // 65537 is a prime.
  // It is a commonly used exponent for RSA.
  //  It is 2^16 + 1.
  // static const Int32 PubKeyExponentL = 65537;

  Rsa( void )
    {
    }

  Rsa( const Rsa& in )
    {
    if( in.testForCopy )
      return;

    throw "Copy constructor for Rsa.";
    }


  ~Rsa( void )
    {
    }


  bool makeKeys( const Integer& prime1,
                 const Integer& prime2,
                 IntegerMath& intMath,
                 Mod& mod,
                 SPrimes& sPrimes,
                 FindFacSm& findFacSm,
                 // FindFacQr& findFacQr,
                 const MultInv& multInv,
                 // const QuadRes& quadRes
                 const CrtMath& crtMath,
                 GarnerCrt& garnerCrt );

  bool isGoodPair( IntegerMath& intMath );

  bool setPrivateKeys( IntegerMath& intMath );
  bool testEncryption( Mod& mod,
                   IntegerMath& intMath,
                   const SPrimes& sPrimes,
                   const MultInv& multInv,
                   const CrtMath& crtMath );

  };
