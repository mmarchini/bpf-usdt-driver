#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <errno.h>
#include <sys/sdt.h>


#include "../src/bpf_usdt_driver.h"
#include "dyn.h"

extern char *program_invocation_name;
extern char *program_invocation_short_name;

#define DEVICE_FILE_NAME "/dev/bpf_usdt"
#define FUNC_SIZE 16

typedef struct USDTProbe_ {
  BpfUsdtProbe *probe;
  int index;
  void (*fire)();
} USDTProbe;

// TODO ASM implementation like libusdt?
void _bpf_usdt_fire() {

}

int this_probe_is_on_fire(USDTProbe *usdt) {
  int fd;
  char tmp[23] = "/tmp/bpfusdtprobeXXXXXX";

  if ((fd = mkstemp(tmp)) < 0)
          return -1;
  if (unlink(tmp) < 0)
          return -1;
  if (write(fd, "\0", FUNC_SIZE) < FUNC_SIZE)
          return -1;

  usdt->fire = (void (*)())mmap(NULL, FUNC_SIZE,
                                          PROT_READ | PROT_WRITE | PROT_EXEC,
                                          MAP_PRIVATE, fd, 0);

  if (usdt->fire == NULL)
    return -1;

  memcpy((void *)usdt->fire, (const void *)_bpf_usdt_fire, FUNC_SIZE);

  mprotect((void *)usdt->fire, FUNC_SIZE, PROT_READ | PROT_EXEC);

  return 0;
}

USDTProbe *register_probe(char *provider, char *probe_name) {
  int file_desc, ret_val;
  USDTProbe *usdt = malloc(sizeof(USDTProbe));

  if(this_probe_is_on_fire(usdt) < 0) {
    free(usdt);
    return NULL;
  }

  usdt->probe =  malloc(sizeof(BpfUsdtProbe));

  usdt->probe->pid = getpid();
  usdt->probe->addr = (unsigned long) usdt->fire;
  strcpy(usdt->probe->module, program_invocation_short_name);
  strcpy(usdt->probe->provider, provider);
  strcpy(usdt->probe->probe, probe_name);

  file_desc = open(DEVICE_FILE_NAME, 0);
  ret_val = ioctl(file_desc, BPF_USDT_ADD, usdt->probe);

  if(ret_val < 0) {
    printf("Fudeu\n");
  } else {
    usdt->index = ret_val;
    printf("OMG\n");
    ;
  }

  return usdt;
}

int fire_probe(USDTProbe *usdt) {
  usdt->fire();
  return 0;
}

int main() {
  DTRACE_PROBE(opa, treta);
  int which = 0;
  USDTProbe *probe1 = register_probe("mainer", "lorem");
  USDTProbe *probe2 = register_probe("mainer", "ipsun");

  while(1) {
    if(which == 0) {
      printf("Firing probe 'lorem'\n");
      fire_probe(probe1);
      which ++;
    } else {
      printf("Firing probe 'ipsum'\n");
      fire_probe(probe2);
      which = 0;
    }
    sleep(5);
    DTRACE_PROBE(opa, treta);
  }
  return 0;
}
