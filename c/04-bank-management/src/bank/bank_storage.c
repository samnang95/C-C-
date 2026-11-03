#include "bank/bank.h"

#include <stdio.h>
#include <string.h>

#define BANK_FILE_MAGIC   0x42414E4BU /* 'BANK' */
#define BANK_FILE_VERSION 1U

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint64_t count;
} BankFileHeader;

BankStatus bank_storage_save(const BankList *list, const char *filepath) {
    if (list == NULL || filepath == NULL) {
        return BANK_ERR_NULL;
    }

    FILE *fp = fopen(filepath, "wb");
    if (fp == NULL) {
        return BANK_ERR_FILE_IO;
    }

    BankFileHeader header;
    memset(&header, 0, sizeof(header));
    header.magic = BANK_FILE_MAGIC;
    header.version = BANK_FILE_VERSION;
    header.count = (uint64_t)list->count;

    if (fwrite(&header, sizeof(header), 1, fp) != 1) {
        fclose(fp);
        return BANK_ERR_FILE_IO;
    }

    if (list->count > 0) {
        if (fwrite(list->items, sizeof(BankAccount), list->count, fp) != list->count) {
            fclose(fp);
            return BANK_ERR_FILE_IO;
        }
    }

    if (fclose(fp) != 0) {
        return BANK_ERR_FILE_IO;
    }
    return BANK_OK;
}

BankStatus bank_storage_load(BankList *list, const char *filepath) {
    if (list == NULL || filepath == NULL) {
        return BANK_ERR_NULL;
    }

    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL) {
        return BANK_ERR_FILE_IO;
    }

    BankFileHeader header;
    if (fread(&header, sizeof(header), 1, fp) != 1) {
        fclose(fp);
        return BANK_ERR_FILE_IO;
    }

    if (header.magic != BANK_FILE_MAGIC || header.version != BANK_FILE_VERSION) {
        fclose(fp);
        return BANK_ERR_FILE_IO;
    }

    for (uint64_t i = 0; i < header.count; i++) {
        BankAccount acc;
        if (fread(&acc, sizeof(acc), 1, fp) != 1) {
            fclose(fp);
            return BANK_ERR_FILE_IO;
        }
        /* Ensure owner_name is always null-terminated even if file was modified externally */
        acc.owner_name[BANK_OWNER_MAX - 1] = '\0';

        if (bank_account_is_valid(&acc)) {
            BankStatus status = bank_list_create(list, &acc);
            if (status == BANK_ERR_NOMEM) {
                fclose(fp);
                return BANK_ERR_NOMEM;
            }
        }
    }

    fclose(fp);
    return BANK_OK;
}

