#include "mt2_mitm.h"
#include "logger.h"
#include <cstring>
#include <cstdlib>
#include <mutex>
#include <winsock2.h>
#include <windows.h>
#include <algorithm>

// Crypto++ headers
#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>
#include <cryptopp/modes.h>
#include <cryptopp/twofish.h>
#include <cryptopp/rc6.h>
#include <cryptopp/mars.h>
#include <cryptopp/serpent.h>
#include <cryptopp/cast.h>
#include <cryptopp/idea.h>
#include <cryptopp/des.h>
#include <cryptopp/camellia.h>
#include <cryptopp/seed.h>
#include <cryptopp/rc5.h>
#include <cryptopp/blowfish.h>
#include <cryptopp/tea.h>
#include <cryptopp/shacal2.h>
#include <cryptopp/filters.h>

using CryptoPP::Integer;
using CryptoPP::AutoSeededRandomPool;
using CryptoPP::byte;

// ═══════════════════════════════════════════════════════════════════════════
// RFC 5114 Section 2.1 -- 1024-bit MODP with 160-bit subgroup
// ═══════════════════════════════════════════════════════════════════════════
static const char* DH_P_HEX =
    "0xB10B8F96A080E01DDE92DE5EAE5D54EC52C99FBCFB06A3C6"
    "9A6A9DCA52D23B616073E28675A23D189838EF1E2EE652C0"
    "13ECB4AEA906112324975C3CD49B83BFACCBDD7D90C4BD70"
    "98488E9C219A73724EFFD6FAE5644738FAA31A4FF55BCCC0"
    "A151AF5F0DC8B4BD45BF37DF365C1A65E68CFDA76D4DA708"
    "DF1FB2BC2E4A4371";

static const char* DH_G_HEX =
    "0xA4D1CBD5C3FD34126765A442EFB99905F8104DD258AC507F"
    "D6406CFF14266D31266FEA1E5C41564B777E690F5504F213"
    "160217B4B01B886A5E91547F9E2749F4D7FBD7D3B9A92EE1"
    "909D0D2263F80A76A6A24C087A091F531DBF0A0169B6A28A"
    "D662A4D18E73AFA32D779D5918D08BC8858F4DCEF97C2A24"
    "855E6EEB22B3B2E5";

static const char* DH_Q_HEX =
    "0xF518AA8781A8DF278ABA4E7D64B7CB9D49462353";

// ═══════════════════════════════════════════════════════════════════════════
// Cipher algorithm table (14 algorithms)
// ═══════════════════════════════════════════════════════════════════════════
struct AlgorithmInfo {
    const char* name;
    int keyLen;
    int blockSize;
};

static const AlgorithmInfo ALGORITHMS[14] = {
    {"Twofish",    16, 16},  // 0
    {"RC6",        16, 16},  // 1
    {"MARS",       16, 16},  // 2
    {"Twofish",    16, 16},  // 3
    {"Serpent",     16, 16},  // 4
    {"CAST-256",   16, 16},  // 5
    {"IDEA",       16,  8},  // 6
    {"DES-EDE2",   16,  8},  // 7
    {"Camellia",   16, 16},  // 8
    {"SEED",       16, 16},  // 9
    {"RC5",        16,  8},  // 10
    {"Blowfish",   16,  8},  // 11
    {"TEA",        16,  8},  // 12
    {"SHACAL-2",   16, 32},  // 13
};

// ═══════════════════════════════════════════════════════════════════════════
// CTR mode cipher using Crypto++ native CTR_Mode
// No manual counter increment, no virtual dispatch, no heap in hot path
// ═══════════════════════════════════════════════════════════════════════════
#include <cryptopp/modes.h>

class CtrCipher {
public:
    CtrCipher() = default;

    bool Init(int algIndex, const uint8_t* key, int keyLen, const uint8_t* iv, int blockSize, bool quiet = false) {
        m_algIndex = algIndex;
        m_ready = false;

        try {
            // Use Crypto++'s CTR_Mode which handles counter increment internally
            switch (algIndex) {
                case 0: case 3: InitCtr<CryptoPP::Twofish>(key, keyLen, iv); break;
                case 1:  InitCtr<CryptoPP::RC6>(key, keyLen, iv); break;
                case 2:  InitCtr<CryptoPP::MARS>(key, keyLen, iv); break;
                case 4:  InitCtr<CryptoPP::Serpent>(key, keyLen, iv); break;
                case 5:  InitCtr<CryptoPP::CAST256>(key, keyLen, iv); break;
                case 6:  InitCtr<CryptoPP::IDEA>(key, keyLen, iv); break;
                case 7:  InitCtr<CryptoPP::DES_EDE2>(key, keyLen, iv); break;
                case 8:  InitCtr<CryptoPP::Camellia>(key, keyLen, iv); break;
                case 9:  InitCtr<CryptoPP::SEED>(key, keyLen, iv); break;
                case 10: InitCtr<CryptoPP::RC5>(key, keyLen, iv); break;
                case 11: InitCtr<CryptoPP::Blowfish>(key, keyLen, iv); break;
                case 12: InitCtr<CryptoPP::TEA>(key, keyLen, iv); break;
                case 13: InitCtr<CryptoPP::SHACAL2>(key, keyLen, iv); break;
                default: return false;
            }
        } catch (...) {
            Logger::Log("MITM CTR: EXCEPTION creating alg=%d", algIndex);
            return false;
        }

        m_ready = true;
        if (!quiet)
            Logger::Log("MITM CTR: Init alg=%d(%s) keyLen=%d blockSize=%d",
                         algIndex, ALGORITHMS[algIndex].name, keyLen, blockSize);
        return true;
    }

