int isalnum(int c)
{
  return ('0' <= c && c <= '9')
      || ('A' <= c && c <= 'Z')
      || ('a' <= c && c <= 'z');
}

int isnalnum(int c)
{
  return (c < '0' || '9' < c)
      && (c < 'A' || 'Z' < c)
      && (c < 'a' || 'z' < c);
}
