/**
 * Build with: gcc main.c -o hmac -lssl -lcrypto
 * Example usage:
 *  ./hmac wordlist.txt sha1hash
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include "hash.h"

clock_t begin;
pthread_mutex_t lock;

static int verbose_flag = 0;
static int newlines_flag = 0;
static char *format;

FILE *blob;
char *wordlist;
char *target_hash;
char *all_chars = "0123456789abcdef";
int count = 0;
void (*hash_fun_ptr)(char *, char *);
int found = 0;
int applied_hashing = 1;
int hash_length = 0;

static struct option long_options[] = {
    { "verbose",  no_argument,       0,               1  },
    { "format",   required_argument, 0,              'f' },
    { "wordlist", required_argument, 0,              'w' },
    { "target",   required_argument, 0,              't' },
    { "amount",   required_argument, 0,              'a' },
    { "newlines", no_argument,       0,              'n' },
    { "help",     no_argument,       0,              'h' },
    {  0,         0,                 0,               0  }
};

/**
 * Returns the size of the blob.
 * @param char[] The message line.
 */
void manage_read(char msg_line[100])
{
    // Read a line from the blob.
    fgets(msg_line, 100, blob);

    // Return if the end of the line was reached.
    if (msg_line == NULL) {
        return;
    }
}

/**
 * This is the worker callback.
 * @param ptr The input to the worker's callback.
 * 
 */
void *worker_callback(void *ptr)
{
    char msg_line[256];

    pthread_mutex_lock(&lock);
    manage_read(msg_line);
    pthread_mutex_unlock(&lock);

    while (msg_line && found == 0) {
        if (found == 1) {
            pthread_exit(0);
            return 0;
        }

        if (strlen(msg_line) == 0) {
            pthread_mutex_lock(&lock);
            manage_read(msg_line);
            pthread_mutex_unlock(&lock);
            continue;
        }

        // Eliminate the newline at the end.
        if (newlines_flag == 0 && msg_line) {
            msg_line[strlen(msg_line) - 1] = '\0';
        }

        // Allocate the appropriate memory for the 
        char hash[hash_length];
        hash_fun_ptr(msg_line, hash);

        // Re-run the hash function on, but on the hash itself this time.
        if (applied_hashing > 1) {
            for (int m = 0; m < applied_hashing - 1; m++) {
                hash_fun_ptr(hash, hash);
            }
        }

        if (verbose_flag != 0) {
            printf("%s -> %s%c", hash, msg_line, newlines_flag == 0 ? '\n' : '\0');
        }

        count++;

        // Compare the produced hash to the target hash.
        if (strcmp(target_hash, hash) == 0) {
            if (newlines_flag != 0) {
                printf("The result is (with a newline): %s", msg_line);
            } else {
                printf("The result is (without a newline): %s\n", msg_line);
            }

            // Get the execution time.
            clock_t end = clock();
            double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

            printf("Found it after %i tries and %f seconds!\n", count, time_spent);
            printf("The speed was %.2f KHs\n", count / time_spent / 1000);

            // Close the blob file.
            if (blob) {
                fclose(blob);
            }

            exit(0);

            // The thing below causes an error when run. Figure dat shit out.

            // Signal the other threads that we're donezo.
            pthread_mutex_lock(&lock);
            found = 1;
            pthread_mutex_unlock(&lock);

            break;
        }

        pthread_mutex_lock(&lock);
        manage_read(msg_line);
        pthread_mutex_unlock(&lock);
    }

    // Exit the thread.
    pthread_exit(0);

    return 0;
}

/**
 * Prints the help message.
 */
void print_help()
{
    printf("hashcrack\n\n");
    printf("Usage:   hashcrack [options]\n\n");
    printf("Options: --wordlist, -w [filename]: The wordlist to check against\n");
    printf("         --format,   -f [hash_fun]: The hash function (md5|sha|sha1|sha256|sha512)\n");
    printf("         --target,   -t [hash]:     The hash to check\n");
    printf("         --amount,   -a [amount]:   Amount of hashing\n");
    printf("         --newlines, -n:            Whether to hash with newlines\n");
    printf("         --help,     -h:            Prints this message\n\n");
    printf("Examples:\n");
    printf("  Crack the MD5 hash of the word 'test'\n");
    printf("    hashcrack -f md5 -t 098f6bcd4621d373cade4e832627b4f6 -w /path/to/wordlist\n");
    printf("  Crack the double MD5 hash of the word 'test'\n");
    printf("    hashcrack -f md5 -t fb469d7ef430b0baf0cab6c436e70375 -a 2 -w /path/to/wordlist\n");
    printf("  Crack the MD5 hash of the word 'test' that's followed by a newline\n");
    printf("    hashcrack -f md5 -t d8e8fca2dc0f896fd7cb4cb0031ba249 -n -w /path/to/wordlist\n\n");
}

/**
 * Main entry to the program.
 *
 * @param c The argument flag.
 * @param option_index The index of the option.
 */
void process_args(int c, int option_index)
{
    switch(c) {
        case 0:
            break;
        case 'f':
            format = optarg;

            if (strcmp(format, "sha512") == 0) {
                hash_fun_ptr = &sha512;
                hash_length = 128;
            }
            else if (strcmp(format, "sha256") == 0) {
                hash_fun_ptr = &sha256;
                hash_length = 65;
            }
            else if (strcmp(format, "sha1") == 0) {
                hash_fun_ptr = &sha1;
                hash_length = 40;
            }
            else if (strcmp(format, "md5") == 0) {
                hash_fun_ptr = &md5;
                hash_length = 33;
            }
            else {
                printf("Unknown format %s\n", format);
                exit(1);
            }

            break;
        case 'w':
            wordlist = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
            strcpy(wordlist, optarg);
            break;
        case 't':
            target_hash = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
            strcpy(target_hash, optarg);
            break;
        case 'n':
            newlines_flag = 1;
            break;
        case 'a':
            applied_hashing = atoi(optarg);

            if (applied_hashing < 1) {
                applied_hashing = 1;
            }

            break;
        case 'v':
            verbose_flag = 1;
            break;
        case 'h':
            print_help();
            exit(1);
    }
}

/**
 * Main entry to the program.
 *
 * @param argc The argument count.
 * @param argv The arguments list.
 * @return The status code of the program.
 */
int main(int argc, char **argv)
{
    begin = clock();
    int option_index = 0;

    // Get the number of cores.
    long int cores = sysconf(_SC_NPROCESSORS_ONLN);

    printf("Creating %ld threads\n", cores);

    pthread_mutex_init(&lock, NULL);
    int c;

    while ((c = getopt_long(argc, argv, "f:w:t:a:nhv", long_options, &option_index)) != -1) {
        process_args(c, option_index);
    }

    // If something is missing print the help message.
    if (!wordlist || !format || !target_hash) {
        print_help();
        return 1;
    }

    printf("Reading %s and producing %s\n", wordlist, format);

    if (newlines_flag != 0) {
        printf("With newlines.\n");
    }

    if (applied_hashing > 1) {
        printf("Hashing %i times.\n", applied_hashing);
    }

    FILE *msgs = fopen(wordlist, "r");
    blob = msgs;

    if (msgs == NULL) {
        printf("Invalid wordlist file\n");
        return 1;
    }

    // Create as many threads as there are cores.
    for (int i = 0; i < cores; i++) {
        pthread_t worker;

        // Create the thread.
        pthread_create(&worker, NULL, worker_callback, NULL);
    }

    pthread_exit(NULL);

    return 0;
}
