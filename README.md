# CS_Exp_ZNS

> 文本来源：XJTU2022计算机系统综合实验 王老师PPT （侵删）

## 实验目的

- 本实验涉及到计算机组成原理、操作系统、程序设计等多门课程知识，锻炼同学们对业界最新软硬件技术全栈开发能力和学习能力。
- NVMe协议已成为最新SSD访问协议，ZNS SSD是最新SSD技术发展成果，NVMe最新协议已增加对ZNS SSD支持， ZNS SSD对于存储系统性能提升有重要意义。本实验通过使用QEMU模拟ZNS SSD设备，基于Intel SPDK框架，在应用中使用ZNS SSD作为底层存储并实现I/O访问，理解从应用层到物理存储设备之间数据读写原理。

## 背景知识

### NVMe

- NVMe协议标准由NVM Express公司监管，这是一个由100多个组织组成的联盟，这些组织致力于开发更快的协议以提高非易失性存储的性能。该组织由一个13家公司组成的董事会领导，其中包括Cavium、Cisco、Dell EMC、Facebook、英特尔、Micron、Microsemi、微软、NetApp、三星、希捷、东芝内存和Western Digital。
- NVMe是一种高性能、高度可扩展的存储协议，用于连接主机和内存子系统。NVMe是专门为闪存等非易失性存储设计，建立在高速PCIe通道上。
- NVMe的本质是建立了多个计算机与存储设备的通路。NVMe在单个消息队列中支持64000个命令，最多支持65535个I/O队列。相比之下，SAS设备的队列深度通常在一个队列中最多支持256个命令，而SATA驱动器最多支持32个命令。类比来说，如果SATA是一条普通的小道，每次只能通过32辆车的话，那NVMe就是一条拥有65535条车道的高速公路，每条车道能通过64000辆车。

### QEMU

- QEMU是一个通用的开源模拟器，由 Fabrice Bellard 编写。它可以独立模拟出整台计算机，包括 CPU，内存，IO 设备。
- QEMU+KVM 则可以通过 KVM 模块提供的虚拟化技术，提高 CPU 和内存性能，并为虚拟机提供加速功能。
- QEMU对最新硬件设备支持较好，最新版本可支持ZNS SSD。
- QEMU可以模拟ARM、RISC-V等多种处理器架构。

### ZNS SSD

- ZNS SSD是在Open Channel SSD基础上发展而来的，因此它继承了Open Channel SSD I/O分离、可预测性延迟等优势；另一方面，ZNS协议将NVMe 2.0中的一部分进行了标准化处理，简化了软件架构，化解了Open Channel过于灵活发散、不利于规范化、标准化的难题，使得企业更加易于根据自身场景需进行特定软件开发。
- 相比普通SSD，新一代ZNS SSD具有三大优势
  - 采用顺序写入方式大幅缩减了整盘的写放大，提升了SSD使用寿命
  - NAND闪存颗粒控制机制对应用透明化，有效提升读写性能、降低了延迟
  - 普通SSD保存每TB数据需要提供1GB预留空间作为FTL（Flash Translation Layer）闪存转换层，这就好比每本字典需要留几页作为目录，ZNS SSD基于顺序写入技术降低了整盘的预留空间需求

### SPDK

 SPDK是一套存储开发套件，专门为专用设备(NVME)设计。全称是The Storage Performance Development Kit。SPDK提供了一系列的高性能、可扩展、用户态下面的工具和库。在SPDK中，存储设备的驱动代码运行在用户态，不会运行在内核态，避免了内核的上下文切换节省了大量的处理开销，节省下来的CPU时间片可以用于实际的数据处理，比如重复数据删除、压缩、加密。SPDK的原则是通过消除每一处额外的软件开销来提供最少的延迟和最高的效率。

## 相关资源

[QEMU 7.1.0](https://download.qemu.org/qemu-7.1.0.tar.xz)

[Ubuntu Server 22.04](https://mirror.linux-ia64.org/ubuntu-releases/22.04.1/ubuntu-22.04.1-live-server-amd64.iso)

## 实验细节

[Miracle24's BLOG](https://miracle24.site/?s=%E8%AE%A1%E7%AE%97%E6%9C%BA%E7%B3%BB%E7%BB%9F%E7%BB%BC%E5%90%88%E8%AE%BE%E8%AE%A1%E5%AE%9E%E9%AA%8C)
