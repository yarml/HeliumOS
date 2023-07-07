char *strchr(char const *s, int c) {
  /* TODO: Idk why I didn't switch this to use memchr, wanted to change it
      But I'm afraid of breaking it. Figure this out later. */
  for (; *(unsigned char *)s != c; ++s)
    if (*s == 0) return 0;
  return (char *)s;
}
