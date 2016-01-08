
Hellofs - An example kernel filesystem implementation
===

A very simple linux kernel filesystem for learning purpose. It demonstrates how to implement a VFS filesystem, from superblock, inode, dir to file operations. The license is GPL because some kernel functions require it to be available.

Hellofs is written and tested on Centos 7.1.1503 kernel [3.10.0-229.1.2.el7.x86_64](http://lxr.free-electrons.com/source/?v=3.10). Note that from kernel version 3.11 some code related to dir operations are changed, for example [readdir](https://github.com/psankar/simplefs/blob/5d00eebd45ff9402848acfbbdbad4282393dd60a/simple.c#L212). It is rewritten based on Sankar's [Simplefs (commit 5d00eebd)](https://github.com/psankar/simplefs/tree/5d00eebd45ff9402848acfbbdbad4282393dd60a)). Thanks to the greate [Simplefs](https://github.com/psankar/simplefs)! Actually I found most resources about writting kernel FS are outdated (they are always talking about 2.6). But kernel is being developed in such rapid pace (it is 4.x now!). An working example is precious!

Major modifications that I made on Simplefs are

  * Total overhaul of the code structure to make them easier to understand.
  * Removed journal ([jbd2](https://github.com/psankar/simplefs/blob/5d00eebd45ff9402848acfbbdbad4282393dd60a/simple.c#L18)) related code since my kernel build doesn't support them.
  * Use bitmap to allocate inodes and data blocks, which should be more scalable.

The on-disk layout of Hellofs is 

  * superblock (1 block)
  * inode bitmap (1 block)
  * data block bitmap (1 block)
  * inode table (variable length)
  * data block table (variable length)

One disk block contains multiple inodes. One data block corresponds to one disk block (and of the same size). Each inode contains only one data block for simplicity.

To run test cases

```
cd hellofs
sudo ./hellofs-test.sh | grep "Test finished successfully"
```

