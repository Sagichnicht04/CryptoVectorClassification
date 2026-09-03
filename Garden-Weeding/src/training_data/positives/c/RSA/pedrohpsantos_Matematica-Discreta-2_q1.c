/**
 * @description
 * Este programa simula as três fases principais do algoritmo RSA:
 * 1. FATORAÇÃO: Encontra os primos 'p' e 'q' a partir de números compostos
 * usando o método Rho de Pollard.
 * 2. GERAÇÃO DE CHAVES: Cria as chaves pública e privada a partir de 'p' e 'q'.
 * 3. CRIPTOGRAFIA: Criptografa e descriptografa uma mensagem fornecida pelo
 * usuário.
 *
 * A interface foi estilizada para simular um terminal de operações de um agente
 * de criptografia.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definição para usar long long para cálculos maiores
typedef long long ll;

// --- Funções de Utilitário e Aritmética Modular ---
ll mdc(ll a, ll b);
ll mdc_sem_print(ll a, ll b);
ll estendido_euclides(ll a, ll b, ll *x, ll *y);
ll estendido_euclides_sem_print(ll a, ll b, ll *x, ll *y);
ll inverso_modular(ll a, ll m);
ll inverso_modular_sem_print(ll a, ll m);
ll powMod(ll base, ll exp, ll mod);
ll mulmod(ll a, ll b, ll mod);
int is_prime(ll n);
void limpar_buffer();

// --- Funções das Etapas do Protocolo RSA ---
ll pollard_rho(ll n);
void run_fase1_fatoracao(ll *p_out, ll *q_out);
void run_fase2_geracao_chaves(ll p, ll q);
void run_fase3_criptografia(ll n, ll e, ll d, ll p, ll q);

//              FUNÇÃO PRINCIPAL: ORQUESTRADOR DE MISSÃO

int main(void) {
  ll p_rsa, q_rsa;
  ll n_rsa, z_rsa, e_rsa, d_rsa;
  printf("CRYPTOAGENT RSA PROTOCOL SIMULATOR v2.0\n");
  printf("[SYSTEM] Inicializando protocolos...\n");

  // --- FASE 1: Obter fatores primos ---
  run_fase1_fatoracao(&p_rsa, &q_rsa);

  // --- FASE 2: Gerar chaves a partir dos fatores ---
  run_fase2_geracao_chaves(p_rsa, q_rsa);

  // --- Preparação para a Fase 3 (recalcula chaves sem prints para não poluir)
  // ---
  n_rsa = p_rsa * q_rsa;
  z_rsa = (p_rsa - 1) * (q_rsa - 1);
  e_rsa = 2;
  while (e_rsa < z_rsa && mdc_sem_print(e_rsa, z_rsa) != 1) {
    e_rsa++;
  }
  if (e_rsa >= z_rsa) {
    e_rsa = 5;
  } // Fallback
  d_rsa = inverso_modular_sem_print(e_rsa, z_rsa);

  // --- FASE 3: Executar criptografia e descriptografia ---
  if (d_rsa != -1) {
    run_fase3_criptografia(n_rsa, e_rsa, d_rsa, p_rsa, q_rsa);
  } else {
    printf("\n[FATAL ERROR] Falha ao gerar expoente privado 'd'. Missão "
           "abortada.\n");
  }

  printf("\n[SYSTEM] Todos os protocolos foram concluídos. Encerrando.\n\n");
  return 0;
}

//                 FASE 1: FATORAÇÃO DE ALVOS

void run_fase1_fatoracao(ll *p_out, ll *q_out) {
  ll n1, n2;
  ll p = 0, q = 0;

  printf("\nFASE 1: FATORAÇÃO DE ALVOS\n");
  printf("[INFO] Requer dois números compostos (N1, N2) para extrair os "
         "fatores primos 'p' e 'q'.\n");

  do {
    printf("\n[INPUT] Insira o alvo N1 (composto, 3-4 dígitos): ");
    if (scanf("%lld", &n1) != 1) {
      limpar_buffer();
      n1 = 0;
    }

    printf(
        "[INPUT] Insira o alvo N2 (composto, 3-4 dígitos, diferente de N1): ");
    if (scanf("%lld", &n2) != 1) {
      limpar_buffer();
      n2 = 0;
    }
    limpar_buffer();

    if (n1 < 100 || n1 > 9999 || n2 < 100 || n2 > 9999 || n1 == n2) {
      printf("[ERROR] Parâmetros inválidos. Tente novamente.\n");
      continue;
    }
    if (is_prime(n1) || is_prime(n2)) {
      printf("[WARNING] Um dos alvos é primo. O protocolo requer números "
             "compostos.\n");
    }

    p = pollard_rho(n1);
    q = pollard_rho(n2);

    // Lógica de correção para garantir que p e q sejam os fatores primos
    // corretos
    if (p > 1 && p != n1) {
      ll p_other = n1 / p;
      if (!is_prime(p) && is_prime(p_other))
        p = p_other;
    } else {
      p = 0;
    }

    if (q > 1 && q != n2) {
      ll q_other = n2 / q;
      if (!is_prime(q) && is_prime(q_other))
        q = q_other;
    } else {
      q = 0;
    }

    if (p <= 1 || q <= 1 || p == q || !is_prime(p) || !is_prime(q)) {
      printf("\n[ERROR] Fatores extraídos (p=%lld, q=%lld) são inválidos. "
             "Repetindo a operação com novos alvos.\n",
             p, q);
      continue;
    }

    break;
  } while (1);

  printf("\n[SUCCESS] Fatores primos adquiridos: p = %lld, q = %lld\n", p, q);
  *p_out = p;
  *q_out = q;
}

ll pollard_rho(ll n) {
  ll x = 2, y = 2, d = 1, i = 0;
  printf("\n  -> [SUB-ROUTINE] Iniciando Rho de Pollard em N = %lld\n", n);
  if (n % 2 == 0)
    return 2;

  while (d == 1) {
    x = (mulmod(x, x, n) + 1) % n;
    y = (mulmod(y, y, n) + 1) % n;
    y = (mulmod(y, y, n) + 1) % n;

    d = mdc_sem_print(llabs(x - y), n);
    i++;

    printf("     [Ciclo %lld] x = %-5lld | y = %-5lld | d = gcd(|x-y|, N) = "
           "%lld\n",
           i, x, y, d);

    if (d != 1 && d != n) {
      printf("  -> [SUB-ROUTINE] Fator não-trivial encontrado: %lld\n", d);
      return d;
    }
    if (d == n || i > 2000) { // Limite para evitar loop infinito
      printf("  -> [SUB-ROUTINE] Falha na fatoração. O alvo pode ser primo ou "
             "muito complexo.\n");
      return -1;
    }
  }
  return d;
}

//                FASE 2: GERAÇÃO DO PAR DE CHAVES RSA

void run_fase2_geracao_chaves(ll p, ll q) {
  ll n, z, e, d;

  printf("\nFASE 2: GERAÇÃO DE CHAVES RSA\n");
  printf("[INFO] Usando p=%lld e q=%lld para construir as chaves pública e "
         "privada.\n\n",
         p, q);

  // Passo 1: Calcular n e z
  n = p * q;
  z = (p - 1) * (q - 1);
  printf("  [Passo 1] Calculando parâmetros base:\n");
  printf("    -> Módulo (n = p*q): %lld * %lld = %lld\n", p, q, n);
  printf("    -> Totiente (z = (p-1)*(q-1)): %lld * %lld = %lld\n\n", p - 1,
         q - 1, z);

  // Passo 2: Encontrar expoente público 'e'
  e = 2;
  printf("  [Passo 2] Selecionando expoente público 'e' (1 < e < z e "
         "mdc(e,z)=1):\n");
  while (e < z) {
    if (mdc_sem_print(e, z) == 1) {
      printf("    -> Candidato 'e' encontrado: %lld\n\n", e);
      break;
    }
    e++;
  }
  if (e >= z) {
    e = 5;
  } // Fallback

  // Passo 3: Calcular expoente privado 'd'
  printf("  [Passo 3] Calculando expoente privado 'd' (d * e = 1 mod z):\n");
  d = inverso_modular(e, z);

  // Exibição final das chaves

  printf("GERAÇÃO DE CHAVES CONCLUÍDA\n");
  printf("CHAVE PÚBLICA: (n=%-8lld, e=%-5lld)\n", n, e);
  printf("CHAVE PRIVADA: (n=%-8lld, d=%-5lld)\n", n, d);
}

//              FASE 3: PROTOCOLO DE CRIPTO/DECRIPTO

void run_fase3_criptografia(ll n, ll e, ll d, ll p, ll q) {
  char msg_input[100];
  char msg_original[100] = "";

  printf("\nFASE 3: CRIPTOGRAFIA E DESCRIPTOGRAFIA\n");
  printf("[INPUT] Insira a mensagem para transmissão (A-Z, espaço): ");
  if (scanf(" %99[^\n]", msg_input) != 1) {
    printf("[WARNING] Nenhuma mensagem lida. Abortando fase 3.\n");
    return;
  }

  // Normaliza a mensagem (apenas maiúsculas e espaços)
  int len_input = strlen(msg_input);
  int k = 0;
  for (int i = 0; i < len_input; i++) {
    char c = toupper((unsigned char)msg_input[i]);
    if ((c >= 'A' && c <= 'Z') || c == ' ') {
      msg_original[k++] = c;
    }
  }
  msg_original[k] = '\0';
  int len = k;

  if (len == 0) {
    printf("[WARNING] Mensagem vazia após normalização.\n");
    return;
  }

  ll *m_blocos = (ll *)malloc(len * sizeof(ll));
  ll *c_blocos = (ll *)malloc(len * sizeof(ll));
  ll *m_decifrados = (ll *)malloc(len * sizeof(ll));

  // 1. Pré-Codificação
  printf(
      "\n  [Passo 1] Pré-codificação da mensagem '%s' para blocos numéricos:\n",
      msg_original);
  printf("    -> Blocos M: ");
  for (int i = 0; i < len; i++) {
    if (msg_original[i] == ' ')
      m_blocos[i] = 0;
    else
      m_blocos[i] = msg_original[i] - 'A' + 11;
    printf("%02lld ", m_blocos[i]);
  }
  printf("\n");

  // 2. Criptografia
  printf("\n  [Passo 2] Criptografando blocos (C = M^%lld mod %lld):\n", e, n);
  for (int i = 0; i < len; i++) {
    printf("    -> Criptografando M=%lld...", m_blocos[i]);
    c_blocos[i] = powMod(m_blocos[i], e, n);
    printf(" C=%lld\n", c_blocos[i]);
  }
  printf("\n    [TRANSMISSÃO] Mensagem Cifrada: ");
  for (int i = 0; i < len; i++)
    printf("%lld ", c_blocos[i]);
  printf("\n");

  // 3. Descriptografia
  printf("\n  [Passo 3] Descriptografando blocos (M = C^%lld mod %lld):\n", d,
         n);
  for (int i = 0; i < len; i++) {
    printf("    -> Descriptografando C=%lld...", c_blocos[i]);
    m_decifrados[i] = powMod(c_blocos[i], d, n);
    printf(" M=%lld\n", m_decifrados[i]);
  }

  // 4. Reversão e Verificação
  printf("\n  [Passo 4] Verificação final e reversão para texto:\n");
  printf("    -> Mensagem Decifrada: ");
  int sucesso = 1;
  for (int i = 0; i < len; i++) {
    char c;
    if (m_decifrados[i] == 0)
      c = ' ';
    else if (m_decifrados[i] >= 11 && m_decifrados[i] <= 36)
      c = (char)('A' + m_decifrados[i] - 11);
    else {
      c = '?';
      sucesso = 0;
    }
    printf("%c", c);
    if (m_decifrados[i] != m_blocos[i])
      sucesso = 0;
  }
  printf("\n");

  if (sucesso) {
    printf("\n[VERIFIED] SUCESSO! A mensagem original e a decifrada são "
           "idênticas.\n");
  } else {
    printf("\n[VERIFIED] FALHA! Houve uma divergência na descriptografia.\n");
  }

  free(m_blocos);
  free(c_blocos);
  free(m_decifrados);
}

//                FUNÇÕES DE ARITMÉTICA E UTILITÁRIOS

// Limpa o buffer de entrada (stdin)
void limpar_buffer() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF) {
  }
}

// Verifica se um número é primo (otimizado)
int is_prime(ll n) {
  if (n <= 1)
    return 0;
  if (n <= 3)
    return 1;
  if (n % 2 == 0 || n % 3 == 0)
    return 0;
  for (ll i = 5; i * i <= n; i = i + 6)
    if (n % i == 0 || n % (i + 2) == 0)
      return 0;
  return 1;
}

// Multiplicação modular segura para números grandes
ll mulmod(ll a, ll b, ll mod) {
  a %= mod;
  if (a < 0)
    a += mod;
  b %= mod;
  if (b < 0)
    b += mod;
  ll res = 0;
  while (b > 0) {
    if (b & 1LL) {
      res += a;
      if (res >= mod)
        res -= mod;
    }
    b >>= 1LL;
    if (b) {
      a <<= 1LL;
      if (a >= mod)
        a %= mod;
    }
  }
  return res % mod;
}

// Algoritmo de Euclides para Máximo Divisor Comum (MDC)
ll mdc(ll a, ll b) {
  printf("      -- Calculando MDC entre %lld e %lld --\n", a, b);
  while (b != 0) {
    ll resto = a % b;
    printf("      -> %lld mod %lld = %lld\n", a, b, resto);
    a = b;
    b = resto;
  }
  printf("      -- Fim do cálculo. MDC = %lld --\n", llabs(a));
  return llabs(a);
}

// Versão do MDC sem exibição de passos
ll mdc_sem_print(ll a, ll b) {
  while (b != 0) {
    ll resto = a % b;
    a = b;
    b = resto;
  }
  return llabs(a);
}

// Algoritmo de Euclides Estendido
ll estendido_euclides(ll a, ll b, ll *x, ll *y) {
  if (a == 0) {
    *x = 0;
    *y = 1;
    return b;
  }
  ll x1, y1;
  ll mdc_val = estendido_euclides(b % a, a, &x1, &y1);
  *x = y1 - (b / a) * x1;
  *y = x1;
  return mdc_val;
}

// Versão do Euclides Estendido sem exibição de passos
ll estendido_euclides_sem_print(ll a, ll b, ll *x, ll *y) {
  if (a == 0) {
    *x = 0;
    *y = 1;
    return b;
  }
  ll x1, y1;
  ll mdc_val = estendido_euclides_sem_print(b % a, a, &x1, &y1);
  *x = y1 - (b / a) * x1;
  *y = x1;
  return mdc_val;
}

// Cálculo do Inverso Modular
ll inverso_modular(ll a, ll m) {
  ll x, y;
  ll g = estendido_euclides(a, m, &x, &y);
  if (g != 1) {
    printf("    [ERROR] O inverso modular não existe.\n");
    return -1;
  }
  // Garante que o resultado seja positivo
  ll inv = (x % m + m) % m;
  printf("    -> Inverso de %lld mod %lld = %lld\n", a, m, inv);
  return inv;
}

// Versão do Inverso Modular sem exibição de passos
ll inverso_modular_sem_print(ll a, ll m) {
  ll x, y;
  ll g = estendido_euclides_sem_print(a, m, &x, &y);
  if (g != 1)
    return -1;
  return (x % m + m) % m;
}

// Exponenciação Modular (Exponenciação por Quadratura)
long long powMod(ll base, ll exp, ll mod) {
  long long res = 1;
  long long b = base % mod;
  while (exp > 0) {
    // Se o expoente for ímpar, multiplica a base no resultado
    if (exp % 2 != 0)
      res = (res * b) % mod;

    // Eleva a base ao quadrado e reduz o expoente pela metade
    b = (b * b) % mod;
    exp >>= 1; // Equivalente a exp = exp / 2;
  }
  return res;
}
