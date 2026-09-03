#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/md5.h>

using namespace std;

// Funktion för att skapa MD5-hash av en sträng
#include <openssl/evp.h>

string md5(const string& input) {
    EVP_MD_CTX* mdctx;
    const EVP_MD* md = EVP_md5();
    unsigned char digest[MD5_DIGEST_LENGTH];
    unsigned int digest_len;

    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, nullptr);
    EVP_DigestUpdate(mdctx, input.c_str(), input.length());
    EVP_DigestFinal_ex(mdctx, digest, &digest_len);
    EVP_MD_CTX_free(mdctx);

    stringstream ss;
    for (int i = 0; i < digest_len; i++) {
        ss << hex << setw(2) << setfill('0') << (int)digest[i];
    }

    return ss.str();
}


// Funktion för att skapa användare
void createUser() {
    string username, password;
    cout << "Ange användarnamn (epostadress): ";
    cin >> username;

    // Validera epostadress (en enkel validering)
    size_t atPos = username.find('@');
    if (atPos == string::npos) {
        cout << "Ogiltig epostadress." << endl;
        return;
    }

    cout << "Ange lösenord: ";
    cin >> password;

    // Kontrollera lösenordskraven
    if (password.length() < 8 ||
        password.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == string::npos ||
        password.find_first_of("abcdefghijklmnopqrstuvwxyz") == string::npos ||
        password.find_first_of("0123456789") == string::npos ||
        password.find_first_of("!@#$%^&*()-_+=<>?") == string::npos) {
        cout << "Ogiltigt lösenord. Följ lösenordskraven nedan:\n" <<
            "1. Minst 8 tecken\n" <<
            "2. Minst en stor bokstav\n" <<
            "3. Minst en liten bokstav\n" <<
            "4. Minst en siffra\n" <<
            "5. Minst ett specialtecken\n";
        return;
    }

    // Kontrollera om användaren redan finns
    ifstream file("users.txt");
    string line;
    while (getline(file, line)) {
        size_t pos = line.find(':');
        if (pos != string::npos && line.substr(0, pos) == username) {
            cout << "Användaren finns redan." << endl;
            return;
        }
    }
    file.close();

    // Hasha lösenordet med MD5
    string hashedPassword = md5(password);

    // Spara användaren och hashade lösenordet i filen
    ofstream outFile("users.txt", ios::app);
    outFile << username << ':' << hashedPassword << endl;
    outFile.close();

    cout << "Användaren har skapats." << endl;
}

// Funktion för att testa inloggning
void testLogin() {
    string username, password;
    cout << "Ange användarnamn: ";
    cin >> username;
    cout << "Ange lösenord: ";
    cin >> password;

    // Hasha det angivna lösenordet
    string hashedPassword = md5(password);

    // Kontrollera användaren och lösenordet mot filen
    ifstream file("users.txt");
    string line;
    while (getline(file, line)) {
        size_t pos = line.find(':');
        if (pos != string::npos && line.substr(0, pos) == username) {
            if (line.substr(pos + 1) == hashedPassword) {
                cout << "OK, Det gick att logga in." << endl;
                return;
            } else {
                cout << "Fel lösenord." << endl;
                return;
            }
        }
    }
    file.close();

    cout << "Användaren hittades inte." << endl;
}

int main() {
    int choice;
    do {
        cout << "\n1. Skapa användare\n2. Test login\n0. Avsluta\nVälj: ";
        cin >> choice;

        switch (choice) {
            case 1:
                createUser();
                break;
            case 2:
                testLogin();
                break;
            case 0:
                cout << "Programmet avslutas." << endl;
                break;
            default:
                cout << "Ogiltigt val. Försök igen." << endl;
        }
    } while (choice != 0);

    return 0;
}