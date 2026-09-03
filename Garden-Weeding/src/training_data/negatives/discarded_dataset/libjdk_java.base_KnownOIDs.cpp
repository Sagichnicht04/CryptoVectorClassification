#include <sun/security/util/KnownOIDs.h>
#include <java/lang/Enum.h>
#include <java/util/Locale.h>
#include <java/util/Objects.h>
#include <java/util/concurrent/ConcurrentHashMap.h>
#include <sun/security/util/Debug.h>
#include <sun/security/util/KnownOIDs$1.h>
#include <sun/security/util/KnownOIDs$10.h>
#include <sun/security/util/KnownOIDs$2.h>
#include <sun/security/util/KnownOIDs$3.h>
#include <sun/security/util/KnownOIDs$4.h>
#include <sun/security/util/KnownOIDs$5.h>
#include <sun/security/util/KnownOIDs$6.h>
#include <sun/security/util/KnownOIDs$7.h>
#include <sun/security/util/KnownOIDs$8.h>
#include <sun/security/util/KnownOIDs$9.h>
#include <jcpp.h>

#undef AES
#undef ARCFOUR
#undef CE_CERT_POLICIES_ANY
#undef DSA
#undef EC
#undef ECDH
#undef ENGLISH
#undef ITUX509_RSA
#undef MD2
#undef MD5
#undef MGF1
#undef OAEP
#undef OCSP
#undef OIW_DES_CBC
#undef OIW_DSA
#undef PBES2
#undef PKCS1
#undef PKCS7
#undef RSA
#undef RSASSA_PSS
#undef SHAKE128
#undef SHAKE128_LEN
#undef SHAKE256
#undef SHAKE256_LEN
#undef SHA_1

using $KnownOIDsArray = $Array<::sun::security::util::KnownOIDs>;
using $ClassInfo = ::java::lang::ClassInfo;
using $Enum = ::java::lang::Enum;
using $FieldInfo = ::java::lang::FieldInfo;
using $InnerClassInfo = ::java::lang::InnerClassInfo;
using $MethodInfo = ::java::lang::MethodInfo;
using $RuntimeException = ::java::lang::RuntimeException;
using $Locale = ::java::util::Locale;
using $Objects = ::java::util::Objects;
using $ConcurrentHashMap = ::java::util::concurrent::ConcurrentHashMap;
using $Debug = ::sun::security::util::Debug;
using $KnownOIDs$1 = ::sun::security::util::KnownOIDs$1;
using $KnownOIDs$10 = ::sun::security::util::KnownOIDs$10;
using $KnownOIDs$2 = ::sun::security::util::KnownOIDs$2;
using $KnownOIDs$3 = ::sun::security::util::KnownOIDs$3;
using $KnownOIDs$4 = ::sun::security::util::KnownOIDs$4;
using $KnownOIDs$5 = ::sun::security::util::KnownOIDs$5;
using $KnownOIDs$6 = ::sun::security::util::KnownOIDs$6;
using $KnownOIDs$7 = ::sun::security::util::KnownOIDs$7;
using $KnownOIDs$8 = ::sun::security::util::KnownOIDs$8;
using $KnownOIDs$9 = ::sun::security::util::KnownOIDs$9;

