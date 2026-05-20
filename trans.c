// Bank Account Management System using Random Access File

#include <stdio.h>
#include <stdlib.h>

#define TOTAL_RECORDS 100

// structure definition
struct clientData
{
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
};

// function prototypes
unsigned int enterChoice(void);
void initializeFile(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void displayRecords(FILE *fPtr);

int main()
{
    FILE *cfPtr;
    unsigned int choice;

    // create file if not exists
    initializeFile();

    // open file
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("File could not be opened.\n");
        exit(1);
    }

    // menu loop
    while ((choice = enterChoice()) != 6)
    {
        switch (choice)
        {
        case 1:
            displayRecords(cfPtr);
            break;

        case 2:
            textFile(cfPtr);
            break;

        case 3:
            updateRecord(cfPtr);
            break;

        case 4:
            newRecord(cfPtr);
            break;

        case 5:
            deleteRecord(cfPtr);
            break;

        default:
            printf("Invalid choice.\n");
        }
    }

    fclose(cfPtr);

    printf("\nProgram terminated successfully.\n");

    return 0;
}

// initialize empty file with 100 blank records
void initializeFile(void)
{
    FILE *filePtr;

    struct clientData blankClient = {0, "", "", 0.0};

    filePtr = fopen("credit.dat", "rb");

    // if file already exists
    if (filePtr != NULL)
    {
        fclose(filePtr);
        return;
    }

    // create new file
    filePtr = fopen("credit.dat", "wb");

    if (filePtr == NULL)
    {
        printf("Unable to create file.\n");
        exit(1);
    }

    for (int i = 0; i < TOTAL_RECORDS; i++)
    {
        fwrite(&blankClient, sizeof(struct clientData), 1, filePtr);
    }

    fclose(filePtr);
}

// menu
unsigned int enterChoice(void)
{
    unsigned int choice;

    printf("\n========== BANK ACCOUNT MENU ==========\n");
    printf("1 - Display all accounts\n");
    printf("2 - Store formatted text file\n");
    printf("3 - Update an account\n");
    printf("4 - Add a new account\n");
    printf("5 - Delete an account\n");
    printf("6 - Exit\n");
    printf("Enter your choice : ");

    scanf("%u", &choice);

    return choice;
}

// display all records
void displayRecords(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};

    rewind(fPtr);

    printf("\n%-10s %-15s %-15s %-10s\n",
           "Account", "Last Name", "First Name", "Balance");

    printf("------------------------------------------------------\n");

    while (fread(&client, sizeof(struct clientData), 1, fPtr))
    {
        if (client.acctNum != 0)
        {
            printf("%-10u %-15s %-15s %-10.2f\n",
                   client.acctNum,
                   client.lastName,
                   client.firstName,
                   client.balance);
        }
    }
}

// create text file
void textFile(FILE *readPtr)
{
    FILE *writePtr;

    struct clientData client = {0, "", "", 0.0};

    writePtr = fopen("accounts.txt", "w");

    if (writePtr == NULL)
    {
        printf("Text file could not be created.\n");
        return;
    }

    rewind(readPtr);

    fprintf(writePtr,
            "%-10s %-15s %-15s %-10s\n",
            "Account",
            "Last Name",
            "First Name",
            "Balance");

    while (fread(&client, sizeof(struct clientData), 1, readPtr))
    {
        if (client.acctNum != 0)
        {
            fprintf(writePtr,
                    "%-10u %-15s %-15s %-10.2f\n",
                    client.acctNum,
                    client.lastName,
                    client.firstName,
                    client.balance);
        }
    }

    fclose(writePtr);

    printf("accounts.txt created successfully.\n");
}

// update existing account
void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;

    struct clientData client = {0, "", "", 0.0};

    printf("Enter account number to update (1-100): ");
    scanf("%u", &account);

    if (account < 1 || account > 100)
    {
        printf("Invalid account number.\n");
        return;
    }

    fseek(fPtr,
          (account - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account does not exist.\n");
    }
    else
    {
        printf("\nCurrent Details:\n");

        printf("%u %s %s %.2f\n",
               client.acctNum,
               client.lastName,
               client.firstName,
               client.balance);

        printf("Enter amount (+ deposit / - withdraw): ");
        scanf("%lf", &transaction);

        client.balance += transaction;

        fseek(fPtr,
              -sizeof(struct clientData),
              SEEK_CUR);

        fwrite(&client,
               sizeof(struct clientData),
               1,
               fPtr);

        printf("Account updated successfully.\n");
    }
}

// add new record
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};

    unsigned int accountNum;

    printf("Enter new account number (1-100): ");
    scanf("%u", &accountNum);

    if (accountNum < 1 || accountNum > 100)
    {
        printf("Invalid account number.\n");
        return;
    }

    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if (client.acctNum != 0)
    {
        printf("Account already exists.\n");
    }
    else
    {
        printf("Enter Last Name : ");
        scanf("%14s", client.lastName);

        printf("Enter First Name : ");
        scanf("%9s", client.firstName);

        printf("Enter Balance : ");
        scanf("%lf", &client.balance);

        client.acctNum = accountNum;

        fseek(fPtr,
              (accountNum - 1) * sizeof(struct clientData),
              SEEK_SET);

        fwrite(&client,
               sizeof(struct clientData),
               1,
               fPtr);

        printf("Account added successfully.\n");
    }
}

// delete record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;
    struct clientData blankClient = {0, "", "", 0.0};

    unsigned int accountNum;

    printf("Enter account number to delete (1-100): ");
    scanf("%u", &accountNum);

    if (accountNum < 1 || accountNum > 100)
    {
        printf("Invalid account number.\n");
        return;
    }

    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if (client.acctNum == 0)
    {
        printf("Account does not exist.\n");
    }
    else
    {
        fseek(fPtr,
              (accountNum - 1) * sizeof(struct clientData),
              SEEK_SET);

        fwrite(&blankClient,
               sizeof(struct clientData),
               1,
               fPtr);

        printf("Account deleted successfully.\n");
    }
}
