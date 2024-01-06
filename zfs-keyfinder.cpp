#include <cstdio>
#include <cstdlib>
#include <bitset>
#include <inttypes.h>

#define BLOCK_SIZE 1024
#define ZFS_KEYLEN 32

// Adjust me downwards if you're getting too many false positives:
// Should find 99.99% of randomly generated keys:
#define POP_THRESHOLD_WIDTH 32

bool blockMatches(const uint64_t *block64, size_t size) {
    const char *block = (const char *) block64;
    
    // The rest of the block is padded with zeros:
    for (int i = ZFS_KEYLEN; i < (int) size; i++) {
        if (block[i] != 0) {
            return false;
        }
    }

    int popCountTotal = 0;
    for (int i = 0; i < ZFS_KEYLEN / sizeof(uint64_t); i++) {
        int thisPop = (int) std::bitset<sizeof(uint64_t) * 8>(block64[i]).count();
        
        // Reject wildly improbable bitcounts
        if (thisPop < 9 || thisPop > 55) {
            return false;
        }
        
        popCountTotal += thisPop;
    }
    
    // A random key should have about 50% of its bits set to 1:
    return popCountTotal >= ZFS_KEYLEN * 8 / 2 - POP_THRESHOLD_WIDTH
        && popCountTotal <= ZFS_KEYLEN * 8 / 2 + POP_THRESHOLD_WIDTH;
}

void printMatch(int64_t offset, const char *block) {
    printf("0x%012" PRIx64 " ", offset);
    for (int i = 0; i < ZFS_KEYLEN; i++) {
        printf("%02X", (unsigned char) block[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    FILE *file;
    
    if (argc == 2) {
        file = fopen(argv[1], "rb");
        if (!file) {
            perror("Error opening volume");
            return EXIT_FAILURE;
        }
    } else if (argc != 1) {
        printf("Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    } else {
        file = stdin;
    }

    // Ensure it's properly aligned for type-punning later:
    union {
        uint64_t block64[BLOCK_SIZE / sizeof(uint64_t)];
        char block[BLOCK_SIZE];
    };
    int64_t offset = 0;
    int64_t numMatches = 0;
    
    size_t bytesRead;
    while ((bytesRead = fread(block, 1, BLOCK_SIZE, file)) > 0) {
        if (blockMatches(block64, bytesRead)) {
            numMatches++;
            printMatch(offset, block);
            fflush(stdout);
        }
        offset += (int64_t) bytesRead;
    }

    fprintf(stderr, "Found %" PRId64 " candidates\n", numMatches);
    return EXIT_SUCCESS;
}