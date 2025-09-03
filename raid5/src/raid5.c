#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// RAID-5 implementation: minimum of 3 disks, 1 parity disk

int main(int argc, char *argv[]) { // command line arguments, array of strings holding each
    if (argc < 6) { // error if not enough arguments
        fprintf(stderr, "Usage: %s B J input_file K disk0 disk1 ... diskN-1\n", argv[0]);
        return EXIT_FAILURE;
    }

    // parse command line arguments
    int B = atoi(argv[1]); // block size
    int J = atoi(argv[2]); // total size of input file
    char *inputPath = argv[3]; // path to input file
    int K = atoi(argv[4]); // size of each disk
    int N = argc - 5; // number of disks
    char **diskPaths = &argv[5]; // paths to disks

    // validate parameters as instructed
    if (B < 1 || B > (1<<12) // block size must be positive and less than 4096
     || J < 1 || J % B != 0 // total size must be positive and a multiple of block size
     || K < 1 || K % B != 0 // size of each disk must be positive and a multiple of block size
     || (long long)K * (N - 1) < J) { // total size must be less than or equal to size of all disks minus parity disk
        fprintf(stderr, "Invalid parameters.\n");
        return EXIT_FAILURE;
    }

    unsigned char *inputData = malloc(J); // allocate memory for input data
    if (!inputData) { 
        perror("Failed to allocate memory for input data"); 
        return EXIT_FAILURE; } // error if memory allocation fails

    FILE *fin = fopen(inputPath, "r"); // open input file
    if (!fin) { // error if file cannot be opened
        perror("Failed to open input file"); 
        free(inputData); // free memory for input data
        return EXIT_FAILURE;
    } 

    // read input data from file
    for (int i = 0; i < J; i++) { // read input data
        char hex[3] = {0}; // buffer for hex string
        if (fread(hex, 1, 2, fin) != 2) { // read two bytes at a time
            fprintf(stderr, "Error reading byte %d\n", i);
            fclose(fin); // close input file
            free(inputData); // free memory for input data
            return EXIT_FAILURE;
        }
        inputData[i] = (unsigned char) strtol(hex, NULL, 16); // convert hex string to byte
    }
    fclose(fin); // close input file

    unsigned char **disks = malloc(N * sizeof(unsigned char *)); // allocate memory for disks
    if (!disks) { // error if memory allocation fails
        perror("Failed to allocate memory for disk array");
        free(inputData); // free memory for input data 
        return EXIT_FAILURE; 
    } 

    for (int d = 0; d < N; d++) { // allocate memory for each disk
        disks[d] = calloc(K, 1); // initialize each disk to zero
        if (!disks[d]) { // error if memory allocation fails
            perror("Failed to allocate memory for disk");
            for (int i = 0; i < d; i++) { // free memory for disks
                free(disks[i]);
            }
            free(disks); // free memory for disk array
            free(inputData); // free memory for input data 
            return EXIT_FAILURE; 
        } 
    }

    int numDataBlocks = J / B; // number of data blocks in input data
    int perStripe = N - 1; // number of data disks in each stripe

    int currentBlock = 0; // track current data block index
    while (currentBlock < numDataBlocks) { // while there are still data blocks to process
        int stripe = currentBlock / perStripe; // calculate stripe index
        int offsetInStripe = currentBlock % perStripe; // calculate offset in stripe

        int parityDisk = (N - 1 - stripe % N + N) % N; // calculate parity disk for this stripe
        int diskIndex = (parityDisk + 1 + offsetInStripe) % N; // calculate data disk index
        int offsetInDisk = stripe * B; // calculate offset in disk

        if (offsetInDisk >= K) break; // don't exceed disk size

        memcpy(disks[diskIndex] + offsetInDisk, inputData +  (currentBlock * B), B); // copy data block to disk

        if (offsetInStripe == perStripe - 1 || currentBlock == numDataBlocks - 1) { // if all data blocks in this stripe are filled
            int stripeStartBlock =  (currentBlock / perStripe) * perStripe; // calculate start block of stripe
            memset(disks[parityDisk] + offsetInDisk, 0, B); // clear parity block first
            for (int i = 0; i < perStripe && stripeStartBlock + i < numDataBlocks; i++) { // iterate over each data block in the stripe
                int dataDiskIndex = (parityDisk + 1 + i) % N; // calculate data disk index
                for (int b = 0; b < B; b++) { // XOR this data block into the parity block
                    disks[parityDisk][offsetInDisk + b] ^= disks[dataDiskIndex][offsetInDisk + b];
                }
            }
        }
        currentBlock++; // increment current data block index
    }

    // write data to output files
    for (int d = 0; d < N; d++) { // iterate over each disk
        FILE *fout = fopen(diskPaths[d], "w"); // open disk file for writing
        if (!fout) { 
            perror(diskPaths[d]); 
            continue; // error if file cannot be opened
        }

        for (int b = 0; b < K; b++) { // write data to disk
            fprintf(fout, "%02x", disks[d][b]); // write each byte as hex string
        }
        
        fclose(fout); // close disk file
        free(disks[d]); // free memory for disk
    }

    // free memory for input data and disks
    free(disks);
    free(inputData);
    return EXIT_SUCCESS; // return success

}