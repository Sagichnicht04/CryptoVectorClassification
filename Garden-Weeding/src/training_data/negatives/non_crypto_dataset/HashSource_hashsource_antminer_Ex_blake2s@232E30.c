int __fastcall blake2s(void *a1, char *a2, const void *a3, unsigned __int8 a4, unsigned __int64 a5, unsigned __int8 a6)
{
  _BYTE v11[193]; // [sp+4Fh] [bp-CDh] BYREF

  if ( !a2 )
    return -1;
  if ( a1 )
  {
    if ( !a3 )
      a6 = 0;
    if ( a6 )
    {
      if ( blake2s_init_key((_DWORD *)((unsigned int)v11 & 0xFFFFFFC0), a4, a3, a6) < 0 )
        return -1;
    }
    else if ( blake2s_init((void *)((unsigned int)v11 & 0xFFFFFFC0), a4) < 0 )
    {
      return -1;
    }
    blake2s_update((_DWORD *)((unsigned int)v11 & 0xFFFFFFC0), a2, a5);
    blake2s_final((_DWORD *)((unsigned int)v11 & 0xFFFFFFC0), a1, a4);
    return 0;
  }
  return -1;
}
