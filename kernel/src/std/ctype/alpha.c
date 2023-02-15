int isupper(int c)
{
  return 'A' <= c && c <= 'Z';
}

int isnupper(int c)
{
  return c < 'A' || 'Z' < c;
}


int islower(int c)
{
  return 'a' <= c && c <= 'z';
}

int isnlower(int c)
{
  return c < 'a' || 'z' < c;
}


int isalpha(int c)
{
  return ('A' <= c && c <= 'Z')
      || ('a' <= c && c <= 'z');
}

int isnalpha(int c)
{
  return (c < 'A' || 'Z' < c)
      && (c < 'a' || 'z' < c);
}