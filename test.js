const mems = require('./build/Release/mems.node');

console.log('------- Initializing MeMS -------');
mems.init();

console.log('------- Allocating memory -------');
const vAddr = mems.malloc(1000);
console.log(`Virtual address: ${vAddr}`);

console.log('------- Accessing memory -------');
const addresses = [vAddr, BigInt(vAddr) + 4n, BigInt(vAddr) + 996n];
for (const addr of addresses) {
  console.log(`Attempting to get physical address for vAddr: ${addr}`);
  try {
    const pAddr = mems.get(addr);
    console.log(`  Physical address at ${addr} : ${pAddr}`);
  } catch (e) {
    console.log(`  Error accessing ${addr}: ${e.message}`);
  }
}

console.log('------- Printing stats -------');
mems.printStats();

console.log('------- Freeing memory -------');
try {
  mems.free(vAddr);
  console.log(`Memory freed at virtual address: ${vAddr}`);
} catch (e) {
  console.log(`Error freeing memory at ${vAddr}: ${e.message}`);
}

console.log('------- Printing stats after free -------');
mems.printStats();

console.log('------- Cleaning up -------');
mems.finish();
