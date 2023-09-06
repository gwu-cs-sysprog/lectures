### You already know **C** 

well, some of it anyways...

### C constructs [familiar (w.r.t. Java)]

|construct|syntax|
|--------|--------|    
|conditionals| `if` and `else`|
|loops| `while{}`, `do...while`, `for`|
|**basic** types| `int`, `float`, `double`, `char`|
|**compound**| `arrays`*|
|functions| `ret_val` **`func_name`** `(args){}`|
|||

\* **no range checks!!**

### what's different [from java]

* no object-oriented programming
* no function overloading
* **no classes!**
* we have `struct` instead
    * which is similar but *very* different
* **compiled** not interpreted
* **pointers!!!**

### compound types

**contiguous memory** layouts

|type|usage|
|--------|--------|  
| <scb>struct</scb> | related variables (like class) |
| <scb>union</scb> | same, but *shared* memory |
| <scb>enum</scb> | enumeration, assign names |
| array | pointer to contiguous memory |
|||


### **no (built-in) boolean!**