namespace sun {
	namespace security {
		namespace util {

KnownOIDs* KnownOIDs::CommonName = nullptr;
KnownOIDs* KnownOIDs::Surname = nullptr;
KnownOIDs* KnownOIDs::SerialNumber = nullptr;
KnownOIDs* KnownOIDs::CountryName = nullptr;
KnownOIDs* KnownOIDs::LocalityName = nullptr;
KnownOIDs* KnownOIDs::StateName = nullptr;
KnownOIDs* KnownOIDs::StreetAddress = nullptr;
KnownOIDs* KnownOIDs::OrgName = nullptr;
KnownOIDs* KnownOIDs::OrgUnitName = nullptr;
KnownOIDs* KnownOIDs::Title = nullptr;
KnownOIDs* KnownOIDs::GivenName = nullptr;
KnownOIDs* KnownOIDs::Initials = nullptr;
KnownOIDs* KnownOIDs::GenerationQualifier = nullptr;
KnownOIDs* KnownOIDs::DNQualifier = nullptr;
KnownOIDs* KnownOIDs::SubjectDirectoryAttributes = nullptr;
KnownOIDs* KnownOIDs::SubjectKeyID = nullptr;
KnownOIDs* KnownOIDs::KeyUsage = nullptr;
KnownOIDs* KnownOIDs::PrivateKeyUsage = nullptr;
KnownOIDs* KnownOIDs::SubjectAlternativeName = nullptr;
KnownOIDs* KnownOIDs::IssuerAlternativeName = nullptr;
KnownOIDs* KnownOIDs::BasicConstraints = nullptr;
KnownOIDs* KnownOIDs::CRLNumber = nullptr;
KnownOIDs* KnownOIDs::ReasonCode = nullptr;
KnownOIDs* KnownOIDs::HoldInstructionCode = nullptr;
KnownOIDs* KnownOIDs::InvalidityDate = nullptr;
KnownOIDs* KnownOIDs::DeltaCRLIndicator = nullptr;
KnownOIDs* KnownOIDs::IssuingDistributionPoint = nullptr;
KnownOIDs* KnownOIDs::CertificateIssuer = nullptr;
KnownOIDs* KnownOIDs::NameConstraints = nullptr;
KnownOIDs* KnownOIDs::CRLDistributionPoints = nullptr;
KnownOIDs* KnownOIDs::CertificatePolicies = nullptr;
KnownOIDs* KnownOIDs::CE_CERT_POLICIES_ANY = nullptr;
KnownOIDs* KnownOIDs::PolicyMappings = nullptr;
KnownOIDs* KnownOIDs::AuthorityKeyID = nullptr;
KnownOIDs* KnownOIDs::PolicyConstraints = nullptr;
KnownOIDs* KnownOIDs::extendedKeyUsage = nullptr;
KnownOIDs* KnownOIDs::anyExtendedKeyUsage = nullptr;
KnownOIDs* KnownOIDs::FreshestCRL = nullptr;
KnownOIDs* KnownOIDs::InhibitAnyPolicy = nullptr;
KnownOIDs* KnownOIDs::AuthInfoAccess = nullptr;
KnownOIDs* KnownOIDs::SubjectInfoAccess = nullptr;
KnownOIDs* KnownOIDs::serverAuth = nullptr;
KnownOIDs* KnownOIDs::clientAuth = nullptr;
KnownOIDs* KnownOIDs::codeSigning = nullptr;
KnownOIDs* KnownOIDs::emailProtection = nullptr;
KnownOIDs* KnownOIDs::ipsecEndSystem = nullptr;
KnownOIDs* KnownOIDs::ipsecTunnel = nullptr;
KnownOIDs* KnownOIDs::ipsecUser = nullptr;
KnownOIDs* KnownOIDs::KP_TimeStamping = nullptr;
KnownOIDs* KnownOIDs::OCSPSigning = nullptr;
KnownOIDs* KnownOIDs::OCSP = nullptr;
KnownOIDs* KnownOIDs::OCSPBasicResponse = nullptr;
KnownOIDs* KnownOIDs::OCSPNonceExt = nullptr;
KnownOIDs* KnownOIDs::OCSPNoCheck = nullptr;
KnownOIDs* KnownOIDs::caIssuers = nullptr;
KnownOIDs* KnownOIDs::AD_TimeStamping = nullptr;
KnownOIDs* KnownOIDs::caRepository = nullptr;
KnownOIDs* KnownOIDs::AES = nullptr;
KnownOIDs* KnownOIDs::AES_128$ECB$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_128$CBC$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_128$OFB$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_128$CFB$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_128$KW$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_128$GCM$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_128$KWP$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_192$ECB$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_192$CBC$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_192$OFB$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_192$CFB$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_192$KW$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_192$GCM$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_192$KWP$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_256$ECB$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_256$CBC$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_256$OFB$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_256$CFB$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_256$KW$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_256$GCM$NoPadding = nullptr;
KnownOIDs* KnownOIDs::AES_256$KWP$NoPadding = nullptr;
KnownOIDs* KnownOIDs::SHA_256 = nullptr;
KnownOIDs* KnownOIDs::SHA_384 = nullptr;
KnownOIDs* KnownOIDs::SHA_512 = nullptr;
KnownOIDs* KnownOIDs::SHA_224 = nullptr;
KnownOIDs* KnownOIDs::SHA_512$224 = nullptr;
KnownOIDs* KnownOIDs::SHA_512$256 = nullptr;
KnownOIDs* KnownOIDs::SHA3_224 = nullptr;
KnownOIDs* KnownOIDs::SHA3_256 = nullptr;
KnownOIDs* KnownOIDs::SHA3_384 = nullptr;
KnownOIDs* KnownOIDs::SHA3_512 = nullptr;
KnownOIDs* KnownOIDs::SHAKE128 = nullptr;
KnownOIDs* KnownOIDs::SHAKE256 = nullptr;
KnownOIDs* KnownOIDs::HmacSHA3_224 = nullptr;
KnownOIDs* KnownOIDs::HmacSHA3_256 = nullptr;
KnownOIDs* KnownOIDs::HmacSHA3_384 = nullptr;
KnownOIDs* KnownOIDs::HmacSHA3_512 = nullptr;
KnownOIDs* KnownOIDs::SHAKE128_LEN = nullptr;
KnownOIDs* KnownOIDs::SHAKE256_LEN = nullptr;
KnownOIDs* KnownOIDs::SHA224withDSA = nullptr;
KnownOIDs* KnownOIDs::SHA256withDSA = nullptr;
KnownOIDs* KnownOIDs::SHA384withDSA = nullptr;
KnownOIDs* KnownOIDs::SHA512withDSA = nullptr;
KnownOIDs* KnownOIDs::SHA3_224withDSA = nullptr;
KnownOIDs* KnownOIDs::SHA3_256withDSA = nullptr;
KnownOIDs* KnownOIDs::SHA3_384withDSA = nullptr;
KnownOIDs* KnownOIDs::SHA3_512withDSA = nullptr;
KnownOIDs* KnownOIDs::SHA3_224withECDSA = nullptr;
KnownOIDs* KnownOIDs::SHA3_256withECDSA = nullptr;
KnownOIDs* KnownOIDs::SHA3_384withECDSA = nullptr;
KnownOIDs* KnownOIDs::SHA3_512withECDSA = nullptr;
KnownOIDs* KnownOIDs::SHA3_224withRSA = nullptr;
KnownOIDs* KnownOIDs::SHA3_256withRSA = nullptr;
KnownOIDs* KnownOIDs::SHA3_384withRSA = nullptr;
KnownOIDs* KnownOIDs::SHA3_512withRSA = nullptr;
KnownOIDs* KnownOIDs::PKCS1 = nullptr;
KnownOIDs* KnownOIDs::RSA = nullptr;
KnownOIDs* KnownOIDs::MD2withRSA = nullptr;
KnownOIDs* KnownOIDs::MD5withRSA = nullptr;
KnownOIDs* KnownOIDs::SHA1withRSA = nullptr;
KnownOIDs* KnownOIDs::OAEP = nullptr;
KnownOIDs* KnownOIDs::MGF1 = nullptr;
KnownOIDs* KnownOIDs::PSpecified = nullptr;
KnownOIDs* KnownOIDs::RSASSA_PSS = nullptr;
KnownOIDs* KnownOIDs::SHA256withRSA = nullptr;
KnownOIDs* KnownOIDs::SHA384withRSA = nullptr;
KnownOIDs* KnownOIDs::SHA512withRSA = nullptr;
KnownOIDs* KnownOIDs::SHA224withRSA = nullptr;
KnownOIDs* KnownOIDs::SHA512$224withRSA = nullptr;
KnownOIDs* KnownOIDs::SHA512$256withRSA = nullptr;
KnownOIDs* KnownOIDs::DiffieHellman = nullptr;
KnownOIDs* KnownOIDs::PBEWithMD5AndDES = nullptr;
KnownOIDs* KnownOIDs::PBEWithMD5AndRC2 = nullptr;
KnownOIDs* KnownOIDs::PBEWithSHA1AndDES = nullptr;
KnownOIDs* KnownOIDs::PBEWithSHA1AndRC2 = nullptr;
KnownOIDs* KnownOIDs::PBKDF2WithHmacSHA1 = nullptr;
KnownOIDs* KnownOIDs::PBES2 = nullptr;
KnownOIDs* KnownOIDs::PKCS7 = nullptr;
KnownOIDs* KnownOIDs::Data = nullptr;
KnownOIDs* KnownOIDs::SignedData = nullptr;
KnownOIDs* KnownOIDs::JDK_OLD_Data = nullptr;
KnownOIDs* KnownOIDs::JDK_OLD_SignedData = nullptr;
KnownOIDs* KnownOIDs::EnvelopedData = nullptr;
KnownOIDs* KnownOIDs::SignedAndEnvelopedData = nullptr;
KnownOIDs* KnownOIDs::DigestedData = nullptr;
KnownOIDs* KnownOIDs::EncryptedData = nullptr;
KnownOIDs* KnownOIDs::EmailAddress = nullptr;
KnownOIDs* KnownOIDs::UnstructuredName = nullptr;
KnownOIDs* KnownOIDs::ContentType = nullptr;
KnownOIDs* KnownOIDs::MessageDigest = nullptr;
KnownOIDs* KnownOIDs::SigningTime = nullptr;
KnownOIDs* KnownOIDs::CounterSignature = nullptr;
KnownOIDs* KnownOIDs::ChallengePassword = nullptr;
KnownOIDs* KnownOIDs::UnstructuredAddress = nullptr;
KnownOIDs* KnownOIDs::ExtendedCertificateAttributes = nullptr;
KnownOIDs* KnownOIDs::IssuerAndSerialNumber = nullptr;
KnownOIDs* KnownOIDs::ExtensionRequest = nullptr;
KnownOIDs* KnownOIDs::SMIMECapability = nullptr;
KnownOIDs* KnownOIDs::TimeStampTokenInfo = nullptr;
KnownOIDs* KnownOIDs::SigningCertificate = nullptr;
KnownOIDs* KnownOIDs::SignatureTimestampToken = nullptr;
KnownOIDs* KnownOIDs::CHACHA20_POLY1305 = nullptr;
KnownOIDs* KnownOIDs::FriendlyName = nullptr;
KnownOIDs* KnownOIDs::LocalKeyID = nullptr;
KnownOIDs* KnownOIDs::CertTypeX509 = nullptr;
KnownOIDs* KnownOIDs::CMSAlgorithmProtection = nullptr;
KnownOIDs* KnownOIDs::PBEWithSHA1AndRC4_128 = nullptr;
KnownOIDs* KnownOIDs::PBEWithSHA1AndRC4_40 = nullptr;
KnownOIDs* KnownOIDs::PBEWithSHA1AndDESede = nullptr;
KnownOIDs* KnownOIDs::PBEWithSHA1AndRC2_128 = nullptr;
KnownOIDs* KnownOIDs::PBEWithSHA1AndRC2_40 = nullptr;
KnownOIDs* KnownOIDs::PKCS8ShroudedKeyBag = nullptr;
KnownOIDs* KnownOIDs::CertBag = nullptr;
KnownOIDs* KnownOIDs::SecretBag = nullptr;
KnownOIDs* KnownOIDs::MD2 = nullptr;
KnownOIDs* KnownOIDs::MD5 = nullptr;
KnownOIDs* KnownOIDs::HmacSHA1 = nullptr;
KnownOIDs* KnownOIDs::HmacSHA224 = nullptr;
KnownOIDs* KnownOIDs::HmacSHA256 = nullptr;
KnownOIDs* KnownOIDs::HmacSHA384 = nullptr;
KnownOIDs* KnownOIDs::HmacSHA512 = nullptr;
KnownOIDs* KnownOIDs::HmacSHA512$224 = nullptr;
KnownOIDs* KnownOIDs::HmacSHA512$256 = nullptr;
KnownOIDs* KnownOIDs::RC2$CBC$PKCS5Padding = nullptr;
KnownOIDs* KnownOIDs::ARCFOUR = nullptr;
KnownOIDs* KnownOIDs::DESede$CBC$NoPadding = nullptr;
KnownOIDs* KnownOIDs::RC5$CBC$PKCS5Padding = nullptr;
KnownOIDs* KnownOIDs::DSA = nullptr;
KnownOIDs* KnownOIDs::SHA1withDSA = nullptr;
KnownOIDs* KnownOIDs::EC = nullptr;
KnownOIDs* KnownOIDs::c2tnb191v1 = nullptr;
KnownOIDs* KnownOIDs::c2tnb191v2 = nullptr;
KnownOIDs* KnownOIDs::c2tnb191v3 = nullptr;
KnownOIDs* KnownOIDs::c2tnb239v1 = nullptr;
KnownOIDs* KnownOIDs::c2tnb239v2 = nullptr;
KnownOIDs* KnownOIDs::c2tnb239v3 = nullptr;
KnownOIDs* KnownOIDs::c2tnb359v1 = nullptr;
KnownOIDs* KnownOIDs::c2tnb431r1 = nullptr;
KnownOIDs* KnownOIDs::secp192r1 = nullptr;
KnownOIDs* KnownOIDs::prime192v2 = nullptr;
KnownOIDs* KnownOIDs::prime192v3 = nullptr;
KnownOIDs* KnownOIDs::prime239v1 = nullptr;
KnownOIDs* KnownOIDs::prime239v2 = nullptr;
KnownOIDs* KnownOIDs::prime239v3 = nullptr;
KnownOIDs* KnownOIDs::secp256r1 = nullptr;
KnownOIDs* KnownOIDs::SHA1withECDSA = nullptr;
KnownOIDs* KnownOIDs::SHA224withECDSA = nullptr;
KnownOIDs* KnownOIDs::SHA256withECDSA = nullptr;
KnownOIDs* KnownOIDs::SHA384withECDSA = nullptr;
KnownOIDs* KnownOIDs::SHA512withECDSA = nullptr;
KnownOIDs* KnownOIDs::SpecifiedSHA2withECDSA = nullptr;
KnownOIDs* KnownOIDs::X942_DH = nullptr;
KnownOIDs* KnownOIDs::brainpoolP160r1 = nullptr;
KnownOIDs* KnownOIDs::brainpoolP192r1 = nullptr;
KnownOIDs* KnownOIDs::brainpoolP224r1 = nullptr;
KnownOIDs* KnownOIDs::brainpoolP256r1 = nullptr;
KnownOIDs* KnownOIDs::brainpoolP320r1 = nullptr;
KnownOIDs* KnownOIDs::brainpoolP384r1 = nullptr;
KnownOIDs* KnownOIDs::brainpoolP512r1 = nullptr;
KnownOIDs* KnownOIDs::sect163k1 = nullptr;
KnownOIDs* KnownOIDs::sect163r1 = nullptr;
KnownOIDs* KnownOIDs::sect239k1 = nullptr;
KnownOIDs* KnownOIDs::sect113r1 = nullptr;
KnownOIDs* KnownOIDs::sect113r2 = nullptr;
KnownOIDs* KnownOIDs::secp112r1 = nullptr;
KnownOIDs* KnownOIDs::secp112r2 = nullptr;
KnownOIDs* KnownOIDs::secp160r1 = nullptr;
KnownOIDs* KnownOIDs::secp160k1 = nullptr;
KnownOIDs* KnownOIDs::secp256k1 = nullptr;
KnownOIDs* KnownOIDs::sect163r2 = nullptr;
KnownOIDs* KnownOIDs::sect283k1 = nullptr;
KnownOIDs* KnownOIDs::sect283r1 = nullptr;
KnownOIDs* KnownOIDs::sect131r1 = nullptr;
KnownOIDs* KnownOIDs::sect131r2 = nullptr;
KnownOIDs* KnownOIDs::sect193r1 = nullptr;
KnownOIDs* KnownOIDs::sect193r2 = nullptr;
KnownOIDs* KnownOIDs::sect233k1 = nullptr;
KnownOIDs* KnownOIDs::sect233r1 = nullptr;
KnownOIDs* KnownOIDs::secp128r1 = nullptr;
KnownOIDs* KnownOIDs::secp128r2 = nullptr;
KnownOIDs* KnownOIDs::secp160r2 = nullptr;
KnownOIDs* KnownOIDs::secp192k1 = nullptr;
KnownOIDs* KnownOIDs::secp224k1 = nullptr;
KnownOIDs* KnownOIDs::secp224r1 = nullptr;
KnownOIDs* KnownOIDs::secp384r1 = nullptr;
KnownOIDs* KnownOIDs::secp521r1 = nullptr;
KnownOIDs* KnownOIDs::sect409k1 = nullptr;
KnownOIDs* KnownOIDs::sect409r1 = nullptr;
KnownOIDs* KnownOIDs::sect571k1 = nullptr;
KnownOIDs* KnownOIDs::sect571r1 = nullptr;
KnownOIDs* KnownOIDs::ECDH = nullptr;
KnownOIDs* KnownOIDs::OIW_DES_CBC = nullptr;
KnownOIDs* KnownOIDs::OIW_DSA = nullptr;
KnownOIDs* KnownOIDs::OIW_JDK_SHA1withDSA = nullptr;
KnownOIDs* KnownOIDs::OIW_SHA1withRSA_Odd = nullptr;
KnownOIDs* KnownOIDs::DESede = nullptr;
KnownOIDs* KnownOIDs::SHA_1 = nullptr;
KnownOIDs* KnownOIDs::OIW_SHA1withDSA = nullptr;
KnownOIDs* KnownOIDs::OIW_SHA1withRSA = nullptr;
KnownOIDs* KnownOIDs::X25519 = nullptr;
KnownOIDs* KnownOIDs::X448 = nullptr;
KnownOIDs* KnownOIDs::Ed25519 = nullptr;
KnownOIDs* KnownOIDs::Ed448 = nullptr;
KnownOIDs* KnownOIDs::UCL_UserID = nullptr;
KnownOIDs* KnownOIDs::UCL_DomainComponent = nullptr;
KnownOIDs* KnownOIDs::NETSCAPE_CertType = nullptr;
KnownOIDs* KnownOIDs::NETSCAPE_CertSequence = nullptr;
KnownOIDs* KnownOIDs::NETSCAPE_ExportApproved = nullptr;
KnownOIDs* KnownOIDs::ORACLE_TrustedKeyUsage = nullptr;
KnownOIDs* KnownOIDs::ITUX509_RSA = nullptr;
KnownOIDs* KnownOIDs::SkipIPAddress = nullptr;
KnownOIDs* KnownOIDs::JAVASOFT_JDKKeyProtector = nullptr;
KnownOIDs* KnownOIDs::JAVASOFT_JCEKeyProtector = nullptr;
KnownOIDs* KnownOIDs::MICROSOFT_ExportApproved = nullptr;
$KnownOIDsArray* KnownOIDs::$VALUES = nullptr;
$Debug* KnownOIDs::debug = nullptr;
$ConcurrentHashMap* KnownOIDs::name2enum = nullptr;

$KnownOIDsArray* KnownOIDs::$values() {
	$init(KnownOIDs);
	return $new($KnownOIDsArray, {
		KnownOIDs::CommonName,
		KnownOIDs::Surname,
		KnownOIDs::SerialNumber,
		KnownOIDs::CountryName,
		KnownOIDs::LocalityName,
		KnownOIDs::StateName,
		KnownOIDs::StreetAddress,
		KnownOIDs::OrgName,
		KnownOIDs::OrgUnitName,
		KnownOIDs::Title,
		KnownOIDs::GivenName,
		KnownOIDs::Initials,
		KnownOIDs::GenerationQualifier,
		KnownOIDs::DNQualifier,
		KnownOIDs::SubjectDirectoryAttributes,
		KnownOIDs::SubjectKeyID,
		KnownOIDs::KeyUsage,
		KnownOIDs::PrivateKeyUsage,
		KnownOIDs::SubjectAlternativeName,
		KnownOIDs::IssuerAlternativeName,
		KnownOIDs::BasicConstraints,
		KnownOIDs::CRLNumber,
		KnownOIDs::ReasonCode,
		KnownOIDs::HoldInstructionCode,
		KnownOIDs::InvalidityDate,
		KnownOIDs::DeltaCRLIndicator,
		KnownOIDs::IssuingDistributionPoint,
		KnownOIDs::CertificateIssuer,
		KnownOIDs::NameConstraints,
		KnownOIDs::CRLDistributionPoints,
		KnownOIDs::CertificatePolicies,
		KnownOIDs::CE_CERT_POLICIES_ANY,
		KnownOIDs::PolicyMappings,
		KnownOIDs::AuthorityKeyID,
		KnownOIDs::PolicyConstraints,
		KnownOIDs::extendedKeyUsage,
		KnownOIDs::anyExtendedKeyUsage,
		KnownOIDs::FreshestCRL,
		KnownOIDs::InhibitAnyPolicy,
		KnownOIDs::AuthInfoAccess,
		KnownOIDs::SubjectInfoAccess,
		KnownOIDs::serverAuth,
		KnownOIDs::clientAuth,
		KnownOIDs::codeSigning,
		KnownOIDs::emailProtection,
		KnownOIDs::ipsecEndSystem,
		KnownOIDs::ipsecTunnel,
		KnownOIDs::ipsecUser,
		KnownOIDs::KP_TimeStamping,
		KnownOIDs::OCSPSigning,
		KnownOIDs::OCSP,
		KnownOIDs::OCSPBasicResponse,
		KnownOIDs::OCSPNonceExt,
		KnownOIDs::OCSPNoCheck,
		KnownOIDs::caIssuers,
		KnownOIDs::AD_TimeStamping,
		KnownOIDs::caRepository,
		KnownOIDs::AES,
		KnownOIDs::AES_128$ECB$NoPadding,
		KnownOIDs::AES_128$CBC$NoPadding,
		KnownOIDs::AES_128$OFB$NoPadding,
		KnownOIDs::AES_128$CFB$NoPadding,
		KnownOIDs::AES_128$KW$NoPadding,
		KnownOIDs::AES_128$GCM$NoPadding,
		KnownOIDs::AES_128$KWP$NoPadding,
		KnownOIDs::AES_192$ECB$NoPadding,
		KnownOIDs::AES_192$CBC$NoPadding,
		KnownOIDs::AES_192$OFB$NoPadding,
		KnownOIDs::AES_192$CFB$NoPadding,
		KnownOIDs::AES_192$KW$NoPadding,
		KnownOIDs::AES_192$GCM$NoPadding,
		KnownOIDs::AES_192$KWP$NoPadding,
		KnownOIDs::AES_256$ECB$NoPadding,
		KnownOIDs::AES_256$CBC$NoPadding,
		KnownOIDs::AES_256$OFB$NoPadding,
		KnownOIDs::AES_256$CFB$NoPadding,
		KnownOIDs::AES_256$KW$NoPadding,
		KnownOIDs::AES_256$GCM$NoPadding,
		KnownOIDs::AES_256$KWP$NoPadding,
		KnownOIDs::SHA_256,
		KnownOIDs::SHA_384,
		KnownOIDs::SHA_512,
		KnownOIDs::SHA_224,
		KnownOIDs::SHA_512$224,
		KnownOIDs::SHA_512$256,
		KnownOIDs::SHA3_224,
		KnownOIDs::SHA3_256,
		KnownOIDs::SHA3_384,
		KnownOIDs::SHA3_512,
		KnownOIDs::SHAKE128,
		KnownOIDs::SHAKE256,
		KnownOIDs::HmacSHA3_224,
		KnownOIDs::HmacSHA3_256,
		KnownOIDs::HmacSHA3_384,
		KnownOIDs::HmacSHA3_512,
		KnownOIDs::SHAKE128_LEN,
		KnownOIDs::SHAKE256_LEN,
		KnownOIDs::SHA224withDSA,
		KnownOIDs::SHA256withDSA,
		KnownOIDs::SHA384withDSA,
		KnownOIDs::SHA512withDSA,
		KnownOIDs::SHA3_224withDSA,
		KnownOIDs::SHA3_256withDSA,
		KnownOIDs::SHA3_384withDSA,
		KnownOIDs::SHA3_512withDSA,
		KnownOIDs::SHA3_224withECDSA,
		KnownOIDs::SHA3_256withECDSA,
		KnownOIDs::SHA3_384withECDSA,
		KnownOIDs::SHA3_512withECDSA,
		KnownOIDs::SHA3_224withRSA,
		KnownOIDs::SHA3_256withRSA,
		KnownOIDs::SHA3_384withRSA,
		KnownOIDs::SHA3_512withRSA,
		KnownOIDs::PKCS1,
		KnownOIDs::RSA,
		KnownOIDs::MD2withRSA,
		KnownOIDs::MD5withRSA,
		KnownOIDs::SHA1withRSA,
		KnownOIDs::OAEP,
		KnownOIDs::MGF1,
		KnownOIDs::PSpecified,
		KnownOIDs::RSASSA_PSS,
		KnownOIDs::SHA256withRSA,
		KnownOIDs::SHA384withRSA,
		KnownOIDs::SHA512withRSA,
		KnownOIDs::SHA224withRSA,
		KnownOIDs::SHA512$224withRSA,
		KnownOIDs::SHA512$256withRSA,
		KnownOIDs::DiffieHellman,
		KnownOIDs::PBEWithMD5AndDES,
		KnownOIDs::PBEWithMD5AndRC2,
		KnownOIDs::PBEWithSHA1AndDES,
		KnownOIDs::PBEWithSHA1AndRC2,
		KnownOIDs::PBKDF2WithHmacSHA1,
		KnownOIDs::PBES2,
		KnownOIDs::PKCS7,
		KnownOIDs::Data,
		KnownOIDs::SignedData,
		KnownOIDs::JDK_OLD_Data,
		KnownOIDs::JDK_OLD_SignedData,
		KnownOIDs::EnvelopedData,
		KnownOIDs::SignedAndEnvelopedData,
		KnownOIDs::DigestedData,
		KnownOIDs::EncryptedData,
		KnownOIDs::EmailAddress,
		KnownOIDs::UnstructuredName,
		KnownOIDs::ContentType,
		KnownOIDs::MessageDigest,
		KnownOIDs::SigningTime,
		KnownOIDs::CounterSignature,
		KnownOIDs::ChallengePassword,
		KnownOIDs::UnstructuredAddress,
		KnownOIDs::ExtendedCertificateAttributes,
		KnownOIDs::IssuerAndSerialNumber,
		KnownOIDs::ExtensionRequest,
		KnownOIDs::SMIMECapability,
		KnownOIDs::TimeStampTokenInfo,
		KnownOIDs::SigningCertificate,
		KnownOIDs::SignatureTimestampToken,
		KnownOIDs::CHACHA20_POLY1305,
		KnownOIDs::FriendlyName,
		KnownOIDs::LocalKeyID,
		KnownOIDs::CertTypeX509,
		KnownOIDs::CMSAlgorithmProtection,
		KnownOIDs::PBEWithSHA1AndRC4_128,
		KnownOIDs::PBEWithSHA1AndRC4_40,
		KnownOIDs::PBEWithSHA1AndDESede,
		KnownOIDs::PBEWithSHA1AndRC2_128,
		KnownOIDs::PBEWithSHA1AndRC2_40,
		KnownOIDs::PKCS8ShroudedKeyBag,
		KnownOIDs::CertBag,
		KnownOIDs::SecretBag,
		KnownOIDs::MD2,
		KnownOIDs::MD5,
		KnownOIDs::HmacSHA1,
		KnownOIDs::HmacSHA224,
		KnownOIDs::HmacSHA256,
		KnownOIDs::HmacSHA384,
		KnownOIDs::HmacSHA512,
		KnownOIDs::HmacSHA512$224,
		KnownOIDs::HmacSHA512$256,
		KnownOIDs::RC2$CBC$PKCS5Padding,
		KnownOIDs::ARCFOUR,
		KnownOIDs::DESede$CBC$NoPadding,
		KnownOIDs::RC5$CBC$PKCS5Padding,
		KnownOIDs::DSA,
		KnownOIDs::SHA1withDSA,
		KnownOIDs::EC,
		KnownOIDs::c2tnb191v1,
		KnownOIDs::c2tnb191v2,
		KnownOIDs::c2tnb191v3,
		KnownOIDs::c2tnb239v1,
		KnownOIDs::c2tnb239v2,
		KnownOIDs::c2tnb239v3,
		KnownOIDs::c2tnb359v1,
		KnownOIDs::c2tnb431r1,
		KnownOIDs::secp192r1,
		KnownOIDs::prime192v2,
		KnownOIDs::prime192v3,
		KnownOIDs::prime239v1,
		KnownOIDs::prime239v2,
		KnownOIDs::prime239v3,
		KnownOIDs::secp256r1,
		KnownOIDs::SHA1withECDSA,
		KnownOIDs::SHA224withECDSA,
		KnownOIDs::SHA256withECDSA,
		KnownOIDs::SHA384withECDSA,
		KnownOIDs::SHA512withECDSA,
		KnownOIDs::SpecifiedSHA2withECDSA,
		KnownOIDs::X942_DH,
		KnownOIDs::brainpoolP160r1,
		KnownOIDs::brainpoolP192r1,
		KnownOIDs::brainpoolP224r1,
		KnownOIDs::brainpoolP256r1,
		KnownOIDs::brainpoolP320r1,
		KnownOIDs::brainpoolP384r1,
		KnownOIDs::brainpoolP512r1,
		KnownOIDs::sect163k1,
		KnownOIDs::sect163r1,
		KnownOIDs::sect239k1,
		KnownOIDs::sect113r1,
		KnownOIDs::sect113r2,
		KnownOIDs::secp112r1,
		KnownOIDs::secp112r2,
		KnownOIDs::secp160r1,
		KnownOIDs::secp160k1,
		KnownOIDs::secp256k1,
		KnownOIDs::sect163r2,
		KnownOIDs::sect283k1,
		KnownOIDs::sect283r1,
		KnownOIDs::sect131r1,
		KnownOIDs::sect131r2,
		KnownOIDs::sect193r1,
		KnownOIDs::sect193r2,
		KnownOIDs::sect233k1,
		KnownOIDs::sect233r1,
		KnownOIDs::secp128r1,
		KnownOIDs::secp128r2,
		KnownOIDs::secp160r2,
		KnownOIDs::secp192k1,
		KnownOIDs::secp224k1,
		KnownOIDs::secp224r1,
		KnownOIDs::secp384r1,
		KnownOIDs::secp521r1,
		KnownOIDs::sect409k1,
		KnownOIDs::sect409r1,
		KnownOIDs::sect571k1,
		KnownOIDs::sect571r1,
		KnownOIDs::ECDH,
		KnownOIDs::OIW_DES_CBC,
		KnownOIDs::OIW_DSA,
		KnownOIDs::OIW_JDK_SHA1withDSA,
		KnownOIDs::OIW_SHA1withRSA_Odd,
		KnownOIDs::DESede,
		KnownOIDs::SHA_1,
		KnownOIDs::OIW_SHA1withDSA,
		KnownOIDs::OIW_SHA1withRSA,
		KnownOIDs::X25519,
		KnownOIDs::X448,
		KnownOIDs::Ed25519,
		KnownOIDs::Ed448,
		KnownOIDs::UCL_UserID,
		KnownOIDs::UCL_DomainComponent,
		KnownOIDs::NETSCAPE_CertType,
		KnownOIDs::NETSCAPE_CertSequence,
		KnownOIDs::NETSCAPE_ExportApproved,
		KnownOIDs::ORACLE_TrustedKeyUsage,
		KnownOIDs::ITUX509_RSA,
		KnownOIDs::SkipIPAddress,
		KnownOIDs::JAVASOFT_JDKKeyProtector,
		KnownOIDs::JAVASOFT_JCEKeyProtector,
		KnownOIDs::MICROSOFT_ExportApproved
	});
}

$KnownOIDsArray* KnownOIDs::values() {
	$init(KnownOIDs);
	return $cast($KnownOIDsArray, KnownOIDs::$VALUES->clone());
}

KnownOIDs* KnownOIDs::valueOf($String* name) {
	$init(KnownOIDs);
	return $cast(KnownOIDs, $Enum::valueOf(KnownOIDs::class$, name));
}

KnownOIDs* KnownOIDs::findMatch($String* s$renamed) {
	$init(KnownOIDs);
	$useLocalObjectStack();
	$var($String, s, s$renamed);
	$init($Locale);
	$assign(s, $nc(s)->toUpperCase($Locale::ENGLISH));
	KnownOIDs* res = $cast(KnownOIDs, KnownOIDs::name2enum->get(s));
	if (res == nullptr && KnownOIDs::debug != nullptr) {
		KnownOIDs::debug->println($$str({"No KnownOIDs enum found for "_s, s}));
	}
	return res;
}

void KnownOIDs::register$(KnownOIDs* o) {
	$init(KnownOIDs);
	$useLocalObjectStack();
	KnownOIDs* ov = $cast(KnownOIDs, KnownOIDs::name2enum->put($nc(o)->oid, o));
	if (ov != nullptr) {
		$throwNew($RuntimeException, $$str({"ERROR: Duplicate "_s, o->oid, " between "_s, o, " and "_s, ov}));
	} else if (KnownOIDs::debug != nullptr) {
		KnownOIDs::debug->println($$str({o->oid, " => "_s, $(o->name())}));
	}
	if (o->registerNames()) {
		$init($Locale);
		$var($String, stdNameUpper, $nc(o->stdName$)->toUpperCase($Locale::ENGLISH));
		if ($Objects::nonNull($(KnownOIDs::name2enum->put(stdNameUpper, o)))) {
			$throwNew($RuntimeException, $$str({"ERROR: Duplicate "_s, stdNameUpper, " exists already"_s}));
		}
		if (KnownOIDs::debug != nullptr) {
			KnownOIDs::debug->println($$str({stdNameUpper, " => "_s, $(o->name())}));
		}
		{
			$var($StringArray, arr$, o->aliases$);
			for (int32_t len$ = $nc(arr$)->length, i$ = 0; i$ < len$; ++i$) {
				$var($String, a, arr$->get(i$));
				{
					$var($String, aliasUpper, $nc(a)->toUpperCase($Locale::ENGLISH));
					if ($Objects::nonNull($(KnownOIDs::name2enum->put(aliasUpper, o)))) {
						$throwNew($RuntimeException, $$str({"ERROR: Duplicate "_s, aliasUpper, " exists already"_s}));
					}
					if (KnownOIDs::debug != nullptr) {
						KnownOIDs::debug->println($$str({aliasUpper, " => "_s, $(o->name())}));
					}
				}
			}
		}
	}
}

void KnownOIDs::init$($String* $enum$name, int32_t $enum$ordinal, $String* oid) {
	$Enum::init$($enum$name, $enum$ordinal);
	$set(this, oid, oid);
	$set(this, stdName$, name());
	$set(this, aliases$, $new($StringArray, 0));
}

void KnownOIDs::init$($String* $enum$name, int32_t $enum$ordinal, $String* oid, $String* stdName, $StringArray* aliases) {
	$Enum::init$($enum$name, $enum$ordinal);
	$set(this, oid, oid);
	$set(this, stdName$, stdName);
	$set(this, aliases$, aliases);
}

$String* KnownOIDs::value() {
	return this->oid;
}

$String* KnownOIDs::stdName() {
	return this->stdName$;
}

$StringArray* KnownOIDs::aliases() {
	return this->aliases$;
}

bool KnownOIDs::registerNames() {
	return true;
}

void KnownOIDs::clinit$($Class* clazz) {
	$useLocalObjectStack();
	$assignStatic(KnownOIDs::CommonName, $new(KnownOIDs, "CommonName"_s, 0, "2.5.4.3"_s));
	$assignStatic(KnownOIDs::Surname, $new(KnownOIDs, "Surname"_s, 1, "2.5.4.4"_s));
	$assignStatic(KnownOIDs::SerialNumber, $new(KnownOIDs, "SerialNumber"_s, 2, "2.5.4.5"_s));
	$assignStatic(KnownOIDs::CountryName, $new(KnownOIDs, "CountryName"_s, 3, "2.5.4.6"_s));
	$assignStatic(KnownOIDs::LocalityName, $new(KnownOIDs, "LocalityName"_s, 4, "2.5.4.7"_s));
	$assignStatic(KnownOIDs::StateName, $new(KnownOIDs, "StateName"_s, 5, "2.5.4.8"_s));
	$assignStatic(KnownOIDs::StreetAddress, $new(KnownOIDs, "StreetAddress"_s, 6, "2.5.4.9"_s));
	$assignStatic(KnownOIDs::OrgName, $new(KnownOIDs, "OrgName"_s, 7, "2.5.4.10"_s));
	$assignStatic(KnownOIDs::OrgUnitName, $new(KnownOIDs, "OrgUnitName"_s, 8, "2.5.4.11"_s));
	$assignStatic(KnownOIDs::Title, $new(KnownOIDs, "Title"_s, 9, "2.5.4.12"_s));
	$assignStatic(KnownOIDs::GivenName, $new(KnownOIDs, "GivenName"_s, 10, "2.5.4.42"_s));
	$assignStatic(KnownOIDs::Initials, $new(KnownOIDs, "Initials"_s, 11, "2.5.4.43"_s));
	$assignStatic(KnownOIDs::GenerationQualifier, $new(KnownOIDs, "GenerationQualifier"_s, 12, "2.5.4.44"_s));
	$assignStatic(KnownOIDs::DNQualifier, $new(KnownOIDs, "DNQualifier"_s, 13, "2.5.4.46"_s));
	$assignStatic(KnownOIDs::SubjectDirectoryAttributes, $new(KnownOIDs, "SubjectDirectoryAttributes"_s, 14, "2.5.29.9"_s));
	$assignStatic(KnownOIDs::SubjectKeyID, $new(KnownOIDs, "SubjectKeyID"_s, 15, "2.5.29.14"_s));
	$assignStatic(KnownOIDs::KeyUsage, $new(KnownOIDs, "KeyUsage"_s, 16, "2.5.29.15"_s));
	$assignStatic(KnownOIDs::PrivateKeyUsage, $new(KnownOIDs, "PrivateKeyUsage"_s, 17, "2.5.29.16"_s));
	$assignStatic(KnownOIDs::SubjectAlternativeName, $new(KnownOIDs, "SubjectAlternativeName"_s, 18, "2.5.29.17"_s));
	$assignStatic(KnownOIDs::IssuerAlternativeName, $new(KnownOIDs, "IssuerAlternativeName"_s, 19, "2.5.29.18"_s));
	$assignStatic(KnownOIDs::BasicConstraints, $new(KnownOIDs, "BasicConstraints"_s, 20, "2.5.29.19"_s));
	$assignStatic(KnownOIDs::CRLNumber, $new(KnownOIDs, "CRLNumber"_s, 21, "2.5.29.20"_s));
	$assignStatic(KnownOIDs::ReasonCode, $new(KnownOIDs, "ReasonCode"_s, 22, "2.5.29.21"_s));
	$assignStatic(KnownOIDs::HoldInstructionCode, $new(KnownOIDs, "HoldInstructionCode"_s, 23, "2.5.29.23"_s));
	$assignStatic(KnownOIDs::InvalidityDate, $new(KnownOIDs, "InvalidityDate"_s, 24, "2.5.29.24"_s));
	$assignStatic(KnownOIDs::DeltaCRLIndicator, $new(KnownOIDs, "DeltaCRLIndicator"_s, 25, "2.5.29.27"_s));
	$assignStatic(KnownOIDs::IssuingDistributionPoint, $new(KnownOIDs, "IssuingDistributionPoint"_s, 26, "2.5.29.28"_s));
	$assignStatic(KnownOIDs::CertificateIssuer, $new(KnownOIDs, "CertificateIssuer"_s, 27, "2.5.29.29"_s));
	$assignStatic(KnownOIDs::NameConstraints, $new(KnownOIDs, "NameConstraints"_s, 28, "2.5.29.30"_s));
	$assignStatic(KnownOIDs::CRLDistributionPoints, $new(KnownOIDs, "CRLDistributionPoints"_s, 29, "2.5.29.31"_s));
	$assignStatic(KnownOIDs::CertificatePolicies, $new(KnownOIDs, "CertificatePolicies"_s, 30, "2.5.29.32"_s));
	$assignStatic(KnownOIDs::CE_CERT_POLICIES_ANY, $new(KnownOIDs, "CE_CERT_POLICIES_ANY"_s, 31, "2.5.29.32.0"_s));
	$assignStatic(KnownOIDs::PolicyMappings, $new(KnownOIDs, "PolicyMappings"_s, 32, "2.5.29.33"_s));
	$assignStatic(KnownOIDs::AuthorityKeyID, $new(KnownOIDs, "AuthorityKeyID"_s, 33, "2.5.29.35"_s));
	$assignStatic(KnownOIDs::PolicyConstraints, $new(KnownOIDs, "PolicyConstraints"_s, 34, "2.5.29.36"_s));
	$assignStatic(KnownOIDs::extendedKeyUsage, $new(KnownOIDs, "extendedKeyUsage"_s, 35, "2.5.29.37"_s));
	$assignStatic(KnownOIDs::anyExtendedKeyUsage, $new(KnownOIDs, "anyExtendedKeyUsage"_s, 36, "2.5.29.37.0"_s));
	$assignStatic(KnownOIDs::FreshestCRL, $new(KnownOIDs, "FreshestCRL"_s, 37, "2.5.29.46"_s));
	$assignStatic(KnownOIDs::InhibitAnyPolicy, $new(KnownOIDs, "InhibitAnyPolicy"_s, 38, "2.5.29.54"_s));
	$assignStatic(KnownOIDs::AuthInfoAccess, $new(KnownOIDs, "AuthInfoAccess"_s, 39, "1.3.6.1.5.5.7.1.1"_s));
	$assignStatic(KnownOIDs::SubjectInfoAccess, $new(KnownOIDs, "SubjectInfoAccess"_s, 40, "1.3.6.1.5.5.7.1.11"_s));
	$assignStatic(KnownOIDs::serverAuth, $new(KnownOIDs, "serverAuth"_s, 41, "1.3.6.1.5.5.7.3.1"_s));
	$assignStatic(KnownOIDs::clientAuth, $new(KnownOIDs, "clientAuth"_s, 42, "1.3.6.1.5.5.7.3.2"_s));
	$assignStatic(KnownOIDs::codeSigning, $new(KnownOIDs, "codeSigning"_s, 43, "1.3.6.1.5.5.7.3.3"_s));
	$assignStatic(KnownOIDs::emailProtection, $new(KnownOIDs, "emailProtection"_s, 44, "1.3.6.1.5.5.7.3.4"_s));
	$assignStatic(KnownOIDs::ipsecEndSystem, $new(KnownOIDs, "ipsecEndSystem"_s, 45, "1.3.6.1.5.5.7.3.5"_s));
	$assignStatic(KnownOIDs::ipsecTunnel, $new(KnownOIDs, "ipsecTunnel"_s, 46, "1.3.6.1.5.5.7.3.6"_s));
	$assignStatic(KnownOIDs::ipsecUser, $new(KnownOIDs, "ipsecUser"_s, 47, "1.3.6.1.5.5.7.3.7"_s));
	$assignStatic(KnownOIDs::KP_TimeStamping, $new($KnownOIDs$1, "KP_TimeStamping"_s, 48, "1.3.6.1.5.5.7.3.8"_s, "timeStamping"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::OCSPSigning, $new(KnownOIDs, "OCSPSigning"_s, 49, "1.3.6.1.5.5.7.3.9"_s));
	$assignStatic(KnownOIDs::OCSP, $new(KnownOIDs, "OCSP"_s, 50, "1.3.6.1.5.5.7.48.1"_s));
	$assignStatic(KnownOIDs::OCSPBasicResponse, $new(KnownOIDs, "OCSPBasicResponse"_s, 51, "1.3.6.1.5.5.7.48.1.1"_s));
	$assignStatic(KnownOIDs::OCSPNonceExt, $new(KnownOIDs, "OCSPNonceExt"_s, 52, "1.3.6.1.5.5.7.48.1.2"_s));
	$assignStatic(KnownOIDs::OCSPNoCheck, $new(KnownOIDs, "OCSPNoCheck"_s, 53, "1.3.6.1.5.5.7.48.1.5"_s));
	$assignStatic(KnownOIDs::caIssuers, $new(KnownOIDs, "caIssuers"_s, 54, "1.3.6.1.5.5.7.48.2"_s));
	$assignStatic(KnownOIDs::AD_TimeStamping, $new($KnownOIDs$2, "AD_TimeStamping"_s, 55, "1.3.6.1.5.5.7.48.3"_s, "timeStamping"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::caRepository, $new(KnownOIDs, "caRepository"_s, 56, "1.3.6.1.5.5.7.48.5"_s, "caRepository"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES, $new(KnownOIDs, "AES"_s, 57, "2.16.840.1.101.3.4.1"_s));
	$assignStatic(KnownOIDs::AES_128$ECB$NoPadding, $new(KnownOIDs, "AES_128$ECB$NoPadding"_s, 58, "2.16.840.1.101.3.4.1.1"_s, "AES_128/ECB/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_128$CBC$NoPadding, $new(KnownOIDs, "AES_128$CBC$NoPadding"_s, 59, "2.16.840.1.101.3.4.1.2"_s, "AES_128/CBC/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_128$OFB$NoPadding, $new(KnownOIDs, "AES_128$OFB$NoPadding"_s, 60, "2.16.840.1.101.3.4.1.3"_s, "AES_128/OFB/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_128$CFB$NoPadding, $new(KnownOIDs, "AES_128$CFB$NoPadding"_s, 61, "2.16.840.1.101.3.4.1.4"_s, "AES_128/CFB/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_128$KW$NoPadding, $new(KnownOIDs, "AES_128$KW$NoPadding"_s, 62, "2.16.840.1.101.3.4.1.5"_s, "AES_128/KW/NoPadding"_s, $$new($StringArray, {"AESWrap_128"_s})));
	$assignStatic(KnownOIDs::AES_128$GCM$NoPadding, $new(KnownOIDs, "AES_128$GCM$NoPadding"_s, 63, "2.16.840.1.101.3.4.1.6"_s, "AES_128/GCM/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_128$KWP$NoPadding, $new(KnownOIDs, "AES_128$KWP$NoPadding"_s, 64, "2.16.840.1.101.3.4.1.8"_s, "AES_128/KWP/NoPadding"_s, $$new($StringArray, {"AESWrapPad_128"_s})));
	$assignStatic(KnownOIDs::AES_192$ECB$NoPadding, $new(KnownOIDs, "AES_192$ECB$NoPadding"_s, 65, "2.16.840.1.101.3.4.1.21"_s, "AES_192/ECB/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_192$CBC$NoPadding, $new(KnownOIDs, "AES_192$CBC$NoPadding"_s, 66, "2.16.840.1.101.3.4.1.22"_s, "AES_192/CBC/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_192$OFB$NoPadding, $new(KnownOIDs, "AES_192$OFB$NoPadding"_s, 67, "2.16.840.1.101.3.4.1.23"_s, "AES_192/OFB/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_192$CFB$NoPadding, $new(KnownOIDs, "AES_192$CFB$NoPadding"_s, 68, "2.16.840.1.101.3.4.1.24"_s, "AES_192/CFB/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_192$KW$NoPadding, $new(KnownOIDs, "AES_192$KW$NoPadding"_s, 69, "2.16.840.1.101.3.4.1.25"_s, "AES_192/KW/NoPadding"_s, $$new($StringArray, {"AESWrap_192"_s})));
	$assignStatic(KnownOIDs::AES_192$GCM$NoPadding, $new(KnownOIDs, "AES_192$GCM$NoPadding"_s, 70, "2.16.840.1.101.3.4.1.26"_s, "AES_192/GCM/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_192$KWP$NoPadding, $new(KnownOIDs, "AES_192$KWP$NoPadding"_s, 71, "2.16.840.1.101.3.4.1.28"_s, "AES_192/KWP/NoPadding"_s, $$new($StringArray, {"AESWrapPad_192"_s})));
	$assignStatic(KnownOIDs::AES_256$ECB$NoPadding, $new(KnownOIDs, "AES_256$ECB$NoPadding"_s, 72, "2.16.840.1.101.3.4.1.41"_s, "AES_256/ECB/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_256$CBC$NoPadding, $new(KnownOIDs, "AES_256$CBC$NoPadding"_s, 73, "2.16.840.1.101.3.4.1.42"_s, "AES_256/CBC/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_256$OFB$NoPadding, $new(KnownOIDs, "AES_256$OFB$NoPadding"_s, 74, "2.16.840.1.101.3.4.1.43"_s, "AES_256/OFB/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_256$CFB$NoPadding, $new(KnownOIDs, "AES_256$CFB$NoPadding"_s, 75, "2.16.840.1.101.3.4.1.44"_s, "AES_256/CFB/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_256$KW$NoPadding, $new(KnownOIDs, "AES_256$KW$NoPadding"_s, 76, "2.16.840.1.101.3.4.1.45"_s, "AES_256/KW/NoPadding"_s, $$new($StringArray, {"AESWrap_256"_s})));
	$assignStatic(KnownOIDs::AES_256$GCM$NoPadding, $new(KnownOIDs, "AES_256$GCM$NoPadding"_s, 77, "2.16.840.1.101.3.4.1.46"_s, "AES_256/GCM/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::AES_256$KWP$NoPadding, $new(KnownOIDs, "AES_256$KWP$NoPadding"_s, 78, "2.16.840.1.101.3.4.1.48"_s, "AES_256/KWP/NoPadding"_s, $$new($StringArray, {"AESWrapPad_256"_s})));
	$assignStatic(KnownOIDs::SHA_256, $new(KnownOIDs, "SHA_256"_s, 79, "2.16.840.1.101.3.4.2.1"_s, "SHA-256"_s, $$new($StringArray, {"SHA256"_s})));
	$assignStatic(KnownOIDs::SHA_384, $new(KnownOIDs, "SHA_384"_s, 80, "2.16.840.1.101.3.4.2.2"_s, "SHA-384"_s, $$new($StringArray, {"SHA384"_s})));
	$assignStatic(KnownOIDs::SHA_512, $new(KnownOIDs, "SHA_512"_s, 81, "2.16.840.1.101.3.4.2.3"_s, "SHA-512"_s, $$new($StringArray, {"SHA512"_s})));
	$assignStatic(KnownOIDs::SHA_224, $new(KnownOIDs, "SHA_224"_s, 82, "2.16.840.1.101.3.4.2.4"_s, "SHA-224"_s, $$new($StringArray, {"SHA224"_s})));
	$assignStatic(KnownOIDs::SHA_512$224, $new(KnownOIDs, "SHA_512$224"_s, 83, "2.16.840.1.101.3.4.2.5"_s, "SHA-512/224"_s, $$new($StringArray, {"SHA512/224"_s})));
	$assignStatic(KnownOIDs::SHA_512$256, $new(KnownOIDs, "SHA_512$256"_s, 84, "2.16.840.1.101.3.4.2.6"_s, "SHA-512/256"_s, $$new($StringArray, {"SHA512/256"_s})));
	$assignStatic(KnownOIDs::SHA3_224, $new(KnownOIDs, "SHA3_224"_s, 85, "2.16.840.1.101.3.4.2.7"_s, "SHA3-224"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_256, $new(KnownOIDs, "SHA3_256"_s, 86, "2.16.840.1.101.3.4.2.8"_s, "SHA3-256"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_384, $new(KnownOIDs, "SHA3_384"_s, 87, "2.16.840.1.101.3.4.2.9"_s, "SHA3-384"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_512, $new(KnownOIDs, "SHA3_512"_s, 88, "2.16.840.1.101.3.4.2.10"_s, "SHA3-512"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHAKE128, $new(KnownOIDs, "SHAKE128"_s, 89, "2.16.840.1.101.3.4.2.11"_s));
	$assignStatic(KnownOIDs::SHAKE256, $new(KnownOIDs, "SHAKE256"_s, 90, "2.16.840.1.101.3.4.2.12"_s));
	$assignStatic(KnownOIDs::HmacSHA3_224, $new(KnownOIDs, "HmacSHA3_224"_s, 91, "2.16.840.1.101.3.4.2.13"_s, "HmacSHA3-224"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::HmacSHA3_256, $new(KnownOIDs, "HmacSHA3_256"_s, 92, "2.16.840.1.101.3.4.2.14"_s, "HmacSHA3-256"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::HmacSHA3_384, $new(KnownOIDs, "HmacSHA3_384"_s, 93, "2.16.840.1.101.3.4.2.15"_s, "HmacSHA3-384"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::HmacSHA3_512, $new(KnownOIDs, "HmacSHA3_512"_s, 94, "2.16.840.1.101.3.4.2.16"_s, "HmacSHA3-512"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHAKE128_LEN, $new(KnownOIDs, "SHAKE128_LEN"_s, 95, "2.16.840.1.101.3.4.2.17"_s, "SHAKE128-LEN"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHAKE256_LEN, $new(KnownOIDs, "SHAKE256_LEN"_s, 96, "2.16.840.1.101.3.4.2.18"_s, "SHAKE256-LEN"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA224withDSA, $new(KnownOIDs, "SHA224withDSA"_s, 97, "2.16.840.1.101.3.4.3.1"_s));
	$assignStatic(KnownOIDs::SHA256withDSA, $new(KnownOIDs, "SHA256withDSA"_s, 98, "2.16.840.1.101.3.4.3.2"_s));
	$assignStatic(KnownOIDs::SHA384withDSA, $new(KnownOIDs, "SHA384withDSA"_s, 99, "2.16.840.1.101.3.4.3.3"_s));
	$assignStatic(KnownOIDs::SHA512withDSA, $new(KnownOIDs, "SHA512withDSA"_s, 100, "2.16.840.1.101.3.4.3.4"_s));
	$assignStatic(KnownOIDs::SHA3_224withDSA, $new(KnownOIDs, "SHA3_224withDSA"_s, 101, "2.16.840.1.101.3.4.3.5"_s, "SHA3-224withDSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_256withDSA, $new(KnownOIDs, "SHA3_256withDSA"_s, 102, "2.16.840.1.101.3.4.3.6"_s, "SHA3-256withDSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_384withDSA, $new(KnownOIDs, "SHA3_384withDSA"_s, 103, "2.16.840.1.101.3.4.3.7"_s, "SHA3-384withDSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_512withDSA, $new(KnownOIDs, "SHA3_512withDSA"_s, 104, "2.16.840.1.101.3.4.3.8"_s, "SHA3-512withDSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_224withECDSA, $new(KnownOIDs, "SHA3_224withECDSA"_s, 105, "2.16.840.1.101.3.4.3.9"_s, "SHA3-224withECDSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_256withECDSA, $new(KnownOIDs, "SHA3_256withECDSA"_s, 106, "2.16.840.1.101.3.4.3.10"_s, "SHA3-256withECDSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_384withECDSA, $new(KnownOIDs, "SHA3_384withECDSA"_s, 107, "2.16.840.1.101.3.4.3.11"_s, "SHA3-384withECDSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_512withECDSA, $new(KnownOIDs, "SHA3_512withECDSA"_s, 108, "2.16.840.1.101.3.4.3.12"_s, "SHA3-512withECDSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_224withRSA, $new(KnownOIDs, "SHA3_224withRSA"_s, 109, "2.16.840.1.101.3.4.3.13"_s, "SHA3-224withRSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_256withRSA, $new(KnownOIDs, "SHA3_256withRSA"_s, 110, "2.16.840.1.101.3.4.3.14"_s, "SHA3-256withRSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_384withRSA, $new(KnownOIDs, "SHA3_384withRSA"_s, 111, "2.16.840.1.101.3.4.3.15"_s, "SHA3-384withRSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA3_512withRSA, $new(KnownOIDs, "SHA3_512withRSA"_s, 112, "2.16.840.1.101.3.4.3.16"_s, "SHA3-512withRSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::PKCS1, $new($KnownOIDs$3, "PKCS1"_s, 113, "1.2.840.113549.1.1"_s, "RSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::RSA, $new(KnownOIDs, "RSA"_s, 114, "1.2.840.113549.1.1.1"_s));
	$assignStatic(KnownOIDs::MD2withRSA, $new(KnownOIDs, "MD2withRSA"_s, 115, "1.2.840.113549.1.1.2"_s));
	$assignStatic(KnownOIDs::MD5withRSA, $new(KnownOIDs, "MD5withRSA"_s, 116, "1.2.840.113549.1.1.4"_s));
	$assignStatic(KnownOIDs::SHA1withRSA, $new(KnownOIDs, "SHA1withRSA"_s, 117, "1.2.840.113549.1.1.5"_s));
	$assignStatic(KnownOIDs::OAEP, $new(KnownOIDs, "OAEP"_s, 118, "1.2.840.113549.1.1.7"_s));
	$assignStatic(KnownOIDs::MGF1, $new(KnownOIDs, "MGF1"_s, 119, "1.2.840.113549.1.1.8"_s));
	$assignStatic(KnownOIDs::PSpecified, $new(KnownOIDs, "PSpecified"_s, 120, "1.2.840.113549.1.1.9"_s));
	$assignStatic(KnownOIDs::RSASSA_PSS, $new(KnownOIDs, "RSASSA_PSS"_s, 121, "1.2.840.113549.1.1.10"_s, "RSASSA-PSS"_s, $$new($StringArray, {"PSS"_s})));
	$assignStatic(KnownOIDs::SHA256withRSA, $new(KnownOIDs, "SHA256withRSA"_s, 122, "1.2.840.113549.1.1.11"_s));
	$assignStatic(KnownOIDs::SHA384withRSA, $new(KnownOIDs, "SHA384withRSA"_s, 123, "1.2.840.113549.1.1.12"_s));
	$assignStatic(KnownOIDs::SHA512withRSA, $new(KnownOIDs, "SHA512withRSA"_s, 124, "1.2.840.113549.1.1.13"_s));
	$assignStatic(KnownOIDs::SHA224withRSA, $new(KnownOIDs, "SHA224withRSA"_s, 125, "1.2.840.113549.1.1.14"_s));
	$assignStatic(KnownOIDs::SHA512$224withRSA, $new(KnownOIDs, "SHA512$224withRSA"_s, 126, "1.2.840.113549.1.1.15"_s, "SHA512/224withRSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA512$256withRSA, $new(KnownOIDs, "SHA512$256withRSA"_s, 127, "1.2.840.113549.1.1.16"_s, "SHA512/256withRSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::DiffieHellman, $new(KnownOIDs, "DiffieHellman"_s, 128, "1.2.840.113549.1.3.1"_s, "DiffieHellman"_s, $$new($StringArray, {"DH"_s})));
	$assignStatic(KnownOIDs::PBEWithMD5AndDES, $new(KnownOIDs, "PBEWithMD5AndDES"_s, 129, "1.2.840.113549.1.5.3"_s));
	$assignStatic(KnownOIDs::PBEWithMD5AndRC2, $new(KnownOIDs, "PBEWithMD5AndRC2"_s, 130, "1.2.840.113549.1.5.6"_s));
	$assignStatic(KnownOIDs::PBEWithSHA1AndDES, $new(KnownOIDs, "PBEWithSHA1AndDES"_s, 131, "1.2.840.113549.1.5.10"_s));
	$assignStatic(KnownOIDs::PBEWithSHA1AndRC2, $new(KnownOIDs, "PBEWithSHA1AndRC2"_s, 132, "1.2.840.113549.1.5.11"_s));
	$assignStatic(KnownOIDs::PBKDF2WithHmacSHA1, $new(KnownOIDs, "PBKDF2WithHmacSHA1"_s, 133, "1.2.840.113549.1.5.12"_s));
	$assignStatic(KnownOIDs::PBES2, $new(KnownOIDs, "PBES2"_s, 134, "1.2.840.113549.1.5.13"_s));
	$assignStatic(KnownOIDs::PKCS7, $new(KnownOIDs, "PKCS7"_s, 135, "1.2.840.113549.1.7"_s));
	$assignStatic(KnownOIDs::Data, $new(KnownOIDs, "Data"_s, 136, "1.2.840.113549.1.7.1"_s));
	$assignStatic(KnownOIDs::SignedData, $new(KnownOIDs, "SignedData"_s, 137, "1.2.840.113549.1.7.2"_s));
	$assignStatic(KnownOIDs::JDK_OLD_Data, $new(KnownOIDs, "JDK_OLD_Data"_s, 138, "1.2.840.1113549.1.7.1"_s));
	$assignStatic(KnownOIDs::JDK_OLD_SignedData, $new(KnownOIDs, "JDK_OLD_SignedData"_s, 139, "1.2.840.1113549.1.7.2"_s));
	$assignStatic(KnownOIDs::EnvelopedData, $new(KnownOIDs, "EnvelopedData"_s, 140, "1.2.840.113549.1.7.3"_s));
	$assignStatic(KnownOIDs::SignedAndEnvelopedData, $new(KnownOIDs, "SignedAndEnvelopedData"_s, 141, "1.2.840.113549.1.7.4"_s));
	$assignStatic(KnownOIDs::DigestedData, $new(KnownOIDs, "DigestedData"_s, 142, "1.2.840.113549.1.7.5"_s));
	$assignStatic(KnownOIDs::EncryptedData, $new(KnownOIDs, "EncryptedData"_s, 143, "1.2.840.113549.1.7.6"_s));
	$assignStatic(KnownOIDs::EmailAddress, $new(KnownOIDs, "EmailAddress"_s, 144, "1.2.840.113549.1.9.1"_s));
	$assignStatic(KnownOIDs::UnstructuredName, $new(KnownOIDs, "UnstructuredName"_s, 145, "1.2.840.113549.1.9.2"_s));
	$assignStatic(KnownOIDs::ContentType, $new(KnownOIDs, "ContentType"_s, 146, "1.2.840.113549.1.9.3"_s));
	$assignStatic(KnownOIDs::MessageDigest, $new(KnownOIDs, "MessageDigest"_s, 147, "1.2.840.113549.1.9.4"_s));
	$assignStatic(KnownOIDs::SigningTime, $new(KnownOIDs, "SigningTime"_s, 148, "1.2.840.113549.1.9.5"_s));
	$assignStatic(KnownOIDs::CounterSignature, $new(KnownOIDs, "CounterSignature"_s, 149, "1.2.840.113549.1.9.6"_s));
	$assignStatic(KnownOIDs::ChallengePassword, $new(KnownOIDs, "ChallengePassword"_s, 150, "1.2.840.113549.1.9.7"_s));
	$assignStatic(KnownOIDs::UnstructuredAddress, $new(KnownOIDs, "UnstructuredAddress"_s, 151, "1.2.840.113549.1.9.8"_s));
	$assignStatic(KnownOIDs::ExtendedCertificateAttributes, $new(KnownOIDs, "ExtendedCertificateAttributes"_s, 152, "1.2.840.113549.1.9.9"_s));
	$assignStatic(KnownOIDs::IssuerAndSerialNumber, $new(KnownOIDs, "IssuerAndSerialNumber"_s, 153, "1.2.840.113549.1.9.10"_s));
	$assignStatic(KnownOIDs::ExtensionRequest, $new(KnownOIDs, "ExtensionRequest"_s, 154, "1.2.840.113549.1.9.14"_s));
	$assignStatic(KnownOIDs::SMIMECapability, $new(KnownOIDs, "SMIMECapability"_s, 155, "1.2.840.113549.1.9.15"_s));
	$assignStatic(KnownOIDs::TimeStampTokenInfo, $new(KnownOIDs, "TimeStampTokenInfo"_s, 156, "1.2.840.113549.1.9.16.1.4"_s));
	$assignStatic(KnownOIDs::SigningCertificate, $new(KnownOIDs, "SigningCertificate"_s, 157, "1.2.840.113549.1.9.16.2.12"_s));
	$assignStatic(KnownOIDs::SignatureTimestampToken, $new(KnownOIDs, "SignatureTimestampToken"_s, 158, "1.2.840.113549.1.9.16.2.14"_s));
	$assignStatic(KnownOIDs::CHACHA20_POLY1305, $new(KnownOIDs, "CHACHA20_POLY1305"_s, 159, "1.2.840.113549.1.9.16.3.18"_s, "CHACHA20-POLY1305"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::FriendlyName, $new(KnownOIDs, "FriendlyName"_s, 160, "1.2.840.113549.1.9.20"_s));
	$assignStatic(KnownOIDs::LocalKeyID, $new(KnownOIDs, "LocalKeyID"_s, 161, "1.2.840.113549.1.9.21"_s));
	$assignStatic(KnownOIDs::CertTypeX509, $new(KnownOIDs, "CertTypeX509"_s, 162, "1.2.840.113549.1.9.22.1"_s));
	$assignStatic(KnownOIDs::CMSAlgorithmProtection, $new(KnownOIDs, "CMSAlgorithmProtection"_s, 163, "1.2.840.113549.1.9.52"_s));
	$assignStatic(KnownOIDs::PBEWithSHA1AndRC4_128, $new(KnownOIDs, "PBEWithSHA1AndRC4_128"_s, 164, "1.2.840.113549.1.12.1.1"_s));
	$assignStatic(KnownOIDs::PBEWithSHA1AndRC4_40, $new(KnownOIDs, "PBEWithSHA1AndRC4_40"_s, 165, "1.2.840.113549.1.12.1.2"_s));
	$assignStatic(KnownOIDs::PBEWithSHA1AndDESede, $new(KnownOIDs, "PBEWithSHA1AndDESede"_s, 166, "1.2.840.113549.1.12.1.3"_s));
	$assignStatic(KnownOIDs::PBEWithSHA1AndRC2_128, $new(KnownOIDs, "PBEWithSHA1AndRC2_128"_s, 167, "1.2.840.113549.1.12.1.5"_s));
	$assignStatic(KnownOIDs::PBEWithSHA1AndRC2_40, $new(KnownOIDs, "PBEWithSHA1AndRC2_40"_s, 168, "1.2.840.113549.1.12.1.6"_s));
	$assignStatic(KnownOIDs::PKCS8ShroudedKeyBag, $new(KnownOIDs, "PKCS8ShroudedKeyBag"_s, 169, "1.2.840.113549.1.12.10.1.2"_s));
	$assignStatic(KnownOIDs::CertBag, $new(KnownOIDs, "CertBag"_s, 170, "1.2.840.113549.1.12.10.1.3"_s));
	$assignStatic(KnownOIDs::SecretBag, $new(KnownOIDs, "SecretBag"_s, 171, "1.2.840.113549.1.12.10.1.5"_s));
	$assignStatic(KnownOIDs::MD2, $new(KnownOIDs, "MD2"_s, 172, "1.2.840.113549.2.2"_s));
	$assignStatic(KnownOIDs::MD5, $new(KnownOIDs, "MD5"_s, 173, "1.2.840.113549.2.5"_s));
	$assignStatic(KnownOIDs::HmacSHA1, $new(KnownOIDs, "HmacSHA1"_s, 174, "1.2.840.113549.2.7"_s));
	$assignStatic(KnownOIDs::HmacSHA224, $new(KnownOIDs, "HmacSHA224"_s, 175, "1.2.840.113549.2.8"_s));
	$assignStatic(KnownOIDs::HmacSHA256, $new(KnownOIDs, "HmacSHA256"_s, 176, "1.2.840.113549.2.9"_s));
	$assignStatic(KnownOIDs::HmacSHA384, $new(KnownOIDs, "HmacSHA384"_s, 177, "1.2.840.113549.2.10"_s));
	$assignStatic(KnownOIDs::HmacSHA512, $new(KnownOIDs, "HmacSHA512"_s, 178, "1.2.840.113549.2.11"_s));
	$assignStatic(KnownOIDs::HmacSHA512$224, $new(KnownOIDs, "HmacSHA512$224"_s, 179, "1.2.840.113549.2.12"_s, "HmacSHA512/224"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::HmacSHA512$256, $new(KnownOIDs, "HmacSHA512$256"_s, 180, "1.2.840.113549.2.13"_s, "HmacSHA512/256"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::RC2$CBC$PKCS5Padding, $new(KnownOIDs, "RC2$CBC$PKCS5Padding"_s, 181, "1.2.840.113549.3.2"_s, "RC2/CBC/PKCS5Padding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::ARCFOUR, $new(KnownOIDs, "ARCFOUR"_s, 182, "1.2.840.113549.3.4"_s, "ARCFOUR"_s, $$new($StringArray, {"RC4"_s})));
	$assignStatic(KnownOIDs::DESede$CBC$NoPadding, $new(KnownOIDs, "DESede$CBC$NoPadding"_s, 183, "1.2.840.113549.3.7"_s, "DESede/CBC/NoPadding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::RC5$CBC$PKCS5Padding, $new(KnownOIDs, "RC5$CBC$PKCS5Padding"_s, 184, "1.2.840.113549.3.9"_s, "RC5/CBC/PKCS5Padding"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::DSA, $new(KnownOIDs, "DSA"_s, 185, "1.2.840.10040.4.1"_s));
	$assignStatic(KnownOIDs::SHA1withDSA, $new(KnownOIDs, "SHA1withDSA"_s, 186, "1.2.840.10040.4.3"_s, "SHA1withDSA"_s, $$new($StringArray, {"DSS"_s})));
	$assignStatic(KnownOIDs::EC, $new(KnownOIDs, "EC"_s, 187, "1.2.840.10045.2.1"_s));
	$assignStatic(KnownOIDs::c2tnb191v1, $new(KnownOIDs, "c2tnb191v1"_s, 188, "1.2.840.10045.3.0.5"_s, "X9.62 c2tnb191v1"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::c2tnb191v2, $new(KnownOIDs, "c2tnb191v2"_s, 189, "1.2.840.10045.3.0.6"_s, "X9.62 c2tnb191v2"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::c2tnb191v3, $new(KnownOIDs, "c2tnb191v3"_s, 190, "1.2.840.10045.3.0.7"_s, "X9.62 c2tnb191v3"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::c2tnb239v1, $new(KnownOIDs, "c2tnb239v1"_s, 191, "1.2.840.10045.3.0.11"_s, "X9.62 c2tnb239v1"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::c2tnb239v2, $new(KnownOIDs, "c2tnb239v2"_s, 192, "1.2.840.10045.3.0.12"_s, "X9.62 c2tnb239v2"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::c2tnb239v3, $new(KnownOIDs, "c2tnb239v3"_s, 193, "1.2.840.10045.3.0.13"_s, "X9.62 c2tnb239v3"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::c2tnb359v1, $new(KnownOIDs, "c2tnb359v1"_s, 194, "1.2.840.10045.3.0.18"_s, "X9.62 c2tnb359v1"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::c2tnb431r1, $new(KnownOIDs, "c2tnb431r1"_s, 195, "1.2.840.10045.3.0.20"_s, "X9.62 c2tnb431r1"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::secp192r1, $new(KnownOIDs, "secp192r1"_s, 196, "1.2.840.10045.3.1.1"_s, "secp192r1"_s, $$new($StringArray, {
		"NIST P-192"_s,
		"X9.62 prime192v1"_s
	})));
	$assignStatic(KnownOIDs::prime192v2, $new(KnownOIDs, "prime192v2"_s, 197, "1.2.840.10045.3.1.2"_s, "X9.62 prime192v2"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::prime192v3, $new(KnownOIDs, "prime192v3"_s, 198, "1.2.840.10045.3.1.3"_s, "X9.62 prime192v3"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::prime239v1, $new(KnownOIDs, "prime239v1"_s, 199, "1.2.840.10045.3.1.4"_s, "X9.62 prime239v1"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::prime239v2, $new(KnownOIDs, "prime239v2"_s, 200, "1.2.840.10045.3.1.5"_s, "X9.62 prime239v2"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::prime239v3, $new(KnownOIDs, "prime239v3"_s, 201, "1.2.840.10045.3.1.6"_s, "X9.62 prime239v3"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::secp256r1, $new(KnownOIDs, "secp256r1"_s, 202, "1.2.840.10045.3.1.7"_s, "secp256r1"_s, $$new($StringArray, {
		"NIST P-256"_s,
		"X9.62 prime256v1"_s
	})));
	$assignStatic(KnownOIDs::SHA1withECDSA, $new(KnownOIDs, "SHA1withECDSA"_s, 203, "1.2.840.10045.4.1"_s));
	$assignStatic(KnownOIDs::SHA224withECDSA, $new(KnownOIDs, "SHA224withECDSA"_s, 204, "1.2.840.10045.4.3.1"_s));
	$assignStatic(KnownOIDs::SHA256withECDSA, $new(KnownOIDs, "SHA256withECDSA"_s, 205, "1.2.840.10045.4.3.2"_s));
	$assignStatic(KnownOIDs::SHA384withECDSA, $new(KnownOIDs, "SHA384withECDSA"_s, 206, "1.2.840.10045.4.3.3"_s));
	$assignStatic(KnownOIDs::SHA512withECDSA, $new(KnownOIDs, "SHA512withECDSA"_s, 207, "1.2.840.10045.4.3.4"_s));
	$assignStatic(KnownOIDs::SpecifiedSHA2withECDSA, $new(KnownOIDs, "SpecifiedSHA2withECDSA"_s, 208, "1.2.840.10045.4.3"_s));
	$assignStatic(KnownOIDs::X942_DH, $new($KnownOIDs$4, "X942_DH"_s, 209, "1.2.840.10046.2.1"_s, "DiffieHellman"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::brainpoolP160r1, $new(KnownOIDs, "brainpoolP160r1"_s, 210, "1.3.36.3.3.2.8.1.1.1"_s));
	$assignStatic(KnownOIDs::brainpoolP192r1, $new(KnownOIDs, "brainpoolP192r1"_s, 211, "1.3.36.3.3.2.8.1.1.3"_s));
	$assignStatic(KnownOIDs::brainpoolP224r1, $new(KnownOIDs, "brainpoolP224r1"_s, 212, "1.3.36.3.3.2.8.1.1.5"_s));
	$assignStatic(KnownOIDs::brainpoolP256r1, $new(KnownOIDs, "brainpoolP256r1"_s, 213, "1.3.36.3.3.2.8.1.1.7"_s));
	$assignStatic(KnownOIDs::brainpoolP320r1, $new(KnownOIDs, "brainpoolP320r1"_s, 214, "1.3.36.3.3.2.8.1.1.9"_s));
	$assignStatic(KnownOIDs::brainpoolP384r1, $new(KnownOIDs, "brainpoolP384r1"_s, 215, "1.3.36.3.3.2.8.1.1.11"_s));
	$assignStatic(KnownOIDs::brainpoolP512r1, $new(KnownOIDs, "brainpoolP512r1"_s, 216, "1.3.36.3.3.2.8.1.1.13"_s));
	$assignStatic(KnownOIDs::sect163k1, $new(KnownOIDs, "sect163k1"_s, 217, "1.3.132.0.1"_s, "sect163k1"_s, $$new($StringArray, {"NIST K-163"_s})));
	$assignStatic(KnownOIDs::sect163r1, $new(KnownOIDs, "sect163r1"_s, 218, "1.3.132.0.2"_s));
	$assignStatic(KnownOIDs::sect239k1, $new(KnownOIDs, "sect239k1"_s, 219, "1.3.132.0.3"_s));
	$assignStatic(KnownOIDs::sect113r1, $new(KnownOIDs, "sect113r1"_s, 220, "1.3.132.0.4"_s));
	$assignStatic(KnownOIDs::sect113r2, $new(KnownOIDs, "sect113r2"_s, 221, "1.3.132.0.5"_s));
	$assignStatic(KnownOIDs::secp112r1, $new(KnownOIDs, "secp112r1"_s, 222, "1.3.132.0.6"_s));
	$assignStatic(KnownOIDs::secp112r2, $new(KnownOIDs, "secp112r2"_s, 223, "1.3.132.0.7"_s));
	$assignStatic(KnownOIDs::secp160r1, $new(KnownOIDs, "secp160r1"_s, 224, "1.3.132.0.8"_s));
	$assignStatic(KnownOIDs::secp160k1, $new(KnownOIDs, "secp160k1"_s, 225, "1.3.132.0.9"_s));
	$assignStatic(KnownOIDs::secp256k1, $new(KnownOIDs, "secp256k1"_s, 226, "1.3.132.0.10"_s));
	$assignStatic(KnownOIDs::sect163r2, $new(KnownOIDs, "sect163r2"_s, 227, "1.3.132.0.15"_s, "sect163r2"_s, $$new($StringArray, {"NIST B-163"_s})));
	$assignStatic(KnownOIDs::sect283k1, $new(KnownOIDs, "sect283k1"_s, 228, "1.3.132.0.16"_s, "sect283k1"_s, $$new($StringArray, {"NIST K-283"_s})));
	$assignStatic(KnownOIDs::sect283r1, $new(KnownOIDs, "sect283r1"_s, 229, "1.3.132.0.17"_s, "sect283r1"_s, $$new($StringArray, {"NIST B-283"_s})));
	$assignStatic(KnownOIDs::sect131r1, $new(KnownOIDs, "sect131r1"_s, 230, "1.3.132.0.22"_s));
	$assignStatic(KnownOIDs::sect131r2, $new(KnownOIDs, "sect131r2"_s, 231, "1.3.132.0.23"_s));
	$assignStatic(KnownOIDs::sect193r1, $new(KnownOIDs, "sect193r1"_s, 232, "1.3.132.0.24"_s));
	$assignStatic(KnownOIDs::sect193r2, $new(KnownOIDs, "sect193r2"_s, 233, "1.3.132.0.25"_s));
	$assignStatic(KnownOIDs::sect233k1, $new(KnownOIDs, "sect233k1"_s, 234, "1.3.132.0.26"_s, "sect233k1"_s, $$new($StringArray, {"NIST K-233"_s})));
	$assignStatic(KnownOIDs::sect233r1, $new(KnownOIDs, "sect233r1"_s, 235, "1.3.132.0.27"_s, "sect233r1"_s, $$new($StringArray, {"NIST B-233"_s})));
	$assignStatic(KnownOIDs::secp128r1, $new(KnownOIDs, "secp128r1"_s, 236, "1.3.132.0.28"_s));
	$assignStatic(KnownOIDs::secp128r2, $new(KnownOIDs, "secp128r2"_s, 237, "1.3.132.0.29"_s));
	$assignStatic(KnownOIDs::secp160r2, $new(KnownOIDs, "secp160r2"_s, 238, "1.3.132.0.30"_s));
	$assignStatic(KnownOIDs::secp192k1, $new(KnownOIDs, "secp192k1"_s, 239, "1.3.132.0.31"_s));
	$assignStatic(KnownOIDs::secp224k1, $new(KnownOIDs, "secp224k1"_s, 240, "1.3.132.0.32"_s));
	$assignStatic(KnownOIDs::secp224r1, $new(KnownOIDs, "secp224r1"_s, 241, "1.3.132.0.33"_s, "secp224r1"_s, $$new($StringArray, {"NIST P-224"_s})));
	$assignStatic(KnownOIDs::secp384r1, $new(KnownOIDs, "secp384r1"_s, 242, "1.3.132.0.34"_s, "secp384r1"_s, $$new($StringArray, {"NIST P-384"_s})));
	$assignStatic(KnownOIDs::secp521r1, $new(KnownOIDs, "secp521r1"_s, 243, "1.3.132.0.35"_s, "secp521r1"_s, $$new($StringArray, {"NIST P-521"_s})));
	$assignStatic(KnownOIDs::sect409k1, $new(KnownOIDs, "sect409k1"_s, 244, "1.3.132.0.36"_s, "sect409k1"_s, $$new($StringArray, {"NIST K-409"_s})));
	$assignStatic(KnownOIDs::sect409r1, $new(KnownOIDs, "sect409r1"_s, 245, "1.3.132.0.37"_s, "sect409r1"_s, $$new($StringArray, {"NIST B-409"_s})));
	$assignStatic(KnownOIDs::sect571k1, $new(KnownOIDs, "sect571k1"_s, 246, "1.3.132.0.38"_s, "sect571k1"_s, $$new($StringArray, {"NIST K-571"_s})));
	$assignStatic(KnownOIDs::sect571r1, $new(KnownOIDs, "sect571r1"_s, 247, "1.3.132.0.39"_s, "sect571r1"_s, $$new($StringArray, {"NIST B-571"_s})));
	$assignStatic(KnownOIDs::ECDH, $new(KnownOIDs, "ECDH"_s, 248, "1.3.132.1.12"_s));
	$assignStatic(KnownOIDs::OIW_DES_CBC, $new(KnownOIDs, "OIW_DES_CBC"_s, 249, "1.3.14.3.2.7"_s, "DES/CBC"_s, $$new($StringArray, {"DES"_s})));
	$assignStatic(KnownOIDs::OIW_DSA, $new($KnownOIDs$5, "OIW_DSA"_s, 250, "1.3.14.3.2.12"_s, "DSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::OIW_JDK_SHA1withDSA, $new($KnownOIDs$6, "OIW_JDK_SHA1withDSA"_s, 251, "1.3.14.3.2.13"_s, "SHA1withDSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::OIW_SHA1withRSA_Odd, $new($KnownOIDs$7, "OIW_SHA1withRSA_Odd"_s, 252, "1.3.14.3.2.15"_s, "SHA1withRSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::DESede, $new(KnownOIDs, "DESede"_s, 253, "1.3.14.3.2.17"_s, "DESede"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SHA_1, $new(KnownOIDs, "SHA_1"_s, 254, "1.3.14.3.2.26"_s, "SHA-1"_s, $$new($StringArray, {
		"SHA"_s,
		"SHA1"_s
	})));
	$assignStatic(KnownOIDs::OIW_SHA1withDSA, $new($KnownOIDs$8, "OIW_SHA1withDSA"_s, 255, "1.3.14.3.2.27"_s, "SHA1withDSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::OIW_SHA1withRSA, $new($KnownOIDs$9, "OIW_SHA1withRSA"_s, 256, "1.3.14.3.2.29"_s, "SHA1withRSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::X25519, $new(KnownOIDs, "X25519"_s, 257, "1.3.101.110"_s));
	$assignStatic(KnownOIDs::X448, $new(KnownOIDs, "X448"_s, 258, "1.3.101.111"_s));
	$assignStatic(KnownOIDs::Ed25519, $new(KnownOIDs, "Ed25519"_s, 259, "1.3.101.112"_s));
	$assignStatic(KnownOIDs::Ed448, $new(KnownOIDs, "Ed448"_s, 260, "1.3.101.113"_s));
	$assignStatic(KnownOIDs::UCL_UserID, $new(KnownOIDs, "UCL_UserID"_s, 261, "0.9.2342.19200300.100.1.1"_s));
	$assignStatic(KnownOIDs::UCL_DomainComponent, $new(KnownOIDs, "UCL_DomainComponent"_s, 262, "0.9.2342.19200300.100.1.25"_s));
	$assignStatic(KnownOIDs::NETSCAPE_CertType, $new(KnownOIDs, "NETSCAPE_CertType"_s, 263, "2.16.840.1.113730.1.1"_s));
	$assignStatic(KnownOIDs::NETSCAPE_CertSequence, $new(KnownOIDs, "NETSCAPE_CertSequence"_s, 264, "2.16.840.1.113730.2.5"_s));
	$assignStatic(KnownOIDs::NETSCAPE_ExportApproved, $new(KnownOIDs, "NETSCAPE_ExportApproved"_s, 265, "2.16.840.1.113730.4.1"_s));
	$assignStatic(KnownOIDs::ORACLE_TrustedKeyUsage, $new(KnownOIDs, "ORACLE_TrustedKeyUsage"_s, 266, "2.16.840.1.113894.746875.1.1"_s));
	$assignStatic(KnownOIDs::ITUX509_RSA, $new($KnownOIDs$10, "ITUX509_RSA"_s, 267, "2.5.8.1.1"_s, "RSA"_s, $$new($StringArray, 0)));
	$assignStatic(KnownOIDs::SkipIPAddress, $new(KnownOIDs, "SkipIPAddress"_s, 268, "1.3.6.1.4.1.42.2.11.2.1"_s));
	$assignStatic(KnownOIDs::JAVASOFT_JDKKeyProtector, $new(KnownOIDs, "JAVASOFT_JDKKeyProtector"_s, 269, "1.3.6.1.4.1.42.2.17.1.1"_s));
	$assignStatic(KnownOIDs::JAVASOFT_JCEKeyProtector, $new(KnownOIDs, "JAVASOFT_JCEKeyProtector"_s, 270, "1.3.6.1.4.1.42.2.19.1"_s));
	$assignStatic(KnownOIDs::MICROSOFT_ExportApproved, $new(KnownOIDs, "MICROSOFT_ExportApproved"_s, 271, "1.3.6.1.4.1.311.10.3.3"_s));
	$assignStatic(KnownOIDs::$VALUES, KnownOIDs::$values());
	$assignStatic(KnownOIDs::debug, $Debug::getInstance("jca"_s));
	$assignStatic(KnownOIDs::name2enum, $new($ConcurrentHashMap));
	{
		if (KnownOIDs::debug != nullptr) {
			KnownOIDs::debug->println("Setting up name2enum:"_s);
		}
		{
			$var($KnownOIDsArray, arr$, KnownOIDs::values());
			for (int32_t len$ = arr$->length, i$ = 0; i$ < len$; ++i$) {
				KnownOIDs* o = arr$->get(i$);
				{
					KnownOIDs::register$(o);
				}
			}
		}
		;
	}
}

KnownOIDs::KnownOIDs() {
}

$Class* KnownOIDs::load$($String* name, bool initialize) {
	$FieldInfo fieldInfos$$[] = {
		{"CommonName", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, CommonName)},
		{"Surname", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, Surname)},
		{"SerialNumber", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SerialNumber)},
		{"CountryName", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, CountryName)},
		{"LocalityName", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, LocalityName)},
		{"StateName", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, StateName)},
		{"StreetAddress", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, StreetAddress)},
		{"OrgName", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OrgName)},
		{"OrgUnitName", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OrgUnitName)},
		{"Title", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, Title)},
		{"GivenName", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, GivenName)},
		{"Initials", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, Initials)},
		{"GenerationQualifier", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, GenerationQualifier)},
		{"DNQualifier", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, DNQualifier)},
		{"SubjectDirectoryAttributes", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SubjectDirectoryAttributes)},
		{"SubjectKeyID", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SubjectKeyID)},
		{"KeyUsage", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, KeyUsage)},
		{"PrivateKeyUsage", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PrivateKeyUsage)},
		{"SubjectAlternativeName", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SubjectAlternativeName)},
		{"IssuerAlternativeName", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, IssuerAlternativeName)},
		{"BasicConstraints", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, BasicConstraints)},
		{"CRLNumber", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, CRLNumber)},
		{"ReasonCode", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, ReasonCode)},
		{"HoldInstructionCode", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, HoldInstructionCode)},
		{"InvalidityDate", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, InvalidityDate)},
		{"DeltaCRLIndicator", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, DeltaCRLIndicator)},
		{"IssuingDistributionPoint", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, IssuingDistributionPoint)},
		{"CertificateIssuer", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, CertificateIssuer)},
		{"NameConstraints", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, NameConstraints)},
		{"CRLDistributionPoints", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, CRLDistributionPoints)},
		{"CertificatePolicies", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, CertificatePolicies)},
		{"CE_CERT_POLICIES_ANY", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, CE_CERT_POLICIES_ANY)},
		{"PolicyMappings", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PolicyMappings)},
		{"AuthorityKeyID", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AuthorityKeyID)},
		{"PolicyConstraints", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PolicyConstraints)},
		{"extendedKeyUsage", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, extendedKeyUsage)},
		{"anyExtendedKeyUsage", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, anyExtendedKeyUsage)},
		{"FreshestCRL", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, FreshestCRL)},
		{"InhibitAnyPolicy", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, InhibitAnyPolicy)},
		{"AuthInfoAccess", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AuthInfoAccess)},
		{"SubjectInfoAccess", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SubjectInfoAccess)},
		{"serverAuth", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, serverAuth)},
		{"clientAuth", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, clientAuth)},
		{"codeSigning", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, codeSigning)},
		{"emailProtection", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, emailProtection)},
		{"ipsecEndSystem", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, ipsecEndSystem)},
		{"ipsecTunnel", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, ipsecTunnel)},
		{"ipsecUser", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, ipsecUser)},
		{"KP_TimeStamping", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, KP_TimeStamping)},
		{"OCSPSigning", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OCSPSigning)},
		{"OCSP", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OCSP)},
		{"OCSPBasicResponse", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OCSPBasicResponse)},
		{"OCSPNonceExt", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OCSPNonceExt)},
		{"OCSPNoCheck", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OCSPNoCheck)},
		{"caIssuers", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, caIssuers)},
		{"AD_TimeStamping", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AD_TimeStamping)},
		{"caRepository", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, caRepository)},
		{"AES", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES)},
		{"AES_128$ECB$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_128$ECB$NoPadding)},
		{"AES_128$CBC$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_128$CBC$NoPadding)},
		{"AES_128$OFB$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_128$OFB$NoPadding)},
		{"AES_128$CFB$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_128$CFB$NoPadding)},
		{"AES_128$KW$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_128$KW$NoPadding)},
		{"AES_128$GCM$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_128$GCM$NoPadding)},
		{"AES_128$KWP$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_128$KWP$NoPadding)},
		{"AES_192$ECB$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_192$ECB$NoPadding)},
		{"AES_192$CBC$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_192$CBC$NoPadding)},
		{"AES_192$OFB$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_192$OFB$NoPadding)},
		{"AES_192$CFB$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_192$CFB$NoPadding)},
		{"AES_192$KW$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_192$KW$NoPadding)},
		{"AES_192$GCM$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_192$GCM$NoPadding)},
		{"AES_192$KWP$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_192$KWP$NoPadding)},
		{"AES_256$ECB$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_256$ECB$NoPadding)},
		{"AES_256$CBC$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_256$CBC$NoPadding)},
		{"AES_256$OFB$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_256$OFB$NoPadding)},
		{"AES_256$CFB$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_256$CFB$NoPadding)},
		{"AES_256$KW$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_256$KW$NoPadding)},
		{"AES_256$GCM$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_256$GCM$NoPadding)},
		{"AES_256$KWP$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, AES_256$KWP$NoPadding)},
		{"SHA_256", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA_256)},
		{"SHA_384", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA_384)},
		{"SHA_512", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA_512)},
		{"SHA_224", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA_224)},
		{"SHA_512$224", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA_512$224)},
		{"SHA_512$256", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA_512$256)},
		{"SHA3_224", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_224)},
		{"SHA3_256", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_256)},
		{"SHA3_384", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_384)},
		{"SHA3_512", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_512)},
		{"SHAKE128", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHAKE128)},
		{"SHAKE256", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHAKE256)},
		{"HmacSHA3_224", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, HmacSHA3_224)},
		{"HmacSHA3_256", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, HmacSHA3_256)},
		{"HmacSHA3_384", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, HmacSHA3_384)},
		{"HmacSHA3_512", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, HmacSHA3_512)},
		{"SHAKE128_LEN", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHAKE128_LEN)},
		{"SHAKE256_LEN", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHAKE256_LEN)},
		{"SHA224withDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA224withDSA)},
		{"SHA256withDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA256withDSA)},
		{"SHA384withDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA384withDSA)},
		{"SHA512withDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA512withDSA)},
		{"SHA3_224withDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_224withDSA)},
		{"SHA3_256withDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_256withDSA)},
		{"SHA3_384withDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_384withDSA)},
		{"SHA3_512withDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_512withDSA)},
		{"SHA3_224withECDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_224withECDSA)},
		{"SHA3_256withECDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_256withECDSA)},
		{"SHA3_384withECDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_384withECDSA)},
		{"SHA3_512withECDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_512withECDSA)},
		{"SHA3_224withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_224withRSA)},
		{"SHA3_256withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_256withRSA)},
		{"SHA3_384withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_384withRSA)},
		{"SHA3_512withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA3_512withRSA)},
		{"PKCS1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PKCS1)},
		{"RSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, RSA)},
		{"MD2withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, MD2withRSA)},
		{"MD5withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, MD5withRSA)},
		{"SHA1withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA1withRSA)},
		{"OAEP", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OAEP)},
		{"MGF1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, MGF1)},
		{"PSpecified", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PSpecified)},
		{"RSASSA_PSS", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, RSASSA_PSS)},
		{"SHA256withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA256withRSA)},
		{"SHA384withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA384withRSA)},
		{"SHA512withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA512withRSA)},
		{"SHA224withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA224withRSA)},
		{"SHA512$224withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA512$224withRSA)},
		{"SHA512$256withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA512$256withRSA)},
		{"DiffieHellman", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, DiffieHellman)},
		{"PBEWithMD5AndDES", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PBEWithMD5AndDES)},
		{"PBEWithMD5AndRC2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PBEWithMD5AndRC2)},
		{"PBEWithSHA1AndDES", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PBEWithSHA1AndDES)},
		{"PBEWithSHA1AndRC2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PBEWithSHA1AndRC2)},
		{"PBKDF2WithHmacSHA1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PBKDF2WithHmacSHA1)},
		{"PBES2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PBES2)},
		{"PKCS7", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PKCS7)},
		{"Data", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, Data)},
		{"SignedData", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SignedData)},
		{"JDK_OLD_Data", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, JDK_OLD_Data)},
		{"JDK_OLD_SignedData", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, JDK_OLD_SignedData)},
		{"EnvelopedData", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, EnvelopedData)},
		{"SignedAndEnvelopedData", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SignedAndEnvelopedData)},
		{"DigestedData", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, DigestedData)},
		{"EncryptedData", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, EncryptedData)},
		{"EmailAddress", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, EmailAddress)},
		{"UnstructuredName", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, UnstructuredName)},
		{"ContentType", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, ContentType)},
		{"MessageDigest", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, MessageDigest)},
		{"SigningTime", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SigningTime)},
		{"CounterSignature", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, CounterSignature)},
		{"ChallengePassword", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, ChallengePassword)},
		{"UnstructuredAddress", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, UnstructuredAddress)},
		{"ExtendedCertificateAttributes", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, ExtendedCertificateAttributes)},
		{"IssuerAndSerialNumber", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, IssuerAndSerialNumber)},
		{"ExtensionRequest", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, ExtensionRequest)},
		{"SMIMECapability", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SMIMECapability)},
		{"TimeStampTokenInfo", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, TimeStampTokenInfo)},
		{"SigningCertificate", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SigningCertificate)},
		{"SignatureTimestampToken", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SignatureTimestampToken)},
		{"CHACHA20_POLY1305", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, CHACHA20_POLY1305)},
		{"FriendlyName", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, FriendlyName)},
		{"LocalKeyID", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, LocalKeyID)},
		{"CertTypeX509", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, CertTypeX509)},
		{"CMSAlgorithmProtection", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, CMSAlgorithmProtection)},
		{"PBEWithSHA1AndRC4_128", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PBEWithSHA1AndRC4_128)},
		{"PBEWithSHA1AndRC4_40", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PBEWithSHA1AndRC4_40)},
		{"PBEWithSHA1AndDESede", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PBEWithSHA1AndDESede)},
		{"PBEWithSHA1AndRC2_128", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PBEWithSHA1AndRC2_128)},
		{"PBEWithSHA1AndRC2_40", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PBEWithSHA1AndRC2_40)},
		{"PKCS8ShroudedKeyBag", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, PKCS8ShroudedKeyBag)},
		{"CertBag", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, CertBag)},
		{"SecretBag", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SecretBag)},
		{"MD2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, MD2)},
		{"MD5", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, MD5)},
		{"HmacSHA1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, HmacSHA1)},
		{"HmacSHA224", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, HmacSHA224)},
		{"HmacSHA256", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, HmacSHA256)},
		{"HmacSHA384", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, HmacSHA384)},
		{"HmacSHA512", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, HmacSHA512)},
		{"HmacSHA512$224", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, HmacSHA512$224)},
		{"HmacSHA512$256", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, HmacSHA512$256)},
		{"RC2$CBC$PKCS5Padding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, RC2$CBC$PKCS5Padding)},
		{"ARCFOUR", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, ARCFOUR)},
		{"DESede$CBC$NoPadding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, DESede$CBC$NoPadding)},
		{"RC5$CBC$PKCS5Padding", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, RC5$CBC$PKCS5Padding)},
		{"DSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, DSA)},
		{"SHA1withDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA1withDSA)},
		{"EC", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, EC)},
		{"c2tnb191v1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, c2tnb191v1)},
		{"c2tnb191v2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, c2tnb191v2)},
		{"c2tnb191v3", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, c2tnb191v3)},
		{"c2tnb239v1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, c2tnb239v1)},
		{"c2tnb239v2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, c2tnb239v2)},
		{"c2tnb239v3", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, c2tnb239v3)},
		{"c2tnb359v1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, c2tnb359v1)},
		{"c2tnb431r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, c2tnb431r1)},
		{"secp192r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp192r1)},
		{"prime192v2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, prime192v2)},
		{"prime192v3", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, prime192v3)},
		{"prime239v1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, prime239v1)},
		{"prime239v2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, prime239v2)},
		{"prime239v3", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, prime239v3)},
		{"secp256r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp256r1)},
		{"SHA1withECDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA1withECDSA)},
		{"SHA224withECDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA224withECDSA)},
		{"SHA256withECDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA256withECDSA)},
		{"SHA384withECDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA384withECDSA)},
		{"SHA512withECDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA512withECDSA)},
		{"SpecifiedSHA2withECDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SpecifiedSHA2withECDSA)},
		{"X942_DH", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, X942_DH)},
		{"brainpoolP160r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, brainpoolP160r1)},
		{"brainpoolP192r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, brainpoolP192r1)},
		{"brainpoolP224r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, brainpoolP224r1)},
		{"brainpoolP256r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, brainpoolP256r1)},
		{"brainpoolP320r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, brainpoolP320r1)},
		{"brainpoolP384r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, brainpoolP384r1)},
		{"brainpoolP512r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, brainpoolP512r1)},
		{"sect163k1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect163k1)},
		{"sect163r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect163r1)},
		{"sect239k1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect239k1)},
		{"sect113r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect113r1)},
		{"sect113r2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect113r2)},
		{"secp112r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp112r1)},
		{"secp112r2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp112r2)},
		{"secp160r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp160r1)},
		{"secp160k1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp160k1)},
		{"secp256k1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp256k1)},
		{"sect163r2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect163r2)},
		{"sect283k1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect283k1)},
		{"sect283r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect283r1)},
		{"sect131r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect131r1)},
		{"sect131r2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect131r2)},
		{"sect193r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect193r1)},
		{"sect193r2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect193r2)},
		{"sect233k1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect233k1)},
		{"sect233r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect233r1)},
		{"secp128r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp128r1)},
		{"secp128r2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp128r2)},
		{"secp160r2", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp160r2)},
		{"secp192k1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp192k1)},
		{"secp224k1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp224k1)},
		{"secp224r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp224r1)},
		{"secp384r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp384r1)},
		{"secp521r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, secp521r1)},
		{"sect409k1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect409k1)},
		{"sect409r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect409r1)},
		{"sect571k1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect571k1)},
		{"sect571r1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, sect571r1)},
		{"ECDH", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, ECDH)},
		{"OIW_DES_CBC", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OIW_DES_CBC)},
		{"OIW_DSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OIW_DSA)},
		{"OIW_JDK_SHA1withDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OIW_JDK_SHA1withDSA)},
		{"OIW_SHA1withRSA_Odd", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OIW_SHA1withRSA_Odd)},
		{"DESede", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, DESede)},
		{"SHA_1", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SHA_1)},
		{"OIW_SHA1withDSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OIW_SHA1withDSA)},
		{"OIW_SHA1withRSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, OIW_SHA1withRSA)},
		{"X25519", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, X25519)},
		{"X448", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, X448)},
		{"Ed25519", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, Ed25519)},
		{"Ed448", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, Ed448)},
		{"UCL_UserID", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, UCL_UserID)},
		{"UCL_DomainComponent", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, UCL_DomainComponent)},
		{"NETSCAPE_CertType", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, NETSCAPE_CertType)},
		{"NETSCAPE_CertSequence", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, NETSCAPE_CertSequence)},
		{"NETSCAPE_ExportApproved", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, NETSCAPE_ExportApproved)},
		{"ORACLE_TrustedKeyUsage", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, ORACLE_TrustedKeyUsage)},
		{"ITUX509_RSA", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, ITUX509_RSA)},
		{"SkipIPAddress", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, SkipIPAddress)},
		{"JAVASOFT_JDKKeyProtector", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, JAVASOFT_JDKKeyProtector)},
		{"JAVASOFT_JCEKeyProtector", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, JAVASOFT_JCEKeyProtector)},
		{"MICROSOFT_ExportApproved", "Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC | $FINAL | $ENUM, $staticField(KnownOIDs, MICROSOFT_ExportApproved)},
		{"$VALUES", "[Lsun/security/util/KnownOIDs;", nullptr, $PRIVATE | $STATIC | $FINAL | $SYNTHETIC, $staticField(KnownOIDs, $VALUES)},
		{"stdName", "Ljava/lang/String;", nullptr, $PRIVATE, $field(KnownOIDs, stdName$)},
		{"oid", "Ljava/lang/String;", nullptr, $PRIVATE, $field(KnownOIDs, oid)},
		{"aliases", "[Ljava/lang/String;", nullptr, $PRIVATE, $field(KnownOIDs, aliases$)},
		{"debug", "Lsun/security/util/Debug;", nullptr, $PRIVATE | $STATIC | $FINAL, $staticField(KnownOIDs, debug)},
		{"name2enum", "Ljava/util/concurrent/ConcurrentHashMap;", "Ljava/util/concurrent/ConcurrentHashMap<Ljava/lang/String;Lsun/security/util/KnownOIDs;>;", $PRIVATE | $STATIC | $FINAL, $staticField(KnownOIDs, name2enum)},
		{}
	};
	$MethodInfo methodInfos$$[] = {
		{"$values", "()[Lsun/security/util/KnownOIDs;", nullptr, $PRIVATE | $STATIC | $SYNTHETIC, $staticMethod(KnownOIDs, $values, $KnownOIDsArray*)},
		{"<init>", "(Ljava/lang/String;ILjava/lang/String;)V", "(Ljava/lang/String;)V", $PRIVATE, $method(KnownOIDs, init$, void, $String*, int32_t, $String*)},
		{"<init>", "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;[Ljava/lang/String;)V", "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)V", $PRIVATE | $TRANSIENT, $method(KnownOIDs, init$, void, $String*, int32_t, $String*, $String*, $StringArray*)},
		{"aliases", "()[Ljava/lang/String;", nullptr, $PUBLIC, $virtualMethod(KnownOIDs, aliases, $StringArray*)},
		{"findMatch", "(Ljava/lang/String;)Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC, $staticMethod(KnownOIDs, findMatch, KnownOIDs*, $String*)},
		{"register", "(Lsun/security/util/KnownOIDs;)V", nullptr, $PRIVATE | $STATIC, $staticMethod(KnownOIDs, register$, void, KnownOIDs*)},
		{"registerNames", "()Z", nullptr, 0, $virtualMethod(KnownOIDs, registerNames, bool)},
		{"stdName", "()Ljava/lang/String;", nullptr, $PUBLIC, $virtualMethod(KnownOIDs, stdName, $String*)},
		{"value", "()Ljava/lang/String;", nullptr, $PUBLIC, $virtualMethod(KnownOIDs, value, $String*)},
		{"valueOf", "(Ljava/lang/String;)Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC, $staticMethod(KnownOIDs, valueOf, KnownOIDs*, $String*)},
		{"values", "()[Lsun/security/util/KnownOIDs;", nullptr, $PUBLIC | $STATIC, $staticMethod(KnownOIDs, values, $KnownOIDsArray*)},
		{}
	};
	$InnerClassInfo innerClassesInfo$$[] = {
		{"sun.security.util.KnownOIDs$10", nullptr, nullptr, $FINAL | $ENUM},
		{"sun.security.util.KnownOIDs$9", nullptr, nullptr, $FINAL | $ENUM},
		{"sun.security.util.KnownOIDs$8", nullptr, nullptr, $FINAL | $ENUM},
		{"sun.security.util.KnownOIDs$7", nullptr, nullptr, $FINAL | $ENUM},
		{"sun.security.util.KnownOIDs$6", nullptr, nullptr, $FINAL | $ENUM},
		{"sun.security.util.KnownOIDs$5", nullptr, nullptr, $FINAL | $ENUM},
		{"sun.security.util.KnownOIDs$4", nullptr, nullptr, $FINAL | $ENUM},
		{"sun.security.util.KnownOIDs$3", nullptr, nullptr, $FINAL | $ENUM},
		{"sun.security.util.KnownOIDs$2", nullptr, nullptr, $FINAL | $ENUM},
		{"sun.security.util.KnownOIDs$1", nullptr, nullptr, $FINAL | $ENUM},
		{}
	};
	$ClassInfo classInfo$$ = {
		$PUBLIC | $ACC_SUPER | $ENUM,
		"sun.security.util.KnownOIDs",
		"java.lang.Enum",
		nullptr,
		fieldInfos$$,
		methodInfos$$,
		"Ljava/lang/Enum<Lsun/security/util/KnownOIDs;>;",
		nullptr,
		innerClassesInfo$$,
		nullptr,
		nullptr,
		"sun.security.util.KnownOIDs$10,sun.security.util.KnownOIDs$9,sun.security.util.KnownOIDs$8,sun.security.util.KnownOIDs$7,sun.security.util.KnownOIDs$6,sun.security.util.KnownOIDs$5,sun.security.util.KnownOIDs$4,sun.security.util.KnownOIDs$3,sun.security.util.KnownOIDs$2,sun.security.util.KnownOIDs$1"
	};
	$loadClass(KnownOIDs, name, initialize, &classInfo$$, KnownOIDs::clinit$, []($Class* clazz) -> $Object* {
		return $of($alloc(KnownOIDs));
	});
	return class$;
}

$Class* KnownOIDs::class$ = nullptr;

		} // util
	} // security
} // sun