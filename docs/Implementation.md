# llvm-covmap Implementation

`llvm-covmap` provides LLVM passes and runtime libraries to collect function-level
coverage information during runtime.

## The Coverage Bitmap

`llvm-covmap` records coverage information in a global bitmap data structure. Each
function in the instrumented program is been mapped to a single bit in the bitmap.
During program initialization, all bits in the bitmap are set to 0. In some run, 
if some function is being called, the corresponding bit in the bitmap is set to 1
to indicate the coverage of the function.

During instrumentation time, `llvm-covmap` assigns a 64-bit uniformly distributed 
random number to each  function in the program. During runtime, the offset of the 
bit corresponding to some function `foo` is computed as follows:

```
bit offset = (random number of foo) % (size of bitmap in bits)
```

where `%` is the modulo operation.

It's recommended to set the bit size of the bitmap to some power of 2 to ensure
that the bit offsets of all functions are still uniformly distributed within the
size of the bitmap. Keeping the bit offsets of all functions uniformly distributed
is important since this avoids collision as much as possible.

## Shared Memory

The coverage bitmap is stored in a POSIX shared memory region during runtime. This
allows other programs to read the coverage in real-time easily.

## Environment Variables

The following environment variables are used during runtime:

- `LLVM_COVMAP_SHM_SIZE`: This variable specifies the size of the coverage bitmap.
Note that this variable indicates the **byte** size of the coverage bitmap. The
default value of this variable is 1048576, which implies an 1MB bitmap.
- `LLVM_COVMAP_SHM_NAME`: This variable specifies the name of the POSIX shared
memory in which the coverage bitmap is stored. This name will be passed to the
[`shm_open`](https://man7.org/linux/man-pages/man3/shm_open.3.html) function 
without modification. If this variable is not set, the coverage will not be recorded.
