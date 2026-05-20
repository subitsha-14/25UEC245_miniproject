#include <stdio.h>
#include <stdlib.h>

struct clientData
{
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
};

unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);

int main()
{
    FILE *cfPtr;
    unsigned int choice;

    cfPtr = fopen("credit.dat", "rb+");

    if (cfPtr == NULL)
    {
        cfPtr = fopen("credit.dat", "wb+");

        if (cfPtr == NULL)
        {
            printf("File could not be opened.\n");
            return 1;
        }

        struct clientData blank = {0,"","",0.0};

        for(int i=0;i<100;i++)
        {
            fwrite(&blank,
                   sizeof(struct clientData),
                   1,
                   cfPtr);
        }

        rewind(cfPtr);
    }

    while((choice = enterChoice()) != 5)
    {
        switch(choice)
        {
            case 1:
                textFile(cfPtr);
                break;

            case 2:
                updateRecord(cfPtr);
                break;

            case 3:
                newRecord(cfPtr);
                break;

            case 4:
                deleteRecord(cfPtr);
                break;

            default:
                printf("Invalid choice\n");
        }
    }

    fclose(cfPtr);

    return 0;
}

unsigned int enterChoice(void)
{
    unsigned int choice;

    printf("\nBANK MENU\n");
    printf("1. Create text file\n");
    printf("2. Update account\n");
    printf("3. Add account\n");
    printf("4. Delete account\n");
    printf("5. Exit\n");

    printf("Enter choice: ");
    scanf("%u",&choice);

    return choice;
}

void textFile(FILE *readPtr)
{
    FILE *writePtr;
    struct clientData client;

    writePtr = fopen("accounts.txt","w");

    if(writePtr == NULL)
    {
        printf("Cannot create text file.\n");
        return;
    }

    rewind(readPtr);

    fprintf(writePtr,
            "%-6s%-15s%-15s%-10s\n",
            "Acct",
            "LastName",
            "FirstName",
            "Balance");

    while(fread(&client,
                sizeof(struct clientData),
                1,
                readPtr))
    {
        if(client.acctNum != 0)
        {
            fprintf(writePtr,
                    "%-6u%-15s%-15s%.2lf\n",
                    client.acctNum,
                    client.lastName,
                    client.firstName,
                    client.balance);
        }
    }

    fclose(writePtr);

    printf("accounts.txt created successfully.\n");
}

void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;

    struct clientData client={0};

    printf("Enter account number: ");
    scanf("%u",&account);

    fseek(fPtr,
          (account-1)*sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if(client.acctNum == 0)
    {
        printf("Account not found.\n");
        return;
    }

    printf("Current Balance: %.2lf\n",
           client.balance);

    printf("Enter amount(+ deposit / - withdraw): ");
    scanf("%lf",&transaction);

    client.balance += transaction;

    fseek(fPtr,
          -sizeof(struct clientData),
          SEEK_CUR);

    fwrite(&client,
           sizeof(struct clientData),
           1,
           fPtr);

    printf("Balance Updated.\n");
}

void newRecord(FILE *fPtr)
{
    unsigned int account;

    struct clientData client={0};

    printf("Enter account number: ");
    scanf("%u",&account);

    fseek(fPtr,
          (account-1)*sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if(client.acctNum != 0)
    {
        printf("Account already exists.\n");
        return;
    }

    client.acctNum = account;

    printf("Enter Last Name: ");
    scanf("%14s",
          client.lastName);

    printf("Enter First Name: ");
    scanf("%9s",
          client.firstName);

    printf("Enter Balance: ");
    scanf("%lf",
          &client.balance);

    fseek(fPtr,
          (account-1)*sizeof(struct clientData),
          SEEK_SET);

    fwrite(&client,
           sizeof(struct clientData),
           1,
           fPtr);

    printf("Account Added Successfully.\n");
}

void deleteRecord(FILE *fPtr)
{
    unsigned int account;

    struct clientData client;
    struct clientData blank={0};

    printf("Enter account number: ");
    scanf("%u",&account);

    fseek(fPtr,
          (account-1)*sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if(client.acctNum == 0)
    {
        printf("Account not found.\n");
        return;
    }

    fseek(fPtr,
          (account-1)*sizeof(struct clientData),
          SEEK_SET);

    fwrite(&blank,
           sizeof(struct clientData),
           1,
           fPtr);

    printf("Account Deleted Successfully.\n");
}
