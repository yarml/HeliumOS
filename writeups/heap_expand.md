Start: May 23, 2024
End: 

# Problem
Heap expansion takes a lot of time, even more so in real hardware. PC takes 10 minutes
to finish the initialization routine.

There are 2 things that happen when expanding kernel heap: Allocating physical memory, then
mapping it. Since I can't access logs on hardware, and I am too lazy to print characters to the
framebuffer, I jsut made a debug_set_pixel function, each pixel color in a location has a meaning. Autism.

Anyways, the problem seems to be with the mapping part, the physical allocation happens so fast I cannot even see it's pixel.
Narrowing it further down, the actual mapping happens fast as well, what seems to be the problem is the allocation of the 2
temporary pages from VCache.

I made a small sort of "profiler" using RDTSC, seems like the bottleneck I get in real hardware I also get in Qemu, just less extreme
There is a double for loop in allocation of VCache pages that takes most the time.