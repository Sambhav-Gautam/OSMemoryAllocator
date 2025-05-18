# MeMS: Node.js Memory Allocator

```plaintext
  __  __ _____ __  __  ____  
 |  \/  | ____|  \/  |/ ___| 
 | |\/| |  _| | |\/| | |___  
 |_|  |_|_|   |_|  |_|\____| 
 Memory Management System
```

Custom memory management for Node.js with a page-based allocation system.

---

## Overview

MeMS (Memory Management System) is a Node.js module that provides a custom memory allocator implemented as a native addon. Built with node-gyp, it offers efficient page-based memory allocation and deallocation, ideal for applications requiring fine-grained control over memory.

### Key Features

- **Page-Based Allocation**: Allocates memory in 4KB pages for efficiency  
- **Main and Sub-Chains**: Organizes memory in linked lists for dynamic allocation  
- **Memory Stats**: Provides detailed statistics on memory usage and fragmentation  
- **Cross-Platform**: Supports Windows, macOS, and Linux (Node.js >= 14.0.0)  
- **Native Performance**: Leverages C for low-level memory operations  

---

## Prerequisites

Before installing MeMS, ensure you have the following:

- **Node.js** (>= 14.0.0): [Download](https://nodejs.org/)
- **Python** (3.7–3.12): Required for node-gyp builds. [Download](https://www.python.org/)
- **Build Tools**:
  - Windows: Visual Studio 2022 Build Tools with "Desktop development with C++" workload
  - macOS/Linux: gcc, make, and python3
- **node-gyp**: Install globally with `npm install -g node-gyp@latest`

### Windows Setup

```bash
winget install Python.Python.3.12
npm config set python C:\Path\To\Python312\python.exe
```

Install Visual Studio 2022 Build Tools with C++ workload.  
For stability with Node.js 20.x:

```bash
nvm install 20
nvm use 20
```

---

## Installation

Install MeMS via npm:

```bash
npm install @sambhav-gautam/osmemoryallocator
```

This command downloads the package and runs `node-gyp rebuild` to compile the native addon (`mems.node`).

---

## Usage

```javascript
const mems = require('@sambhav-gautam/osmemoryallocator');

// Initialize MeMS
mems.init();
console.log('MeMS initialized');

// Allocate 1000 bytes
const ptr = mems.malloc(1000);
console.log('Allocated at virtual address:', ptr);

// Get physical address
const phyPtr = mems.get(ptr);
console.log('Physical address:', phyPtr);

// Print memory stats
mems.printStats();

// Free memory
mems.free(ptr);
console.log('Memory freed');

// Clean up
mems.finish();
console.log('MeMS finished');
```

**Example Output:**
```
MeMS initialized
Allocated at virtual address: 1000
Physical address: <some_physical_address>
MeMS Stats:
MAIN[1000:5095] -> <P>[SVA:1000:EVA:1999] <-> <H>[SVA:2000:EVA:5095] <-> <NULL>
Page used: 1
Space unused: 3096
Main Chain Length: 1
Sub-chain Length array: 2
Memory freed
..........Successfully UnMapped the Memory without Errors .........
MeMS finished
```

---

## Architecture

```
+----------------+    +----------------+    +----------------+
|   Main Chain   |--->| Virtual Memory |--->| Physical Pages |
+----------------+    +----------------+    +----------------+
       |                     |
       v                     v
+----------------+    +----------------+
|  Sub-Chain     |    | Address Mapping|
+----------------+    +----------------+
```

MeMS uses a two-level structure:  
- **Main Chain**: Tracks large memory blocks allocated via mmap  
- **Sub-Chain**: Manages allocated and free segments within main blocks  

---

## API Reference

| Function       | Description                                      | Parameters       | Returns         |
|----------------|--------------------------------------------------|------------------|-----------------|
| `init()`       | Initializes the MeMS system                      | None             | undefined       |
| `malloc(size)` | Allocates size bytes (aligns to 4KB)             | size (number)    | BigInt (vaddr)  |
| `get(v_ptr)`   | Maps virtual address to physical address         | v_ptr (BigInt)   | BigInt (paddr)  |
| `free(ptr)`    | Frees memory at given virtual address            | ptr (BigInt)     | undefined       |
| `printStats()` | Prints memory usage statistics                   | None             | undefined       |
| `finish()`     | Unmaps all allocated memory                      | None             | undefined       |

**Notes:**
- Always call `init()` before other functions  
- `malloc` returns a BigInt representing the starting virtual address  
- `free` merges adjacent holes to reduce fragmentation  

---

## Testing

Run the included test script:

```bash
npm test
```

Test script example (`test.js`):
```javascript
const mems = require('@sambhav-gautam/osmemoryallocator');

mems.init();
const ptr = mems.malloc(1000);
console.log('Allocated:', ptr);
mems.printStats();
mems.free(ptr);
mems.printStats();
mems.finish();
```

---

## Building from Source

```bash
git clone https://github.com/Sambhav-Gautam/osmemoryallocator.git
cd osmemoryallocator
npm install
npm run build
```

---

## Contributing

1. Fork the repository  
2. Create a feature branch: `git checkout -b feature/your-feature`  
3. Commit changes: `git commit -m "Add your feature"`  
4. Push to the branch: `git push origin feature/your-feature`  
5. Open a pull request  

See `CONTRIBUTING.md` for details.

---

## License

ISC License. See `LICENSE` for details.

---

## Contact

- **Author**: Sambhav Gautam  
- **GitHub**: [Sambhav-Gautam](https://github.com/Sambhav-Gautam)  
- **Issues**: [Report a bug](https://github.com/Sambhav-Gautam/osmemoryallocator/issues)