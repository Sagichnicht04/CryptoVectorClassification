// 程序开始
#include <stdio.h>
#include <string.h>

/*初始化函数*/
// 参数1:传入长度256的unsigned char型数组首地址
// 参数2：密钥，其内容可以随便定义：char key[256];
// 参数3是密钥的长度，Len = strlen(key);
void rc4_init(unsigned char *s, unsigned char *key, unsigned long Len)
{
    int i = 0, j = 0;
    char k[256] = {0};
    unsigned char tmp = 0;
    for (i = 0; i < 256; i++)
    {
        s[i] = i; // 初始化s盒,0~255
        k[i] = key[i % Len];
    }
    for (i = 0; i < 256; i++) // 将s盒打乱
    {
        j = (j + s[i] + k[i]) % 256;

        tmp = s[i]; // 交换s[i]和s[j]
        s[i] = s[j];
        s[j] = tmp;
    }
}

/*加解密*/
// 参数1：是上边rc4_init函数中，被搅乱的S-box;
// 参数2：是需要加密/解密的数据data;
// 参数3：data的长度.
void rc4_crypt(unsigned char *s, unsigned char *Data, unsigned long Len)
{
    int i = 0, j = 0, t = 0;
    unsigned long k = 0;
    unsigned char tmp;
    for (k = 0; k < Len; k++)
    {
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        tmp = s[i];
        s[i] = s[j]; // 交换s[x]和s[y]
        s[j] = tmp;

        t = (s[i] + s[j]) % 256;
        Data[k] ^= s[t];
    }
}

int main()
{
    unsigned char s[256] = {0}, s2[256] = {0}; // S-box
    char key[256] = {"justfortest"};
    unsigned char pData[512] = "Hello World";
    unsigned long len = strlen((char *)pData);
    int i;

    printf("pData=%s\n", pData);
    printf("key=%s,length=%d\n\n", key, strlen(key));
    rc4_init(s, (unsigned char *)key, strlen(key)); // 已经完成了初始化
    printf("完成对S[i]的初始化，如下：\n\n");
    for (i = 0; i < 256; i++)
    {
        printf("%02X", s[i]);
        if (i && (i + 1) % 16 == 0)
            putchar('\n');
    }
    printf("\n\n");
    for (i = 0; i < 256; i++) // 用s2[i]暂时保留经过初始化的s[i]，很重要的！！！
    {
        s2[i] = s[i];
    }
    printf("已经初始化，现在加密:\n\n");
    rc4_crypt(s, (unsigned char *)pData, len); // 加密
    for (i = 0; pData[i]; i++)
    {
        printf("0x%x,", pData[i]);
    }
    // printf("pData=%s\n\n", pData);
    printf("\n");
    printf("已经加密，现在解密:\n\n");
    // rc4_init(s,(unsignedchar*)key,strlen(key));//初始化密钥
    rc4_crypt(s2, (unsigned char *)pData, len); // 解密
    printf("pData=%s\n\n", pData);
    return 0;
}

// 程序完
