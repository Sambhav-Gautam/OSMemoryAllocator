MeMS: Node.js Memory Allocator 📦
Custom memory management for Node.js with a page-based allocation system.


🚀 Overview
MeMS (Memory Management System) is a Node.js module that provides a custom memory allocator implemented as a native addon. Built with node-gyp, it offers efficient page-based memory allocation and deallocation, ideal for applications requiring fine-grained control over memory. MeMS uses a virtual address space with main and sub-chains to manage memory blocks, supporting functions like malloc, free, and get for physical address mapping.
Key Features

🛠 Page-Based Allocation: Allocates memory in 4KB pages for efficiency.
🔗 Main and Sub-Chains: Organizes memory in linked lists for dynamic allocation.
📊 Memory Stats: Provides detailed statistics on memory usage and fragmentation.
💻 Cross-Platform: Supports Windows, macOS, and Linux (Node.js >= 14.0.0).
⚡ Native Performance: Leverages C for low-level memory operations.


📋 Prerequisites
Before installing MeMS, ensure you have the following:

Node.js (>= 14.0.0): Download
Python (3.7–3.12): Required for node-gyp builds. Download
Build Tools:
Windows: Visual Studio 2022 Build Tools with "Desktop development with C++" workload.
macOS/Linux: gcc, make, and python3.


node-gyp: Install globally with npm install -g node-gyp@latest.


🛠 Installation
Install MeMS via npm:
npm install @sambhav-gautam/osmemoryallocator

This command downloads the package and runs node-gyp rebuild to compile the native addon (mems.node). Ensure your build environment is set up correctly to avoid compilation errors.
Windows Setup

Install Python 3.12:winget install Python.Python.3.12
npm config set python C:\Path\To\Python312\python.exe


Install Visual Studio 2022 Build Tools with C++ workload.
Use Node.js 20.x for stability:nvm install 20
nvm use 20




💻 Usage
MeMS provides a simple API for memory management. Below is an example to get started:
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

Example Output:
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


🖼 Architecture

Placeholder: Add an architecture diagram showing main/sub-chains, virtual/physical address mapping, and page allocation. Host the image in your GitHub repo (e.g., images/architecture.png) and update the URL.
MeMS uses a two-level structure:

Main Chain: Tracks large memory blocks allocated via mmap, each with a virtual address range (SVA to EVA).
Sub-Chain: Manages allocated and free (hole) segments within a main block, enabling efficient reuse.


📚 API Reference



Function
Description
Parameters
Returns



init()
Initializes the MeMS system, setting up the memory management structure.
None
undefined


malloc(size)
Allocates size bytes, returning a virtual address. Aligns to 4KB pages.
size (number)
BigInt (virtual address)


get(v_ptr)
Maps a virtual address to its physical address.
v_ptr (BigInt)
BigInt (physical address)


free(ptr)
Frees memory at the given virtual address, marking it as a hole.
ptr (BigInt)
undefined


printStats()
Prints memory usage statistics, including pages used and unused space.
None
undefined


finish()
Unmaps all allocated memory and resets the MeMS system.
None
undefined


Notes

Always call init() before other functions.
malloc returns a BigInt representing the starting virtual address (SVA).
free merges adjacent holes to reduce fragmentation.
Use printStats to debug memory allocation.


🧪 Testing
Run the included test script to verify functionality:
npm test

This executes test.js (create one if not present):
const mems = require('@sambhav-gautam/osmemoryallocator');

mems.init();
const ptr = mems.malloc(1000);
console.log('Allocated:', ptr);
mems.printStats();
mems.free(ptr);
mems.printStats();
mems.finish();


🛠 Building from Source
To build the native addon manually:

Clone the repository:git clone https://github.com/Sambhav-Gautam/osmemoryallocator.git
cd osmemoryallocator


Install dependencies:npm install


Build:npm run build




🤝 Contributing
Contributions are welcome! 🎉 Follow these steps:

Fork the repository.
Create a feature branch: git checkout -b feature/your-feature.
Commit changes: git commit -m "Add your feature".
Push to the branch: git push origin feature/your-feature.
Open a pull request.

Please read CONTRIBUTING.md for details.

📜 License
This project is licensed under the ISC License. See LICENSE for details.

📬 Contact

Author: Sambhav Gautam
GitHub: Sambhav-Gautam
Issues: Report a bug


Built with 💖 for performance-driven Node.js applications.
