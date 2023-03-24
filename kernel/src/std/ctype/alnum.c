int isalnum(int c)
{
  switch(c)
  {
    case 'A' ... 'Z':
    case 'a' ... 'z':
    case '0' ... '9':
      return 1;
    default:
      return 0;
  }
}

int isnalnum(int c)
{
  switch(c)
  {
    case 'A' ... 'Z':
    case 'a' ... 'z':
    case '0' ... '9':
      return 0;
    default:
      return 1;
  }
}
