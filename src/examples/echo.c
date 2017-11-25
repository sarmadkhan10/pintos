#include <stdio.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
  int i;
  printf ("g");
    printf ("re");
  for (i = 0; i < argc; i++)
    printf ("%s ", argv[i]);


  return EXIT_SUCCESS;
}
