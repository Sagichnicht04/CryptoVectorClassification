#include <cstdint>
#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <fstream>

using namespace std;

class RC4 {
private:
    vector<uint8_t> S;
    int i,j;

public:
    RC4(const vector<uint8_t>& key):S(256),i(0),j(0) {
        for (int k=0;k<256;++k) {
            S[k]=k;
        }


        int j_ksa=0;
        for (int k=0;k<256;++k) {
            j_ksa=(j_ksa+S[k]+key[k % key.size()]) %256;
            swap(S[k],S[j_ksa]);
        }
    }

    vector<uint8_t> process(const vector<uint8_t>& data) {
        vector<uint8_t> output(data.size());


        for (size_t k=0;k<data.size();++k) {
            i=(i+1)%256;
            j=(j+S[i]) % 256;

            swap(S[i],S[j]);

            uint8_t keystream_byte=S[(S[i]+S[j])%256];
            output[k]=data[k] ^ keystream_byte;
        }
        return output;
    }

};

void printHex(const string& label, const vector<uint8_t>& data) {
    cout<<label<<":";
    for (uint8_t byte: data) {
        cout<<hex<<setw(2)<<setfill('0')<<(int)byte<<" ";

    }
    cout<<dec<<"\n";
}


int main() {
    ifstream file("flag.enc",ios::binary);
    vector<uint8_t> file_data((istreambuf_iterator<char>(file)),istreambuf_iterator<char>());
    file.close();


    size_t footer_start=file_data.size()-13;

    uint32_t original_len=file_data[footer_start] |
        (file_data[footer_start+1]<<8) |
            (file_data[footer_start+2]<<16) |
                (file_data[footer_start+3]<<24);

    vector<uint8_t> key(file_data.begin()+footer_start+4,file_data.begin()+footer_start+12);

    vector<uint8_t> ciphertext(file_data.begin(),file_data.begin()+footer_start);


    for (int i=0;i<ciphertext.size();++i) {
        ciphertext[i] ^= key[i%8];
    }

    RC4 rc4(key);
    vector<uint8_t>rc4_output=rc4.process(ciphertext);

    for (int i = 0; i < rc4_output.size(); ++i) {
        rc4_output[i] ^= key[i % 8];
    }

    for (size_t i = 0; i < original_len && i < rc4_output.size(); ++i) {
        cout << (char)rc4_output[i];
    }
    cout << "\n";

    return 0;
}
