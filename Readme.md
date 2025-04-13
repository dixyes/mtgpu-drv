# mtgpu dkms driver mod

摩尔线程 MooreThreads MTT mtgpu dkms 驱动修改适配

## 官方原版驱动

musa_sdk中附带了较版本的驱动，

musa_sdk[下载链接](https://developer.mthreads.com/sdk/download/musa)

驱动中心提供的[驱动下载](https://www.mthreads.com/pes/drivers/search)

## 使用说明

当进行驱动安装时候，

`sudo dpkg -i musa_3.0.0_amd64.deb`

所用的linux内核版本过大，可能会有一些问题，

本仓库致力于采取非官方方式解决问题。

本仓库的文件是位于 `/usr/src/mtgpu-x.y.z` 下面的（xyz为版本号），可以对照进行修改。

## 6.13问题

6.13移除了`iommu_domain_alloc`和`os_iommu_present`，而私有二进制中似乎用到了这两个函数。

没有简单的替代手段，这可能导致IOMMU相关的问题

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
