# mtgpu dkms driver

摩尔线程 MooreThreads MTT mtgpu dkms 驱动

大约适配了6.12内核

## modification notes

binary中（至少）包含了services/server/devices和linux的pvr_bridge_k.c fwload.c osconnection_server.c

可以参考来自ti的img-rouge驱动 https://git.ti.com/git/graphics/ti-img-rogue-driver.git

conftest改起来太麻烦了，也不好通过patch来存储修改（不方便给打包使用），所以遇到内核api变动的地方，只能用内核版本号来判断：

```C
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
    // do something
#else
    // do something else
#endif
```
