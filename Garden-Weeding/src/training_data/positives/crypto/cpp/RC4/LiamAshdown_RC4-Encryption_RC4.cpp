/*
* Liam Ashdown
* Copyright(C) 2019
*
* This program is free software : you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
*(at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
*but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.If not, see < http://www.gnu.org/licenses/>.
*/

#include "RC4.h"

namespace SteerStone
{
    /// Constructor
    RC4::RC4()
    {
    }

    /// Deconstructor
    RC4::~RC4()
    {
    }

    /// InitializeKey
    /// Begin our KSA initialization
    void RC4::Initialize()
    {
        /// Permutate our S array
        for (int l_I = 0; l_I < 256; l_I++)
            m_S[l_I] = l_I;

        char l_K[256] = { 0 };
        int l_J = 0;
        for (int l_I = 0; l_I < 256; l_I++)
        {
            l_J = (l_J + m_S[l_I] + l_K[l_I] + m_Key[l_I % m_Key.length()]) % 256;
            std::swap(m_S[l_I], m_S[l_J]);
        }
    }

    /// SetData
    /// @p_Data : Set data to be encrypted
    void RC4::SetData(std::string const p_Data)
    {
        m_Data = p_Data;
    }

    /// SetKey
    /// @p_Key : The key we will use to encrypt/decrypt with
    bool RC4::SetKey(std::string const p_Key)
    {
        /// Key must not be smaller than 40 or larger than 512
        if (p_Key.length() < 40 || p_Key.length() > 512)
            return false;

        m_Key = p_Key;

        return true;
    }

    /// Encrypt 
    /// Return our encrypted data
    std::string RC4::Encrypt()
    {
        Initialize();

        int l_I = 0;
        int l_J = 0;
        int l_Output = 0;

        for (int l_K = 0; l_K < m_Data.length(); l_K++)
        {
            l_I = (l_I + 1) % 256;
            l_J = (l_J + m_S[l_I]) % 256;

            std::swap(m_S[l_I], m_S[l_J]);

            l_Output = (m_S[l_I] + m_S[l_J]) % 256;
            m_Data[l_K] ^= m_S[l_Output];
        }

        return m_Data;
    }

    /// Decrypt
    /// Return our decrypted data
    std::string RC4::Decrypt()
    {
        Initialize();

        int l_I = 0;
        int l_J = 0;
        int l_Output = 0;

        for (int l_K = 0; l_K < m_Data.length(); l_K++)
        {
            l_I = (l_I + 1) % 256;
            l_J = (l_J + m_S[l_I]) % 256;

            std::swap(m_S[l_I], m_S[l_J]);

            l_Output = (m_S[l_I] + m_S[l_J]) % 256;
            m_Data[l_K] ^= m_S[l_Output];
        }

        return m_Data;
    }
}
