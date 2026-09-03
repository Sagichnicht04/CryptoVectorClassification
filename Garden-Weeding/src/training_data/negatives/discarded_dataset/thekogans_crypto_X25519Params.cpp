// Copyright 2016 Boris Kogan (boris@thekogans.net)
//
// This file is part of libthekogans_crypto.
//
// libthekogans_crypto is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// libthekogans_crypto is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libthekogans_crypto. If not, see <http://www.gnu.org/licenses/>.

#include "thekogans/util/SecureAllocator.h"
#include "thekogans/crypto/Curve25519.h"
#include "thekogans/crypto/X25519AsymmetricKey.h"
#include "thekogans/crypto/X25519Params.h"

namespace thekogans {
    namespace crypto {

        #if !defined (THEKOGANS_CRYPTO_MIN_X25519_PARAMS_IN_PAGE)
            #define THEKOGANS_CRYPTO_MIN_X25519_PARAMS_IN_PAGE 16
        #endif // !defined (THEKOGANS_CRYPTO_MIN_X25519_PARAMS_IN_PAGE)

        THEKOGANS_CRYPTO_IMPLEMENT_SERIALIZABLE (
            thekogans::crypto::X25519Params,
            1,
            0,
            THEKOGANS_CRYPTO_MIN_X25519_PARAMS_IN_PAGE,
            Params::TYPE)

        AsymmetricKey::SharedPtr X25519Params::CreateKey (
                const ID &id,
                const std::string &name,
                const std::string &description) const {
            util::SecureVector<util::ui8> privateKey (X25519::PRIVATE_KEY_LENGTH);
            X25519::CreateKey (privateKey.data ());
            return AsymmetricKey::SharedPtr (
                new X25519AsymmetricKey (privateKey.data (), true, id, name, description));
        }

        std::size_t X25519Params::Size () const noexcept {
            return Params::Size ();
        }

        void X25519Params::Read (
                const util::SerializableHeader &header,
                util::Serializer &serializer) {
            Params::Read (header, serializer);
        }

        void X25519Params::Write (util::Serializer &serializer) const {
            Params::Write (serializer);
        }

        void X25519Params::ReadXML (
                const util::SerializableHeader &header,
                const pugi::xml_node &node) {
            Params::ReadXML (header, node);
        }

        void X25519Params::WriteXML (pugi::xml_node &node) const {
            Params::WriteXML (node);
        }

        void X25519Params::ReadJSON (
                const util::SerializableHeader &header,
                const util::JSON::Object &object) {
            Params::ReadJSON (header, object);
        }

        void X25519Params::WriteJSON (util::JSON::Object &object) const {
            Params::WriteJSON (object);
        }

    } // namespace crypto
} // namespace thekogans
