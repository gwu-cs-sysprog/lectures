
## Intro to C

### You already know **C** {-}

well, some of it anyways...

### C constructs [if you're familiar with Java, Python, etc.]

|construct|syntax|
|--------|--------|    
|conditionals| `if` and `else`|
|loops| `while{}`, `do...while`, `for`|
|**basic** types| `int`, `float`, `double`, `char`|
|**compound**| `arrays`*|
|functions| `ret_val` **`func_name`** `(args){}`|
|||

\* **no range checks!!**: C will let you access an array beyond the maximum size that you have specified while creating it. The effects of such access are **implementation specific** -- each platform/operating system will handle it differently. Note that on platforms that don't have memory protection, this can cause some serious problems!

### What's different [from java] {-}

* no object-oriented programming
* no function overloading
* **no classes!**
* we have `struct` instead
    * which is similar but *very* different
* **compiled** not interpreted
* **pointers!!!**

### Compound types

**contiguous memory** layouts: objects within these data types are laid out in memory, next to each other. This becomes important when you're trying to use pointers to access various elements.

|type|usage|
|--------|--------|  
| <scb>struct</scb> | related variables (like class) |
| <scb>union</scb> | same, but *shared* memory |
| <scb>enum</scb> | enumeration, assign names |
| array | pointer to contiguous memory |
|||

#### **no (built-in) boolean!** {-}

C does not have a built in boolean data type. We can mimic it by using integer values (<scb>0</scb> is `false` while any other non-zero values is `true`).