    void Process(uint8_t* data, int len) {
        if (!m_ready || !m_processFunc || !m_cipherState) return;
        m_processFunc(data, len, m_cipherState);
    }

private:
    // Type-erased process function + cleanup
    using ProcessFn = void(*)(uint8_t* data, int len, void* state);
    using CleanupFn = void(*)(void* state);
    ProcessFn m_processFunc = nullptr;
    CleanupFn m_cleanupFunc = nullptr;
    void* m_cipherState = nullptr;
    int m_algIndex = 0;
    bool m_ready = false;

public:
    ~CtrCipher() {
        if (m_cleanupFunc && m_cipherState) m_cleanupFunc(m_cipherState);
    }
    // Prevent copy (cipher state is unique)
    CtrCipher(const CtrCipher&) = delete;
    CtrCipher& operator=(const CtrCipher&) = delete;
    CtrCipher(CtrCipher&& o) noexcept : m_processFunc(o.m_processFunc), m_cleanupFunc(o.m_cleanupFunc),
        m_cipherState(o.m_cipherState), m_algIndex(o.m_algIndex), m_ready(o.m_ready) {
        o.m_cipherState = nullptr; o.m_ready = false;
    }
    CtrCipher& operator=(CtrCipher&& o) noexcept {
        if (m_cleanupFunc && m_cipherState) m_cleanupFunc(m_cipherState);
        m_processFunc = o.m_processFunc; m_cleanupFunc = o.m_cleanupFunc;
        m_cipherState = o.m_cipherState; m_algIndex = o.m_algIndex; m_ready = o.m_ready;
        o.m_cipherState = nullptr; o.m_ready = false;
        return *this;
    }

private:
    template<typename BlockCipher>
    void InitCtr(const uint8_t* key, int keyLen, const uint8_t* iv) {
        using CtrEnc = typename CryptoPP::CTR_Mode<BlockCipher>::Encryption;

        if (m_cleanupFunc && m_cipherState) m_cleanupFunc(m_cipherState);

        auto* enc = new CtrEnc();
        enc->SetKeyWithIV(key, keyLen, iv);

        m_cipherState = enc;
        m_processFunc = [](uint8_t* data, int len, void* state) {
            static_cast<CtrEnc*>(state)->ProcessData(data, data, len);
        };
        m_cleanupFunc = [](void* state) {
            delete static_cast<CtrEnc*>(state);
        };
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// MITM State
// ═══════════════════════════════════════════════════════════════════════════
struct MitmState {
    std::mutex mtx;
    bool active = false;
    uint64_t activeSocket = 0;  // socket that owns the current MITM session

    // DH keypairs for MITM (separate static + ephemeral)
    Integer mitmServerStaticPriv, mitmServerStaticPub;
    Integer mitmServerEphPriv, mitmServerEphPub;
    Integer mitmClientStaticPriv, mitmClientStaticPub;
    Integer mitmClientEphPriv, mitmClientEphPub;

    // Captured real public keys
    uint8_t serverStaticPub[128];
    uint8_t serverEphPub[128];
    uint8_t clientStaticPub[128];
    uint8_t clientEphPub[128];
    bool gotServerKeys = false;
    bool gotClientKeys = false;
    bool needSharedSecretSearch = false;

    // First encrypted recv bytes (saved for dual-direction brute-force)
    uint8_t firstRecvEnc[4];
    int firstRecvLen = 0;
    bool hasFirstRecv = false;

    // Shared secrets
    uint8_t sharedServer[256];  // shared secret with server
    uint8_t sharedClient[256];  // shared secret with client

    // Four MITM cipher contexts:
    CtrCipher decryptFromServer;  // server's encoder cipher -- to undo server's encryption
    CtrCipher encryptToServer;    // server's decoder cipher -- so server can decrypt our data
    CtrCipher decryptFromClient;  // client's encoder cipher -- to undo client's encryption
    CtrCipher encryptToClient;    // client's decoder cipher -- so client can decrypt our data

    // Spoofed HWID
    char spoofedHwid1[129];
    char spoofedHwid2[129];
    char spoofedHwid3[129];
};

static MitmState g_mitm;
static CRITICAL_SECTION g_mitmCS;
static bool g_mitmCSInit = false;
static uint8_t* g_lastFoundAddr = nullptr;   // cached shared secret address
static uint8_t* g_skipAddr = nullptr;         // skip this addr (was false positive)
static char g_configUsername[32] = {0};       // from NothyrConfig.txt
static int g_configUsernameLen = 0;

static void EnsureCS() {
    if (!g_mitmCSInit) {
        InitializeCriticalSection(&g_mitmCS);
        g_mitmCSInit = true;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Helper: Integer ↔ big-endian byte array (128 bytes)
// ═══════════════════════════════════════════════════════════════════════════
static void IntegerToBytes(const Integer& n, uint8_t* buf, int len) {
    memset(buf, 0, len);
    int nBytes = n.MinEncodedSize();
    int offset = len - nBytes;
    if (offset < 0) offset = 0;
    n.Encode(buf + offset, nBytes);
}

static Integer BytesToInteger(const uint8_t* buf, int len) {
    return Integer(buf, len);
}

// ═══════════════════════════════════════════════════════════════════════════
// Generate DH keypairs for MITM
// ═══════════════════════════════════════════════════════════════════════════
static void GenerateMitmKeys() {
    Integer p(DH_P_HEX);
    Integer g(DH_G_HEX);
    Integer q(DH_Q_HEX);

    AutoSeededRandomPool rng;

    // 4 separate keypairs: static+ephemeral for both client and server sides
    g_mitm.mitmServerStaticPriv = Integer(rng, 2, q - 2);
    g_mitm.mitmServerStaticPub = a_exp_b_mod_c(g, g_mitm.mitmServerStaticPriv, p);
    g_mitm.mitmServerEphPriv = Integer(rng, 2, q - 2);
    g_mitm.mitmServerEphPub = a_exp_b_mod_c(g, g_mitm.mitmServerEphPriv, p);

    g_mitm.mitmClientStaticPriv = Integer(rng, 2, q - 2);
    g_mitm.mitmClientStaticPub = a_exp_b_mod_c(g, g_mitm.mitmClientStaticPriv, p);
    g_mitm.mitmClientEphPriv = Integer(rng, 2, q - 2);
    g_mitm.mitmClientEphPub = a_exp_b_mod_c(g, g_mitm.mitmClientEphPriv, p);

    Logger::Log("MITM: Generated 4 DH keypairs");
}

// ═══════════════════════════════════════════════════════════════════════════
// Compute shared secret and derive cipher keys
// ═══════════════════════════════════════════════════════════════════════════
static void ComputeSharedSecret(const uint8_t* staticPub, const Integer& staticPriv,
                                 const uint8_t* ephPub, const Integer& ephPriv,
                                 uint8_t* sharedOut) {
    Integer p(DH_P_HEX);

    Integer sPub = BytesToInteger(staticPub, 128);
    Integer ePub = BytesToInteger(ephPub, 128);

    Integer staticSecret = a_exp_b_mod_c(sPub, staticPriv, p);
    Integer ephSecret = a_exp_b_mod_c(ePub, ephPriv, p);

    IntegerToBytes(staticSecret, sharedOut, 128);
    IntegerToBytes(ephSecret, sharedOut + 128, 128);
}

static void DeriveKeys(const uint8_t* shared, int sharedLen,
                       CtrCipher& encoder, CtrCipher& decoder, bool clientPolarity) {
    // Algorithm selection -- key0/key1/iv0/iv1 are ALWAYS tied to sel0/sel1
    int hint0 = shared[shared[0] % sharedLen];
    int hint1 = shared[shared[1] % sharedLen];
    int sel0 = hint0 % 14;
    int sel1 = hint1 % 14;

    const auto& alg0 = ALGORITHMS[sel0];
    const auto& alg1 = ALGORITHMS[sel1];

    Logger::Log("MITM: Cipher selection: sel0=%d(%s) sel1=%d(%s) polarity=%s",
                 sel0, alg0.name, sel1, alg1.name,
                 clientPolarity ? "client" : "server");

    // Key derivation -- indices 0/1 match sel0/sel1, NOT encoder/decoder
    uint8_t key0[32], key1[32], iv0[32], iv1[32];

    // key0: first alg0.keyLen bytes
    memcpy(key0, shared, alg0.keyLen);

    // key1: offset = min(alg0.keyLen, 256 - alg1.keyLen)
    int offset1 = alg0.keyLen;
    if (offset1 > sharedLen - alg1.keyLen) offset1 = sharedLen - alg1.keyLen;
    memcpy(key1, shared + offset1, alg1.keyLen);

    // iv0: last alg0.blockSize bytes
    int ivOffset0 = sharedLen - alg0.blockSize;
    memcpy(iv0, shared + ivOffset0, alg0.blockSize);

    // iv1: before iv0
    int ivOffset1 = ivOffset0 - alg1.blockSize;
    if (ivOffset1 < 0) ivOffset1 = 0;
    memcpy(iv1, shared + ivOffset1, alg1.blockSize);

    // Polarity determines which cipher is encoder vs decoder
    // polarity=true (client):  encoder=alg[sel1](key1,iv1), decoder=alg[sel0](key0,iv0)
    // polarity=false (server): encoder=alg[sel0](key0,iv0), decoder=alg[sel1](key1,iv1)
    if (clientPolarity) {
        encoder.Init(sel1, key1, alg1.keyLen, iv1, alg1.blockSize);
        decoder.Init(sel0, key0, alg0.keyLen, iv0, alg0.blockSize);
    } else {
        encoder.Init(sel0, key0, alg0.keyLen, iv0, alg0.blockSize);
        decoder.Init(sel1, key1, alg1.keyLen, iv1, alg1.blockSize);
    }
}

static void SetupCiphers() {
    // PASSIVE MODE: same shared secret, we observe both directions
    // For SEND interception (modify HWID):
    //   - decrypt with client's encoder (polarity=true → encoder=sel1,key1,iv1)
    //   - modify plaintext
    //   - re-encrypt with client's encoder (SAME cipher = XOR twice = no-op... wrong!)
    //
    // Actually in passive mode we need TWO copies of each cipher:
    //   - One to decrypt (advances counter)
    //   - One to re-encrypt (advances counter in sync)
    // Since CTR XOR is its own inverse, decrypt = encrypt with same keystream.
    // So we need matched cipher pairs that stay in sync.

    // Server sends with encoder (polarity=false, sel0). We decrypt with same.
    Logger::Log("MITM: Setting up ciphers (passive mode, same shared secret)...");
    DeriveKeys(g_mitm.sharedServer, 256,
               g_mitm.decryptFromServer,  // server's encoder keystream (for decrypting recv)
               g_mitm.encryptToServer,    // server's decoder keystream (for re-encrypting send to server)
               false);

    // Client sends with encoder (polarity=true, sel1). We decrypt with same.
    DeriveKeys(g_mitm.sharedServer, 256,  // SAME shared secret
               g_mitm.decryptFromClient,  // client's encoder keystream (for decrypting send)
               g_mitm.encryptToClient,    // client's decoder keystream (unused in passive)
               true);

    g_mitm.active = true;
    Logger::Log("MITM: Cipher setup complete -- PASSIVE mode active!");
}

// ═══════════════════════════════════════════════════════════════════════════
// Generate spoofed HWID
// ═══════════════════════════════════════════════════════════════════════════
// Generated once per DLL load, reused across all connections
static bool g_hwidGenerated = false;
static int g_loginPacketsPatched = 0;  // count patched login packets
static bool g_hooksRemoved = false;
static int g_connectionCount = 0;
static volatile bool g_gotShopResponse = false; // set when 0x26 received
static char g_spoofGuid[64] = {0};    // MachineGuid UUID for CG_LOGIN3
static char g_spoofMac[24] = {0};     // MAC address for CG_LOGIN2
static char g_spoofGuid2[64] = {0};   // second UUID for CG_LOGIN3 hwid3

static void GenerateSpoofedHwid() {
    if (g_hwidGenerated) {
        // Reuse existing -- just copy into current MITM state
        strncpy(g_mitm.spoofedHwid1, g_spoofGuid, sizeof(g_mitm.spoofedHwid1));
        strncpy(g_mitm.spoofedHwid2, g_spoofMac, sizeof(g_mitm.spoofedHwid2));
        strncpy(g_mitm.spoofedHwid3, g_spoofGuid2, sizeof(g_mitm.spoofedHwid3));
        Logger::Log("MITM HWID: reusing guid='%s' mac='%s'", g_spoofGuid, g_spoofMac);
        return;
    }

    static const char hex[] = "0123456789abcdef";
    AutoSeededRandomPool rng;

    auto randHex = [&](char* buf, int len) {
        for (int i = 0; i < len; i++)
            buf[i] = hex[rng.GenerateByte() % 16];
        buf[len] = '\0';
    };

    // MachineGuid format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    char a[9], b[5], c[5], d[5], e[13];
    randHex(a, 8); randHex(b, 4); randHex(c, 4); randHex(d, 4); randHex(e, 12);
    snprintf(g_spoofGuid, sizeof(g_spoofGuid), "%s-%s-%s-%s-%s", a, b, c, d, e);

    // MAC address: XX:XX:XX:XX:XX:XX
    uint8_t mac[6];
    rng.GenerateBlock(mac, 6);
    mac[0] = (mac[0] & 0xFE) | 0x02;
    snprintf(g_spoofMac, sizeof(g_spoofMac), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // Second UUID for hwid3
    char a2[9], b2[5], c2[5], d2[5], e2[13];
    randHex(a2, 8); randHex(b2, 4); randHex(c2, 4); randHex(d2, 4); randHex(e2, 12);
    snprintf(g_spoofGuid2, sizeof(g_spoofGuid2), "%s-%s-%s-%s-%s", a2, b2, c2, d2, e2);

    strncpy(g_mitm.spoofedHwid1, g_spoofGuid, sizeof(g_mitm.spoofedHwid1));
    strncpy(g_mitm.spoofedHwid2, g_spoofMac, sizeof(g_mitm.spoofedHwid2));
    strncpy(g_mitm.spoofedHwid3, g_spoofGuid2, sizeof(g_mitm.spoofedHwid3));

    g_hwidGenerated = true;
    Logger::Log("MITM HWID: guid='%s' mac='%s' guid2='%s'", g_spoofGuid, g_spoofMac, g_spoofGuid2);
}

// ═══════════════════════════════════════════════════════════════════════════
// Patch HWID in plaintext login packets
// ═══════════════════════════════════════════════════════════════════════════
static bool PatchHwidInPlaintext(uint8_t* data, int len) {
    uint8_t hdr = data[0];

    // CG_LOGIN3 (0x6F) -- 468 bytes
    // Real client: HWID1=MachineGuid, HWID2=MAC, HWID3=GUID2
    if (hdr == 0x6F && len >= 468) {
        Logger::Log("MITM: Patching CG_LOGIN3 HWID fields");
        Logger::Log("MITM:   original hwid1: '%.30s'", data + 0x48);

        memset(data + 0x48, 0, 128);
        memset(data + 0xC9, 0, 128);
        memset(data + 0x14A, 0, 128);
        memcpy(data + 0x48,  g_spoofGuid, strlen(g_spoofGuid));   // HWID1 = GUID
        memcpy(data + 0xC9,  g_spoofMac, strlen(g_spoofMac));     // HWID2 = MAC
        memcpy(data + 0x14A, g_spoofGuid2, strlen(g_spoofGuid2)); // HWID3 = GUID2

        Logger::Log("MITM:   spoofed: guid='%s' mac='%s'", g_spoofGuid, g_spoofMac);
        g_loginPacketsPatched++;
        // beep removed
        return true;
    }

    // CG_LOGIN2 (0x6D) -- 308 bytes
    // Real client: HWID1=MAC address, HWID2=MachineGuid
    if (hdr == 0x6D && len >= 308) {
        Logger::Log("MITM: Patching CG_LOGIN2 HWID fields");
        Logger::Log("MITM:   original hwid1: '%.30s'", data + 52);

        memset(data + 52, 0, 128);
        memset(data + 180, 0, 128);
        memcpy(data + 52,  g_spoofMac, strlen(g_spoofMac));      // HWID1 = MAC
        memcpy(data + 180, g_spoofGuid, strlen(g_spoofGuid));    // HWID2 = GUID

        Logger::Log("MITM:   spoofed hwid1:  '%s'", g_spoofMac);
        g_loginPacketsPatched++;
        // beep removed
        return true;
    }

    return false;
}

bool Mt2Mitm::ShouldUnhook() {
    // After 3 login packets (auth + GS1 + GS2), all HWIDs are spoofed
    return g_loginPacketsPatched >= 3 && !g_hooksRemoved;
}

void Mt2Mitm::MarkUnhooked() {
    g_hooksRemoved = true;
    g_mitm.active = false;
    Logger::Log("MITM: All %d login packets patched -- hooks can be removed", g_loginPacketsPatched);
}

// ═══════════════════════════════════════════════════════════════════════════
// Public API
// ═══════════════════════════════════════════════════════════════════════════

bool Mt2Mitm::OnRecv(uint64_t sock, char* buf, int len) {
    EnsureCS();
    if (len < 1) return false;
    uint8_t hdr = (uint8_t)buf[0];

    // When cipher is active, ALL data must go through decrypt/encrypt
    // Do NOT check for 0xFB/0xFA on encrypted bytes -- they'd be false matches!
    if (g_mitm.active && sock == g_mitm.activeSocket)
        goto cipher_processing;

    // Intercept GC_KEY_AGREEMENT (0xFB) -- server's DH public keys (ONLY before cipher active)
    if (hdr == 0xFB && len == 261) {
        std::lock_guard<std::mutex> lk(g_mitm.mtx);
        g_mitm.activeSocket = sock;  // this socket owns the MITM session

        // Format: [0xFB][agreedLen:2LE][dataLen:2LE][staticPub:128][ephPub:128]
        memcpy(g_mitm.serverStaticPub, buf + 5, 128);
        memcpy(g_mitm.serverEphPub, buf + 133, 128);
        g_mitm.gotServerKeys = true;

        Logger::Log("MITM: Captured server DH public keys (261 bytes)");
        Logger::Log("MITM: serverStatic[0:8]=%02X%02X%02X%02X%02X%02X%02X%02X",
            buf[5],buf[6],buf[7],buf[8],buf[9],buf[10],buf[11],buf[12]);
        Logger::Log("MITM: serverEph[0:8]=%02X%02X%02X%02X%02X%02X%02X%02X",
            buf[133],buf[134],buf[135],buf[136],buf[137],buf[138],buf[139],buf[140]);

        // Keys should be pre-generated at init, but generate if missing
        if (g_mitm.mitmClientStaticPub.IsZero()) {
            Logger::Log("MITM: WARNING - generating keys on-the-fly (should be pre-generated)");
            GenerateMitmKeys();
            GenerateSpoofedHwid();
        }

        // PASSIVE MODE: don't replace keys, just observe
        // We'll find the client's private key in memory later
        Logger::Log("MITM: Passive mode -- server keys captured, not modified");
        return false; // don't modify recv buffer
    }

    // On 0xFA (key agreement completed), setup ciphers if we have the shared secret
    if (hdr == 0xFA && len >= 1) {
        std::lock_guard<std::mutex> lk(g_mitm.mtx);
        if (g_mitm.gotServerKeys && g_mitm.gotClientKeys && !g_mitm.active) {
            if (!g_mitm.needSharedSecretSearch) {
                // We already computed the real shared secret from private keys!
                SetupCiphers();
                Logger::Log("MITM: 0xFA received -- ciphers ACTIVE with REAL shared secret!");
                // beep removed
            } else {
                // No real shared secret, use dummy (XOR cancel) mode
                g_mitm.hasFirstRecv = false;
                g_mitm.firstRecvLen = 0;
                Logger::Log("MITM: 0xFA received -- will use dummy mode on first send");
            }
        }
        return false;
    }

    // Save first encrypted recv bytes for dual-direction brute-force (triggered on first send)
    if (g_mitm.needSharedSecretSearch && sock == g_mitm.activeSocket && len >= 2 && !g_mitm.hasFirstRecv) {
        int copyLen = len < 4 ? len : 4;
        memcpy(g_mitm.firstRecvEnc, buf, copyLen);
        g_mitm.firstRecvLen = copyLen;
        g_mitm.hasFirstRecv = true;
        Logger::Log("MITM: Saved first encrypted recv: %02X %02X (len=%d), waiting for first send to brute-force",
                     (uint8_t)buf[0], (uint8_t)buf[1], len);
        return false;
    }

    // PASSIVE MODE: do NOT modify recv buffer -- client decrypts itself
    // We don't touch the buffer at all. No decryptFromServer, no encryptToClient.
    // The client's own cipher handles all recv decryption.
cipher_processing:
    if (g_mitm.active && sock == g_mitm.activeSocket) {
        // Just return false -- let the data pass through untouched to the client
        return false;
    }

    return false;
}

bool Mt2Mitm::OnSend(uint64_t sock, char* buf, int len) {
    EnsureCS();
    if (len < 1) return false;
    uint8_t hdr = (uint8_t)buf[0];

    // When cipher is active, ALL data must go through decrypt/encrypt
    if (g_mitm.active && sock == g_mitm.activeSocket)
        goto send_cipher_processing;

    // Intercept CG_KEY_AGREEMENT (0xFB) -- client's DH public keys (ONLY before cipher active)
    if (hdr == 0xFB && len == 261) {
        std::lock_guard<std::mutex> lk(g_mitm.mtx);

        memcpy(g_mitm.clientStaticPub, buf + 5, 128);
        memcpy(g_mitm.clientEphPub, buf + 133, 128);
        g_mitm.gotClientKeys = true;

        Logger::Log("MITM: Captured client DH public keys (261 bytes)");

        // PASSIVE MODE: find client's PRIVATE keys by scanning memory near pub keys
        // Private key is 20 bytes (RFC 5114 2.1, 160-bit subgroup)
        // Verify: g^priv mod p == known_pub_key
        if (g_mitm.gotServerKeys) {
            Logger::Log("MITM: Searching for client private keys (pointer method)...");
            Integer p(DH_P_HEX);
            Integer g(DH_G_HEX);
            Integer q(DH_Q_HEX);
            Integer clientStaticPubInt = BytesToInteger(g_mitm.clientStaticPub, 128);
            Integer clientEphPubInt = BytesToInteger(g_mitm.clientEphPub, 128);
            bool foundStatic = false, foundEph = false;

            // Step 1: find pub key DATA in memory, remember the address
            uint8_t* staticPubAddr = nullptr;
            {
                SYSTEM_INFO si; GetSystemInfo(&si);
                MEMORY_BASIC_INFORMATION mbi;
                uint8_t* addr = (uint8_t*)si.lpMinimumApplicationAddress;
                while (addr < (uint8_t*)si.lpMaximumApplicationAddress && !staticPubAddr) {
                    if (VirtualQuery(addr, &mbi, sizeof(mbi)) == 0) break;
                    if (mbi.State == MEM_COMMIT && mbi.RegionSize >= 256 &&
                        (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READWRITE)) {
                        uint8_t* base = (uint8_t*)mbi.BaseAddress;
                        size_t rSize = mbi.RegionSize;
                        try {
                            for (size_t i = 0; i + 128 <= rSize; i++) {
                                if (memcmp(base+i, g_mitm.clientStaticPub, 16)==0 &&
                                    memcmp(base+i, g_mitm.clientStaticPub, 128)==0) {
                                    staticPubAddr = base + i;
                                    Logger::Log("MITM: Static pub data at %p", staticPubAddr);
                                    break;
                                }
                            }
                        } catch(...) {}
                    }
                    addr = (uint8_t*)mbi.BaseAddress + mbi.RegionSize;
                }
            }

            // Step 2: search all RW memory for pointers TO pub key data address
            // Then check nearby for DH struct with private key
            if (staticPubAddr) {
                uint32_t targetPtr = (uint32_t)(uintptr_t)staticPubAddr;
                Logger::Log("MITM: Searching for pointers to %p...", staticPubAddr);

                SYSTEM_INFO si; GetSystemInfo(&si);
                MEMORY_BASIC_INFORMATION mbi;
                uint8_t* addr = (uint8_t*)si.lpMinimumApplicationAddress;

                while (addr < (uint8_t*)si.lpMaximumApplicationAddress && (!foundStatic || !foundEph)) {
                    if (VirtualQuery(addr, &mbi, sizeof(mbi)) == 0) break;
                    if (mbi.State == MEM_COMMIT && mbi.RegionSize >= 32 &&
                        (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READWRITE)) {
                        uint8_t* base = (uint8_t*)mbi.BaseAddress;
                        size_t rSize = mbi.RegionSize;
                        try {
                            for (size_t i = 0; i + 4 <= rSize; i += 4) {
                                uint32_t val = *(uint32_t*)(base + i);
                                if (val != targetPtr) continue;

                                // Found pointer to pub key! This is likely Integer::reg
                                // CryptoPP Integer: [reg_ptr:4][regSize:4][sign:4]
                                // The DH struct has: [privKey Integer][pubKey Integer]
                                // So privKey is 12 bytes BEFORE this pointer (or at various offsets)
                                Logger::Log("MITM: Ptr to static pub at %p (offset %d in region)",
                                             base+i, (int)i);

                                // Search ±64 bytes for Integer objects containing priv key
                                int searchStart = (i >= 64) ? (int)i - 64 : 0;
                                int searchEnd = (i + 64 < rSize - 4) ? (int)i + 64 : (int)rSize - 4;
                                for (int j = searchStart; j < searchEnd && !foundStatic; j += 4) {
                                    // This could be a pointer to priv key data
                                    uint32_t privDataPtr = *(uint32_t*)(base + j);
                                    if (privDataPtr < 0x10000 || privDataPtr > 0x7FFFFFFF) continue;

                                    // Try to read 20 bytes from that address
                                    MEMORY_BASIC_INFORMATION mbiPriv;
                                    if (VirtualQuery((void*)privDataPtr, &mbiPriv, sizeof(mbiPriv)) == 0) continue;
                                    if (mbiPriv.State != MEM_COMMIT) continue;
                                    if ((uintptr_t)privDataPtr + 20 > (uintptr_t)mbiPriv.BaseAddress + mbiPriv.RegionSize) continue;

                                    try {
                                        uint8_t* privData = (uint8_t*)(uintptr_t)privDataPtr;
                                        Integer candidate(privData, 20);
                                        if (candidate <= 1 || candidate >= q) continue;
                                        Integer computed = a_exp_b_mod_c(g, candidate, p);
                                        if (computed == clientStaticPubInt) {
                                            g_mitm.mitmClientStaticPriv = candidate;
                                            foundStatic = true;
                                            Logger::Log("MITM: FOUND static priv key! ptr=%p data=%p", base+j, privData);
                                        } else if (!foundEph && computed == clientEphPubInt) {
                                            g_mitm.mitmClientEphPriv = candidate;
                                            foundEph = true;
                                            Logger::Log("MITM: FOUND eph priv key! ptr=%p data=%p", base+j, privData);
                                        }
                                    } catch(...) {}
                                }
                            }
                        } catch(...) {}
                    }
                    addr = (uint8_t*)mbi.BaseAddress + mbi.RegionSize;
                }
                Logger::Log("MITM: Pointer search done (static=%d eph=%d)", foundStatic, foundEph);
            }

            // Step 3: if we found static but not eph, try modexp on heap for eph
            // The eph priv key is likely near the static priv key
            if (foundStatic && !foundEph) {
                Logger::Log("MITM: Searching heap for eph private key...");
                SYSTEM_INFO si; GetSystemInfo(&si);
                MEMORY_BASIC_INFORMATION mbi;
                uint8_t* addr = (uint8_t*)0x00010000;
                while (addr < (uint8_t*)0x10000000 && !foundEph) {
                    if (VirtualQuery(addr, &mbi, sizeof(mbi)) == 0) break;
                    if (mbi.State == MEM_COMMIT && mbi.RegionSize >= 20 &&
                        (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READWRITE)) {
                        uint8_t* base = (uint8_t*)mbi.BaseAddress;
                        try {
                            for (size_t off = 0; off + 20 <= mbi.RegionSize && !foundEph; off += 4) {
                                Integer candidate(base + off, 20);
                                if (candidate <= 1 || candidate >= q) continue;
                                Integer computed = a_exp_b_mod_c(g, candidate, p);
                                if (computed == clientEphPubInt) {
                                    g_mitm.mitmClientEphPriv = candidate;
                                    foundEph = true;
                                    Logger::Log("MITM: FOUND eph priv key at %p!", base + off);
                                }
                            }
                        } catch(...) {}
                    }
                    addr = (uint8_t*)mbi.BaseAddress + mbi.RegionSize;
                }
            }

            if (foundStatic && foundEph) {
                Logger::Log("MITM: Both private keys found! Computing shared secret...");
                ComputeSharedSecret(g_mitm.serverStaticPub, g_mitm.mitmClientStaticPriv,
                                     g_mitm.serverEphPub, g_mitm.mitmClientEphPriv,
                                     g_mitm.sharedServer);
                memcpy(g_mitm.sharedClient, g_mitm.sharedServer, 256);
                g_mitm.needSharedSecretSearch = false;
                Logger::Log("MITM: REAL shared secret computed!");
            } else {
                Logger::Log("MITM: Private keys NOT found (static=%d eph=%d) -- dummy mode",
                             foundStatic, foundEph);
                g_mitm.needSharedSecretSearch = true;
            }
        }

        return true;
    }

    // DUMMY CIPHER ACTIVATION: use random shared secret -- XOR cancel ensures
    // non-HWID bytes pass through correctly, HWID becomes random (not real = safe)
    if (g_mitm.needSharedSecretSearch && g_mitm.hasFirstRecv && sock == g_mitm.activeSocket) {
        Logger::Log("MITM: Activating with dummy shared secret (XOR cancel mode)");

        // Generate random 256-byte dummy shared secret
        AutoSeededRandomPool rng;
        rng.GenerateBlock(g_mitm.sharedServer, 256);
        memcpy(g_mitm.sharedClient, g_mitm.sharedServer, 256);

        SetupCiphers();
        g_mitm.needSharedSecretSearch = false;
        Logger::Log("MITM: ACTIVE -- random HWID mode (real HWID never sent)");
        // beep removed
    }

    // After cipher is active, decrypt from client, modify HWID, re-encrypt for server
send_cipher_processing:
    if (g_mitm.active && sock == g_mitm.activeSocket) {
        EnterCriticalSection(&g_mitmCS);
        if (g_mitm.active) {
            g_mitm.decryptFromClient.Process((uint8_t*)buf, len);

            uint8_t plainHdr = (uint8_t)buf[0];

            static bool g_beeped = false;
            if (plainHdr == 0x6F && len >= 468) {
                Logger::Log("LOGIN3 name='%.10s' lang=%d", buf+1, (uint8_t)buf[0x1D3]);
                PatchHwidInPlaintext((uint8_t*)buf, len);
                if (!g_beeped) { Beep(800, 100); g_beeped = true; }
            } else if (plainHdr == 0x6D && len >= 308) {
                Logger::Log("LOGIN2 name='%.10s'", buf+1);
                PatchHwidInPlaintext((uint8_t*)buf, len);
            } else if (len == 468 || len == 308) {
                // DUMMY MODE: header is garbage but we MUST still patch HWID
                // to prevent real HWID from passing through via XOR cancel
                Logger::Log("MITM: Force-patching HWID (dummy mode, len=%d, hdr=0x%02X)", len, plainHdr);
                if (len == 468) {
                    // CG_LOGIN3 HWID offsets: 0x48 (128B), 0xC9 (128B), 0x14A (128B)
                    memset(buf + 0x48, 0, 128);
                    memset(buf + 0xC9, 0, 128);
                    memset(buf + 0x14A, 0, 128);
                    memcpy(buf + 0x48,  g_spoofGuid, strlen(g_spoofGuid));
                    memcpy(buf + 0xC9,  g_spoofMac, strlen(g_spoofMac));
                    memcpy(buf + 0x14A, g_spoofGuid2, strlen(g_spoofGuid2));
                    if (!g_beeped) { Beep(800, 100); g_beeped = true; }
                } else {
                    // CG_LOGIN2 HWID offsets: 52 (128B), 180 (128B)
                    memset(buf + 52, 0, 128);
                    memset(buf + 180, 0, 128);
                    memcpy(buf + 52,  g_spoofMac, strlen(g_spoofMac));
                    memcpy(buf + 180, g_spoofGuid, strlen(g_spoofGuid));
                    // beep removed
                }
            }

            // Log non-move/pong packets
            if (plainHdr != 0x07 && plainHdr != 0xFE) {
                char hex[200] = {0};
                int pos = 0;
                int dumpLen = len < 64 ? len : 64;
                for (int i = 0; i < dumpLen && pos < 190; i++)
                    pos += snprintf(hex + pos, sizeof(hex) - pos, "%02X ", (uint8_t)buf[i]);
                Logger::Log("PKT> [%d] %s", len, hex);
            }

            g_mitm.encryptToServer.Process((uint8_t*)buf, len);
        }
        LeaveCriticalSection(&g_mitmCS);

        return true;
    }

    return false;
}

extern bool g_injectMode; // defined in hooks_net.cpp

bool Mt2Mitm::InjectPacket(const uint8_t* plaintext, int len) {
    EnsureCS();
    if (!g_mitm.active || !g_mitm.activeSocket || len <= 0 || len > 4096) return false;

    static int (WSAAPI* realSend)(SOCKET, const char*, int, int) = nullptr;
    if (!realSend) {
        HMODULE ws2 = GetModuleHandleA("ws2_32.dll");
        if (ws2) realSend = (int(WSAAPI*)(SOCKET,const char*,int,int))GetProcAddress(ws2, "send");
        if (!realSend) return false;
    }

    uint8_t buf[4096];
    memcpy(buf, plaintext, len);

    EnterCriticalSection(&g_mitmCS);
    if (!g_mitm.active) { LeaveCriticalSection(&g_mitmCS); return false; }
    g_mitm.encryptToServer.Process(buf, len);
    LeaveCriticalSection(&g_mitmCS);

    g_injectMode = true;
    int ret = realSend((SOCKET)g_mitm.activeSocket, (const char*)buf, len, 0);
    g_injectMode = false;

    Logger::Log("InjectPacket: %d bytes (ret=%d)", len, ret);
    return ret > 0;
}

bool Mt2Mitm::PopShopResponse() {
    if (g_gotShopResponse) { g_gotShopResponse = false; return true; }
    return false;
}

void Mt2Mitm::PreGenerateKeys() {
    GenerateMitmKeys();
    GenerateSpoofedHwid();

    /* NothyrConfig.txt username disabled -- not needed for dummy/private-key mode
    FILE* cfg = fopen("NothyrConfig.txt", "r");
    if (cfg) {
        char line[64] = {0};
        if (fgets(line, sizeof(line), cfg)) {
            char* nl = strchr(line, '\n'); if (nl) *nl = 0;
            nl = strchr(line, '\r'); if (nl) *nl = 0;
            int len = (int)strlen(line);
            if (len > 0 && len < 31) {
                memcpy(g_configUsername, line, len);
                g_configUsernameLen = len;
                Logger::Log("MITM: Config username='%s' (%d chars)", g_configUsername, len);
            }
        }
        fclose(cfg);
    }
    */

    Logger::Log("MITM: Keys pre-generated");
}

bool Mt2Mitm::IsActiveFor(uint64_t sock) {
    return g_mitm.active && (sock == 0 || sock == g_mitm.activeSocket);
}

void Mt2Mitm::Reset() {
    EnsureCS();
    EnterCriticalSection(&g_mitmCS);
    g_mitm.active = false;
    g_mitm.activeSocket = 0;
    g_mitm.gotServerKeys = false;
    g_mitm.gotClientKeys = false;
    g_mitm.needSharedSecretSearch = false;
    g_mitm.hasFirstRecv = false;
    g_mitm.firstRecvLen = 0;
    // Clear all keys so they regenerate on next connection
    g_mitm.mitmServerStaticPriv = Integer::Zero();
    g_mitm.mitmServerStaticPub = Integer::Zero();
    g_mitm.mitmServerEphPriv = Integer::Zero();
    g_mitm.mitmServerEphPub = Integer::Zero();
    g_mitm.mitmClientStaticPriv = Integer::Zero();
    g_mitm.mitmClientStaticPub = Integer::Zero();
    g_mitm.mitmClientEphPriv = Integer::Zero();
    g_mitm.mitmClientEphPub = Integer::Zero();
    memset(g_mitm.sharedServer, 0, sizeof(g_mitm.sharedServer));
    memset(g_mitm.sharedClient, 0, sizeof(g_mitm.sharedClient));
    LeaveCriticalSection(&g_mitmCS);
    Logger::Log("MITM: Reset");
}
