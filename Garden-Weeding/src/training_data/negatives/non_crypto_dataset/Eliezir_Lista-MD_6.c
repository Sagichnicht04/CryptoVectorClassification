/*Resolver o seguinte problema: mdc(a,b) = am +bn*/

#include <stdio.h>

int inverso_modular( unsigned long long int a,  unsigned long long int b){
    //Quantia de linhas no algoritimo de euclides 
    int i = 1000;

    //Declarando os vetores necessários
    unsigned long long int resto[i] ,  quociente[i], m[i], n[i], expoente = a, mod_k = b;

    //Definindo os dois valores iniciais que são conhecidos 
    resto[0] = a; resto[1] = b;
    m[0] = 1; m[1] = 0;
    n[0] = 0; n[1] = 1;

    int index = 0; 
    while (a % b != 0)
    {
        quociente[index + 2] = resto[index] / resto[index + 1];
        resto[index + 2] = resto[index] % resto[index+1];

        m[index+2] = m[index] - (m[index+1] * quociente[index+2]);
        n[index+2] = n[index] - (n[index+1] * quociente[index+2]);

        a = b;
        b = resto[index + 2];

        index++;
    }
   
    /*while( m[index+1] < 0){
        m[index+1] =  m[index+1] + mod_k; 
    }*/
    //printf("O inverso %lld mod %lld é %lld\n",resto[0], resto[1], m[index+1]);
    printf("mdc(%lld, %lld) = %lld(%lld) + %lld(%lld)\n", expoente, mod_k, expoente, m[index+1], mod_k, n[index+1]);
    printf("m = %lld | n = %lld\n", m[index+1], n[index+1]);

    return 0;
}

int main(){
    unsigned long long int e, k; 
    //Lendo os valores de a e b 
    scanf("%lld %lld", &e, &k);

    int d = inverso_modular(e, k);

    return 0; 
}