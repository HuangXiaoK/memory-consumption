# memory-consumption
Simple simulation of memory-intensive programs to perform frequent memory operations.

Memory operations with 1 : 1 read-write ratio.
mem_cons_type_one is an operation on a double type array of size n, 
and mem_cons_type_two is a memory operation on a buffer of size n.
When some machines run mem_cons_type_one across numa nodes( For example, the cpu core is in numa 0 and the memory is in numa 1), the read-write ratio of memory operations may not be 1:1.
It may be a problem caused by the architecture of the machine.

The cpu core distribution in numa nodes of different machines may be different, and the **numa_node_cpu** array in the code needs to be changed.

You can use Intel's PCM to check memory consumption:
```
sudo pcm-memory
```

## Compilation
`make`

## Running

./mem_cons_type_one -h
Usage: 
 -s   <array_size>  array size, default is 100000000
 -t   <threads>  number of threads, default is 1
 -nt  <numa_of_threads>  numa of threads, default is 0
 -nm  <numa_of_memory>  numa of memory, default is 0
 -h   display the help information

./mem_cons_type_two -h
Usage: 
 -s   <buf_size>  buffer size, default is 104857600
 -t   <threads>  number of threads, default is 1
 -nt  <numa_of_threads>  numa of threads, default is 0
 -nm  <numa_of_memory>  numa of memory, default is 0
 -h   display the help information
```
