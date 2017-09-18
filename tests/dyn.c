#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/sdt.h>

#include "../src/bpf_usdt_driver.h"
#include "dyn.h"

#define DEVICE_FILE_NAME "/dev/bpf_usdt"
#define FUNC_SIZE 16

typedef struct USDTProbe_ {
  BpfUsdtProbe *probe;
  int index;
  void (*fire)();
} USDTProbe;

// TODO ASM implementation like libusdt?
void _bpf_usdt_fire() {
  DTRACE_PROBE(on, fire);
}

int this_probe_is_on_fire(USDTProbe *usdt) {
  int fd;
  char tmp[25] = "/tmp/BpfUSDTProbe-XXXXXX";

  if ((fd = mkstemp(tmp)) < 0)
          return -1;
  // if (unlink(tmp) < 0)
  //         return -1;
  if (write(fd, "\0", FUNC_SIZE) < FUNC_SIZE)
          return -1;

  usdt->fire = (void (*)())mmap(0, FUNC_SIZE,
                                          PROT_READ | PROT_WRITE | PROT_EXEC,
                                          MAP_SHARED, fd, 0);
                                          // MAP_PRIVATE, fd, 0);

  if (usdt->fire == NULL)
    return -1;

  memcpy((void *)usdt->fire, (const void *)_bpf_usdt_fire, FUNC_SIZE);
  strcpy(usdt->probe->module, tmp);
  printf("Fire: %lx\n", (unsigned long) usdt->fire);
  printf("%s\n", usdt->probe->module);
  // mprotect((void *)usdt->fire, FUNC_SIZE, PROT_READ | PROT_EXEC);

  return 0;
}

USDTProbe *register_probe(char *provider, char *probe_name) {
  int file_desc, ret_val;
  USDTProbe *usdt = malloc(sizeof(USDTProbe));
  usdt->probe =  malloc(sizeof(BpfUsdtProbe));

  if(this_probe_is_on_fire(usdt) < 0) {
    free(usdt->probe);
    free(usdt);
    return NULL;
  }


  usdt->probe->pid = getpid();
  printf("PID: %d\n", usdt->probe->pid);
  usdt->probe->addr = 0x00;
  printf("addr: %lx\n", usdt->probe->addr);
  strcpy(usdt->probe->provider, provider);
  strcpy(usdt->probe->probe, probe_name);
  printf("provider: %s\n", usdt->probe->provider);
  printf("probe: %s\n", usdt->probe->probe);

  file_desc = open(DEVICE_FILE_NAME, 0);
  ret_val = ioctl(file_desc, BPF_USDT_ADD, usdt->probe);

  if(ret_val < 0) {
    printf("Something is not right\n");
    free(usdt->probe);
    free(usdt);
      return NULL;
  } else {
    usdt->index = ret_val;
  }

  return usdt;
}

int fire_probe(USDTProbe *usdt) {
  usdt->fire();
  return 0;
}

int main() {
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
