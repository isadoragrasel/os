#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s B J input_file K disk0 disk1 ... diskN-1\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parse command line arguments
    int B = atoi(argv[1]);         // Block size in bytes
    int J = atoi(argv[2]);         // Size of input data in bytes
    char *inputPath = argv[3];     // Path to input file
    int K = atoi(argv[4]);         // Size of each RAID disk in bytes
    int N = argc - 5;              // Number of disks in RAID array
    char **diskPaths = &argv[5];   // Paths to output files for RAID disks

    // Validate parameters
    if (B < 1 || B > (1<<12) || 
        J < 1 || J % B != 0 || 
        K < 1 || K % B != 0 || 
        (long long)K * (N - 1) < J) {
        fprintf(stderr, "Invalid parameters.\n");
        return EXIT_FAILURE;
    }

    // Allocate memory for input data
    unsigned char *inputData = malloc(J);
    if (!inputData) {
        perror("Failed to allocate memory for input data");
        return EXIT_FAILURE;
    }

    // Read input file
    FILE *fin = fopen(inputPath, "r");
    if (!fin) {
        perror("Failed to open input file");
        free(inputData);
        return EXIT_FAILURE;
    }

    // Read hexadecimal data
    for (int i = 0; i < J; i++) {
        char hex[3] = {0};
        if (fread(hex, 1, 2, fin) != 2) {
            fprintf(stderr, "Error reading input file at byte %d\n", i);
            fclose(fin);
            free(inputData);
            return EXIT_FAILURE;
        }
        inputData[i] = (unsigned char)strtol(hex, NULL, 16);
    }
    fclose(fin);

    // Allocate memory for RAID disks
    unsigned char **disks = malloc(N * sizeof(unsigned char *));
    if (!disks) {
        perror("Failed to allocate memory for disk array");
        free(inputData);
        return EXIT_FAILURE;
    }

    for (int d = 0; d < N; d++) {
        disks[d] = calloc(K, 1); // Initialize with zeros
        if (!disks[d]) {
            perror("Failed to allocate memory for disk");
            for (int i = 0; i < d; i++) {
                free(disks[i]);
            }
            free(disks);
            free(inputData);
            return EXIT_FAILURE;
        }
    }

    // Calculate number of blocks in input data
    int numDataBlocks = J / B;
    int perStripe = N - 1;  // Data blocks per stripe (excluding parity)
    
    // Process each data block
    int dataBlockIdx = 0; // index of data block
    while (dataBlockIdx < numDataBlocks) { // while there are still data blocks to process
        int stripe = dataBlockIdx / perStripe; // calculate stripe index
        int offsetInStripe = dataBlockIdx % perStripe; // calculate offset in stripe

        int parityDisk = (N - 1 - stripe % N + N) % N; // calculate parity disk for this stripe
        int diskIndex = (parityDisk + 1 + offsetInStripe) % N; // calculate data disk index
        int offsetInDisk = stripe * B; // calculate offset in disk

        if (offsetInDisk >= K) break; // don't exceed disk size

        memcpy(disks[diskIndex] + offsetInDisk, inputData + (dataBlockIdx * B), B); // copy data block to disk

        if (offsetInStripe == perStripe - 1 || dataBlockIdx == numDataBlocks - 1) { // if all data blocks in this stripe are filled
            int stripeStartBlock = (dataBlockIdx / perStripe) * perStripe; // calculate start block of stripe
            memset(disks[parityDisk] + offsetInDisk, 0, B); // clear parity block first
            for (int i = 0; i < perStripe && stripeStartBlock + i < numDataBlocks; i++) { // iterate over each data block in the stripe
                int dataBlockIdx2 = stripeStartBlock + i; // index of data block in stripe
                int dataDiskIndex = (parityDisk + 1 + i) % N; // calculate data disk index
                for (int b = 0; b < B; b++) { // XOR this data block into the parity block
                    disks[parityDisk][offsetInDisk + b] ^= disks[dataDiskIndex][offsetInDisk + b];
                }
            }
        }

        dataBlockIdx++; // increment data block index
    }

    // Write data to output files
    for (int d = 0; d < N; d++) {
        FILE *fout = fopen(diskPaths[d], "w");
        if (!fout) {
            perror("Failed to open output file");
            continue;
        }

        for (int b = 0; b < K; b++) {
            fprintf(fout, "%02x", disks[d][b]);
        }
        fclose(fout);
    }

    // Free allocated memory
    for (int d = 0; d < N; d++) {
        free(disks[d]);
    }
    free(disks);
    free(inputData);

    return EXIT_SUCCESS;
}