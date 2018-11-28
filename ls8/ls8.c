#include <stdio.h>
#include "cpu.h"

/**
 * Main
 */
int main(int argc, char **argv)
{
  struct cpu cpu;
  if(argc < 2){
    printf("you are missing a file\n");
    return 0;
  }
  cpu_init(&cpu);
  cpu_load(&cpu, argv[1]);
  cpu_run(&cpu);

  return 0;
}