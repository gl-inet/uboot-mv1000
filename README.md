### Prepare the build environment:

```
sudo apt-get install device-tree-compiler libssl-dev gcc-arm-linux-gnueabi
```

### Get uboot source:

```
git clone https://github.com/gl-inet/uboot-mv1000.git
```

### Setup toolchan:

```
mkdir -p ~/uboot-mv1000/toolchain
cd ~/uboot-mv1000/toolchain
wget https://releases.linaro.org/components/toolchain/binaries/7.5-2019.12/aarch64-linux-gnu/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu.tar.xz
tar -xvf gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu.tar.xz
export PATH=$PATH:~/uboot-mv1000/toolchain/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin
```

### Compile uboot:

```
cd ~/uboot-mv1000
make mvebu_espressobin-88f3720_defconfig
make DEVICE_TREE=armada-3720-espressobin
cd atf-marvell
make DEBUG=1 USE_COHERENT_MEM=0 LOG_LEVEL=20 SECURE=0 CLOCKSPRESET=CPU_1000_DDR_800 DDR_TOPOLOGY=5 BOOTDEV=SPINOR PARTNUM=0 PLAT=a3700 all fip
```

### Uboot image:

~/uboot-mv1000/atf-marvell/build/a3700/debug/flash-image.bin
