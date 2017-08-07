#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <stdint.h>

#define TRANSACTIONS_DIR    "/db/transactions/"

struct transaction
{
    char    *id;
};
typedef struct transaction Transaction;

void            transaction_free(Transaction *a_transaction);
Transaction *   transaction_alloc();
char        *   transaction_dir(
    Transaction *a_transaction
);

#endif

