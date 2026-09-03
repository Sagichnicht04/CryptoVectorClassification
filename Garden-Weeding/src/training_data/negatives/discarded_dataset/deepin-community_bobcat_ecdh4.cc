
#include "ecdh.ih"

ECDH::ECDH(string const &curveName, string const &peerPubFname,
           string const &initSecFname, string const &password)
:
    ECDH()
{
    set(curveName, peerPubFname, initSecFname, password);
}

