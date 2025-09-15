# MYUNIX

## 简介

![ZJUNIX Experiments](https://img.shields.io/badge/ZJUNIX-Experiments-blue.svg)

基于 ZJUNIX 的框架， MYUNIX **重新实现了内存管理、进程管理和文件系统**。  
[关于ZJUNIX](https://github.com/ZJUNIX/ZJUNIX)

## 特性

### 内存管理

1. 物理内存部分。重构**bootmem**，**buddy**，**slab**三个子模块。对`kmalloc`和`kfree`两个函数做了重构，实现了一个结构和空间大小给定的内存池。

2. 虚拟内存部分。在VMA中维护了当前虚拟内存的起止地址，同时用链表和红黑树来管理。

### 进程管理



### 文件系统



### 注意事项



## License

[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](./LICENSE)

此工程遵循 BSD3 协议
