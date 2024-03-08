# mtgpu dkms driver

摩尔线程 MooreThreads MTT mtgpu dkms 驱动

大约适配了6.6 lts内核

## modification notes

binary中（至少）包含了services/server/devices和linux的pvr_bridge_k.c fwload.c osconnection_server.c

驱动源码来自ti的img-rouge驱动 https://git.ti.com/git/graphics/ti-img-rogue-driver.git

- x.y.z-6.1-img-rouge: linuxws/kirkstone/k6.1/23.2.6460340
- x.y.z-6.6-img-rouge: linuxws/scarthgap/k6.6/23.3.6512818

conftest改起来太麻烦了，也不好通过patch来存储修改（不方便给打包使用），所以遇到内核api变动的地方，只能用内核版本号来判断：

```C
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
    // do something
#else
    // do something else
#endif
```
