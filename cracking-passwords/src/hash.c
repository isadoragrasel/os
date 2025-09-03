#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include "hash_functions.h"

#define KEEP 16 // only the first 16 bytes of a hash are kept
#define NUM_THREADS 11

struct cracked_hash {
    char hash[2 * KEEP + 1];
    char *password, *alg;
};

// Struct to hold thread data
typedef struct {
    char **passwords;
    struct cracked_hash *cracked_hashes;
    int start, stop, n_hashed;
} thread_data_t;

typedef unsigned char *(*hashing)(unsigned char *, unsigned int);

int n_algs = 4;
hashing fn[4] = {calculate_md5, calculate_sha1, calculate_sha256, calculate_sha512};
char *algs[4] = {"MD5", "SHA1", "SHA256", "SHA512"};

// Added: Lookup table for hexadecimal digits.
static const char *hex_digits = "0123456789abcdef";

// Compare hashes function
int compare_hashes(const char *a, const char *b) {
    return (memcmp(a, b, 2 * KEEP) == 0);
}

//Function Name: thr_func
//Description: We divide the scanning of the file into 11 parts.
//             Each thread processes its assigned block of candidate passwords.
void *thr_func(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    char hex_hash[2 * KEEP + 1];

    for (int i = data->start; i < data->stop; i++) {
        char *password = data->passwords[i];

        for (int alg = 0; alg < n_algs; alg++) {
            unsigned char *hash = fn[alg]((unsigned char *)password, strlen(password));
            // Optimized Hex Conversion using a lookup table instead of sprintf:
            for (int j = 0; j < KEEP; j++) {
                unsigned char byte = hash[j];
                hex_hash[2 * j]     = hex_digits[byte >> 4];
                hex_hash[2 * j + 1] = hex_digits[byte & 0x0F];
            }
            hex_hash[2 * KEEP] = '\0';

            for (int j = 0; j < data->n_hashed; j++) {
                if (data->cracked_hashes[j].password != NULL)
                    continue;

                if (compare_hashes(hex_hash, data->cracked_hashes[j].hash)) {
                    if (data->cracked_hashes[j].password == NULL) {
                        data->cracked_hashes[j].password = strdup(password);
                        data->cracked_hashes[j].alg = algs[alg];
                    }
                    break;
                }
            }
            free(hash);
        }
    }
    return NULL;
}

// Function name: crack_hashed_passwords
// Description: Computes different hashes for each password in the password list,
//              then compares them to the hashed passwords to decide whether any of them
//              matches. When multiple passwords match the same hash, only the first one
//              in the list is printed.
void crack_hashed_passwords(char *password_list, char *hashed_list, char *output) {
    FILE *fp;
    char password[256];  // passwords have at most 255 characters
    char hex_hash[2 * KEEP + 1]; // hashed passwords have at most 'KEEP' bytes

    // Load hashed passwords
    int n_hashed = 0;
    struct cracked_hash *cracked_hashes;
    fp = fopen(hashed_list, "r");
    assert(fp != NULL);
    while (fscanf(fp, "%s", hex_hash) == 1)
        n_hashed++;
    rewind(fp);
    cracked_hashes = (struct cracked_hash *)malloc(n_hashed * sizeof(struct cracked_hash));
    assert(cracked_hashes != NULL);
    for (int i = 0; i < n_hashed; i++) {
        fscanf(fp, "%s", cracked_hashes[i].hash);
        cracked_hashes[i].password = NULL;
        cracked_hashes[i].alg = NULL;
    }
    fclose(fp);

    // Load candidate passwords with dynamic reallocation
    int password_count = 0;
    int capacity = 100;
    char **passwords = malloc(capacity * sizeof(char *));
    assert(passwords != NULL);

    fp = fopen(password_list, "r");
    assert(fp != NULL);
    while (fscanf(fp, "%255s", password) == 1) {
        if (password_count == capacity) {
            capacity *= 2;
            passwords = realloc(passwords, capacity * sizeof(char *));
            assert(passwords != NULL);
        }
        passwords[password_count++] = strdup(password);
    }
    fclose(fp);

    pthread_t threads[NUM_THREADS];
    thread_data_t thr_data[NUM_THREADS];

    // Divide the candidate passwords evenly among the threads.
    int buffer_size = password_count / NUM_THREADS;
    for (int i = 0; i < NUM_THREADS; i++) {
        thr_data[i].passwords = passwords;
        thr_data[i].cracked_hashes = cracked_hashes;
        thr_data[i].start = i * buffer_size;
        thr_data[i].stop = (i == NUM_THREADS - 1) ? password_count : (i + 1) * buffer_size;
        thr_data[i].n_hashed = n_hashed;
        pthread_create(&threads[i], NULL, thr_func, &thr_data[i]);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print results to output file
    fp = fopen(output, "w");
    assert(fp != NULL);
    for (int i = 0; i < n_hashed; i++) {
        if (cracked_hashes[i].password == NULL)
            fprintf(fp, "not found\n");
        else
            fprintf(fp, "%s:%s\n", cracked_hashes[i].password, cracked_hashes[i].alg);
    }
    fclose(fp);

    // Release allocated memory
    for (int i = 0; i < n_hashed; i++)
        free(cracked_hashes[i].password);
    free(cracked_hashes);
    for (int i = 0; i < password_count; i++)
        free(passwords[i]);
    free(passwords);
}