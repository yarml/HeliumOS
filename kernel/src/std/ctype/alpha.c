int isupper(int c) {
  switch (c) {
    case 'A' ... 'Z':
      return 1;
    default:
      return 0;
  }
}

int isnupper(int c) {
  switch (c) {
    case 'A' ... 'Z':
      return 0;
    default:
      return 1;
  }
}

int islower(int c) {
  switch (c) {
    case 'a' ... 'z':
      return 1;
    default:
      return 0;
  }
}

int isnlower(int c) {
  switch (c) {
    case 'a' ... 'z':
      return 0;
    default:
      return 1;
  }
}

int isalpha(int c) {
  switch (c) {
    case 'A' ... 'Z':
    case 'a' ... 'z':
      return 1;
    default:
      return 0;
  }
}

int isnalpha(int c) {
  switch (c) {
    case 'A' ... 'Z':
    case 'a' ... 'z':
      return 0;
    default:
      return 1;
  }
}