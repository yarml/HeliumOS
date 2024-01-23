int isdigit(int c) {
  switch (c) {
    case '0' ... '9':
      return 1;
    default:
      return 0;
  }
}

int isndigit(int c) {
  switch (c) {
    case '0' ... '9':
      return 0;
    default:
      return 1;
  }
}

int isbdigit(int c, int base) {
  return ('0' <= c && c < '0' + (base < 10 ? base : 10)) ||
         ('A' <= c && c < 'A' + base - 10) || ('a' <= c && c < 'a' + base - 10);
}

int isnbdigit(int c, int base) {
  return !isbdigit(c, base);
}