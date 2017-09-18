# BPF USDT Linux Driver

Example repository on how to achieve the same results from libusdt using Linux
eBPF/bcc.

## Getting started

First you need to get the git submodules, and then start the vagrant box:

```bash
git submodule update --init;
vagrant up;
vagrant ssh;
```

Now you need to build bcc (dependencies are already installed):

```bash
cd /bpf-usdt-driver/bcc;
mkdir build;
cd build;
make
sudo make install
```

After that, you can build the driver:

```bash
cd /bpf-usdt-driver
make
```

To load the driver, just run:

```bash
cd /bpf-usdt-driver
make load  # `make unload` to unload the driver
```

To build and run the validation program, just run:

```bash
cd /bpf-usdt-driver
make bin-dyn
./dyn  # will register two dynamic probes: "mainer:lorem" and "mainer:ipsum" and one static probe "opa:treta"
```

Now you can see all probes using:

```bash
/usr/share/bcc/tools/tplist -p $(pgrep dyn)
```

## TODO

* Make it work with bcc/trace.py
