# UCORE 关于pg fault 不被触发的疑问  
## 疑问简介
在内核空间申请一个比较大的数组（此时内核并没有突破4MB）后，访问0x100地址时，不触发page fault.

按照wiki的说明，内核在任何时间内，应该拥有占据4MB大小的权利。但是当申请比较大的数组后，内核似乎出现了bug，具体表现在，访问一个必然缺页的地址时，不产生缺页异常。

## 复现方法  
本repo代码基于答案`lab4_result`修改而成。  
创建任意一个 .c 文件，申请一个比较大的数组。  
例如创建 `kern/mm/cause_bug.c`，内容为
``` c
#include <defs.h>

// #define NO_BUG 135084
#define HAS_BUG 135085

/* alloc a LONG array */
/* END could not be larger than 0xc01b5000 */
uint32_t array[HAS_BUG];
```

申请长度为 135085的uint32_t数组。  
此时运行 `make qemu-nox`，会有如下报错
```
kernel panic at kern/mm/default_pmm.c:157:
    assertion failed: !PageReserved(p) && !PageProperty(p)
stack trackback:
ebp:0xc012ae78 eip:0xc0101ded args:0x0000000a 0x0000000a 0xc012aeb8 0xc012aeac
    kern/debug/kdebug.c:308: print_stackframe+21
ebp:0xc012ae98 eip:0xc01017ce args:0xc010c5e7 0x0000009d 0xc010c5d2 0xc010c610
    kern/debug/panic.c:27: __panic+107
ebp:0xc012af38 eip:0xc0108345 args:0xc01b6000 0x00000001 0xc012af68 0xc0104857
    kern/mm/default_pmm.c:157: default_free_pages+163
ebp:0xc012af68 eip:0xc010486d args:0xc01b6000 0x00000001 0x00000002 0xc01064f5
    kern/mm/pmm.c:184: free_pages+32
ebp:0xc012afa8 eip:0xc0106681 args:0x00210494 0x0021037c 0x00a011d8 0x00007c2b
    kern/mm/vmm.c:270: check_pgfault+414
ebp:0xc012afc8 eip:0xc01060d2 args:0xc012b560 0x00000100 0xc012aff8 0xc0100096
    kern/mm/vmm.c:168: check_vmm+23
ebp:0xc012afd8 eip:0xc01060b7 args:0x00000000 0x00000000 0x00000000 0xc010a840
    kern/mm/vmm.c:159: vmm_init+10
ebp:0xc012aff8 eip:0xc0100096 args:0xc010ac98 0xc010aca0 0xc0102072 0xc010acbf
    kern/init/init.c:39: kern_init+95
```

原因是在`check_pgfault`运行中，在访问 0x100地址时，没有触发page fault。
进而导致pg_dir[0]依旧保持为空（0x0），导致free_page(pde2page(pg_dir[0]))企图释放0号物理页框。

## 一些猜想
是否 End 不能超过 0xc01b5000?
当End超过这个值后（例如多分配一个页），达到0xc01b5020时，就会出现上述的错误。
