.text

.align 4, 0x90
.globl _bpf_usdt_fire

_bpf_usdt_fire:
  # jmp _bpf_usdt_fire_return
  nop
