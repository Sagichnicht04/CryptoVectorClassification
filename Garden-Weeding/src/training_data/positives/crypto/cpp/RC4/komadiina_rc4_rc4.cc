#include <iostream>
#include <vector>
#include <stack>
#include <stdint.h>
#include <sstream>

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec)
{
    os << "[ ";
    for (T elem : vec)
        os << (int)elem << " ";

    return os << "]";
}

class RC4
{
private:
    uint8_t m_numbits;
    std::vector<uint8_t> m_key;
    std::vector<uint64_t> m_statevector;

    mutable std::string m_text;
    mutable std::string m_cipher;

public:
    RC4(uint8_t numbits, std::string inkey) : m_numbits(numbits)
    {
        std::string key = inkey;

        if (inkey.substr(0, 2) == "0x")
            key = inkey.substr(2, inkey.size());

        if (key.size() % 2 != 0)
            inkey = "0" + key;

        for (int i = 0; i < key.size(); i += 2)
        {
            /* --------- LOOOOL ---------- */
            // cpp type of problem
            char a = key[i];
            char b = key[i + 1];
            std::string str = std::string({a, b});
            /* --------------------------- */

            uint8_t byte = _parse_hexstring(str);
            m_key.push_back(byte);
        }
    }

    void KSA()
    {
        _initialize_statevector();
        uint8_t keylen = m_key.size();
        uint8_t svlen = m_statevector.size();

        uint64_t j = 0;
        for (int i = 0; i < svlen; i++)
        {
            j = (j + m_statevector[i] + m_key[i % keylen]) % svlen;
            this->swap<uint64_t>(m_statevector, i, j);
        }
    }

    void PRGA(const std::string &text)
    {
        std::stack<char> st;
        for (int i = text.size() - 1; i >= 0; i--)
            st.push(text[i]);

        m_text = text;
        m_cipher.clear();

        uint8_t svlen = m_statevector.size();
        char c;
        uint64_t i = 0, j = 0, k;

        while (st.size())
        {
            i = (i + 1) % svlen;
            j = (j + m_statevector[i]) % svlen;
            this->swap(m_statevector, i, j);

            k = m_statevector[(m_statevector[i] + m_statevector[j]) % svlen];
            c = st.top();
            st.pop();

            m_cipher += c ^ (char)k;
        }
    }

    std::string encrypt(const std::string &text)
    {
        KSA();
        PRGA(text);

        return m_cipher;
    }

    friend std::ostream &operator<<(std::ostream &os, const RC4 &obj)
    {
        os << "Key vector: " << obj.m_key << std::endl;
        os << "State vector: " << obj.m_statevector << std::endl;
        os << "Text: " << obj.m_text << std::endl;
        os << "Cipher/encrypted: " << obj.m_cipher;

        return os;
    }

private:
    uint8_t _parse_hexstring(std::string str)
    {
        return (uint8_t)std::stol(str, nullptr, 16);
    }

    void _initialize_statevector()
    {
        for (int i = 0; i < m_numbits / 8; i++)
            m_statevector.push_back(i);
    }

    template <typename T>
    void swap(std::vector<T> &ctr, uint8_t src, uint8_t dest)
    {
        T temp = ctr[dest];
        ctr[dest] = ctr[src];
        ctr[src] = temp;
    }
};

int main(int argC, char **argV)
{
    std::string key = "", text = "";
    uint8_t bits = 0;

    for (int i = 1; i < argC; i++)
    {
        std::string str = std::string(argV[i]);

        if (str == "-k" || str == "--key")
            key = argV[i + 1];
        else if (str == "-b" || str == "--bits")
            bits = std::stoi(argV[i + 1], nullptr, 10);
        else if (str == "-t" || str == "--text")
            text = argV[i + 1];
    }

    if (text == "")
    {
        std::cout << "text to encrypt: ";
        std::getline(std::cin, text);
    }

    if (key == "")
    {
        std::cout << "key value (hex): ";
        std::getline(std::cin, key);
    }

    if (bits == 0)
    {
        std::cout << "vector length (bits): ";
        std::cin >> bits;
        std::cin.ignore(1); // newl
    }

    RC4 algo = RC4(bits, key);
    algo.encrypt(text);
    std::cout << algo << std::endl;
}