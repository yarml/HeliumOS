int isprint(int c)
{
  switch(c)
  {
    case '0' ... '9':
    case 'A' ... 'Z':
    case 'a' ... 'z':
    case ' ':
    case '!':
    case '"':
    case '#':
    case '$':
    case '%':
    case '&':
    case '\'':
    case '(':
    case ')':
    case '*':
    case '+':
    case ',':
    case '-':
    case '.':
    case '/':
    case ':':
    case ';':
    case '<':
    case '=':
    case '>':
    case '?':
    case '@':
    case '[':
    case '\\':
    case ']':
    case '^':
    case '_':
    case '`':
    case '{':
    case '|':
    case '}':
    case '~':
      return 1;
    default:
      return 0;
  }
}

int isspace(int c)
{
  switch(c)
  {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
      return 1;
    default:
      return 0;
  }
}