int isdigit(int c)
{
  return '0' <= c && c <= '9';
}

int isndigit(int c)
{
  return c < '0' || '9' < c;
}

int isbdigit(int c, int base)
{
  return ('0' <= c && c < '0' + (base < 10 ? base : 10))
      || ('A' <= c && c < 'A' +  base - 10)
      || ('a' <= c && c < 'a' +  base - 10);
}

int isnbdigit(int c, int base)
{
  return !isbdigit(c, base);
}