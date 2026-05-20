#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MIN_BALANCE 500.0
#define CORRECT_PASSWORD "bank123"
#define MAX_ATTEMPTS 3
#define INTEREST_RATE 0.04
#define VIP_THRESHOLD 10000.0
#define VIP_INTEREST_RATE 0.06
#define LOAN_INTEREST_RATE 0.08

struct clientData {
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
    int frozen;
    int vip;
    double loanAmount;
    double loanEMI;
    int loanMonths;
};

unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void searchRecord(FILE *fPtr);
void showStatistics(FILE *fPtr);
void viewTransactionHistory(void);
void logTransaction(unsigned int acctNum, const char *type, double amount, double balanceAfter);
int  getValidAccountNumber(const char *prompt);
double getValidDouble(const char *prompt);
int  getValidString(const char *prompt, char *buf, int maxLen);
void clearInputBuffer(void);
int  checkPassword(void);
void checkMinBalance(struct clientData *client);
void checkVIP(struct clientData *client, FILE *fPtr, int accountNum);
void freezeUnfreezeAccount(FILE *fPtr);
void applyInterest(FILE *fPtr);
void applyLoan(FILE *fPtr);
void payEMI(FILE *fPtr);

int checkPassword(void)
{
    char input[20];
    int attempts = 0;
    while (attempts < MAX_ATTEMPTS) {
        printf("Enter password: ");
        scanf("%19s", input);
        if (strcmp(input, CORRECT_PASSWORD) == 0) { puts("Access granted.\n"); return 1; }
        attempts++;
        printf("Wrong password! %d attempt(s) remaining.\n", MAX_ATTEMPTS - attempts);
    }
    puts("Too many wrong attempts. Program locked.");
    return 0;
}

void checkMinBalance(struct clientData *client)
{
    if (client->balance < MIN_BALANCE)
        printf("*** ALERT: Account #%d balance %.2f is below minimum (%.2f) ***\n",
               client->acctNum, client->balance, MIN_BALANCE);
}

void checkVIP(struct clientData *client, FILE *fPtr, int accountNum)
{
    if (!client->vip && client->balance >= VIP_THRESHOLD) {
        client->vip = 1;
        printf("*** CONGRATS! Account #%d is now a VIP account! (%.0f%% interest rate) ***\n",
               client->acctNum, VIP_INTEREST_RATE * 100);
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(client, sizeof(struct clientData), 1, fPtr);
    } else if (client->vip && client->balance < VIP_THRESHOLD) {
        client->vip = 0;
        printf("*** Account #%d VIP status removed (balance below %.2f) ***\n",
               client->acctNum, VIP_THRESHOLD);
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(client, sizeof(struct clientData), 1, fPtr);
    }
}

void freezeUnfreezeAccount(FILE *fPtr)
{
    struct clientData client = {0};
    int accountNum = getValidAccountNumber("Enter account number to freeze/unfreeze ( 1 - 100 ): ");
    if (accountNum == -1) return;

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0) { printf("Account #%d does not exist.\n", accountNum); return; }

    if (client.frozen) {
        client.frozen = 0;
        printf("Account #%d (%s %s) has been UNFROZEN.\n", client.acctNum, client.firstName, client.lastName);
        logTransaction(client.acctNum, "UNFROZEN", 0.0, client.balance);
    } else {
        client.frozen = 1;
        printf("Account #%d (%s %s) has been FROZEN.\n", client.acctNum, client.firstName, client.lastName);
        logTransaction(client.acctNum, "FROZEN", 0.0, client.balance);
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);
}

void applyInterest(FILE *fPtr)
{
    struct clientData client;
    int count = 0;
    rewind(fPtr);

    printf("\n+----------------------------------------------------+\n");
    printf("|              INTEREST APPLIED                      |\n");
    printf("+----------------------------------------------------+\n");

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1) {
        if (client.acctNum == 0 || client.frozen) continue;
        double rate = client.vip ? VIP_INTEREST_RATE : INTEREST_RATE;
        double interest = client.balance * rate;
        client.balance += interest;
        printf("| Acct#%-4d | %s | Rate:%.0f%% | +%.2f | Bal:%.2f\n",
               client.acctNum, client.vip ? "VIP   " : "NORMAL",
               rate * 100, interest, client.balance);
        logTransaction(client.acctNum, "INTEREST", interest, client.balance);
        checkVIP(&client, fPtr, client.acctNum);
        fseek(fPtr, -sizeof(struct clientData), SEEK_CUR);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
        count++;
    }

    printf("+----------------------------------------------------+\n");
    printf("Interest applied to %d account(s).\n", count);
}

void applyLoan(FILE *fPtr)
{
    struct clientData client = {0};
    int accountNum = getValidAccountNumber("Enter account number for loan ( 1 - 100 ): ");
    if (accountNum == -1) return;

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0) { printf("Account #%d does not exist.\n", accountNum); return; }
    if (client.frozen)       { printf("Account #%d is FROZEN. Cannot apply loan.\n", accountNum); return; }
    if (client.loanAmount > 0) {
        printf("Account #%d already has an active loan of %.2f (EMI: %.2f x %d months remaining).\n",
               accountNum, client.loanAmount, client.loanEMI, client.loanMonths);
        return;
    }

    double principal = getValidDouble("Enter loan amount: ");
    if (principal <= 0) { puts("Invalid loan amount."); return; }

    int months;
    printf("Enter loan duration in months: ");
    scanf("%d", &months);
    if (months <= 0) { puts("Invalid duration."); return; }

    double totalRepay = principal * (1 + LOAN_INTEREST_RATE);
    double emi = totalRepay / months;

    client.loanAmount = totalRepay;
    client.loanEMI    = emi;
    client.loanMonths = months;
    client.balance   += principal;

    printf("\n+------------------------------------------+\n");
    printf("|            LOAN APPROVED                 |\n");
    printf("+------------------------------------------+\n");
    printf("|  Principal    : %-24.2f|\n", principal);
    printf("|  Interest (8%%): %-24.2f|\n", principal * LOAN_INTEREST_RATE);
    printf("|  Total Repay  : %-24.2f|\n", totalRepay);
    printf("|  EMI          : %-24.2f|\n", emi);
    printf("|  Duration     : %-4d months               |\n", months);
    printf("+------------------------------------------+\n");

    logTransaction(client.acctNum, "LOAN_CREDIT", principal, client.balance);
    checkVIP(&client, fPtr, accountNum);

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);
}

void payEMI(FILE *fPtr)
{
    struct clientData client = {0};
    int accountNum = getValidAccountNumber("Enter account number to pay EMI ( 1 - 100 ): ");
    if (accountNum == -1) return;

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0) { printf("Account #%d does not exist.\n", accountNum); return; }
    if (client.loanAmount <= 0) { printf("Account #%d has no active loan.\n", accountNum); return; }
    if (client.balance < client.loanEMI) {
        printf("Insufficient balance (%.2f) to pay EMI (%.2f).\n", client.balance, client.loanEMI);
        return;
    }

    client.balance    -= client.loanEMI;
    client.loanAmount -= client.loanEMI;
    client.loanMonths--;

    printf("EMI of %.2f paid. Remaining loan: %.2f (%d months left).\n",
           client.loanEMI, client.loanAmount > 0 ? client.loanAmount : 0.0, client.loanMonths);

    if (client.loanMonths <= 0 || client.loanAmount <= 0) {
        client.loanAmount = 0;
        client.loanEMI    = 0;
        client.loanMonths = 0;
        puts("Loan fully repaid! Congratulations!");
        logTransaction(client.acctNum, "LOAN_CLOSED", 0.0, client.balance);
    } else {
        logTransaction(client.acctNum, "EMI_PAID", -client.loanEMI, client.balance);
    }

    checkMinBalance(&client);
    checkVIP(&client, fPtr, accountNum);

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);
}

int main(int argc, char *argv[])
{
    FILE *cfPtr;
    unsigned int choice;

    if (!checkPassword()) exit(-1);

    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL) {
        printf("%s: File could not be opened.\n", argv[0]);
        exit(-1);
    }

    while ((choice = enterChoice()) != 11) {
        switch (choice) {
            case 1:  textFile(cfPtr);              break;
            case 2:  updateRecord(cfPtr);          break;
            case 3:  newRecord(cfPtr);             break;
            case 4:  deleteRecord(cfPtr);          break;
            case 5:  searchRecord(cfPtr);          break;
            case 6:  showStatistics(cfPtr);        break;
            case 7:  freezeUnfreezeAccount(cfPtr); break;
            case 8:  applyInterest(cfPtr);         break;
            case 9:  applyLoan(cfPtr);             break;
            case 10: payEMI(cfPtr);                break;
            default: puts("Incorrect choice. Please enter 1-11."); break;
        }
    }

    viewTransactionHistory();
    fclose(cfPtr);
    puts("\nProgram ended. Goodbye!\n");
    return 0;
}

void textFile(FILE *readPtr)
{
    FILE *writePtr;
    int result;
    struct clientData client = {0};

    if ((writePtr = fopen("accounts.txt", "w")) == NULL) { puts("File could not be opened."); return; }

    rewind(readPtr);
    fprintf(writePtr, "%-6s%-16s%-11s%10s%8s%6s%12s\n", "Acct", "Last Name", "First Name", "Balance", "Status", "VIP", "Loan Left");
    fprintf(writePtr, "%-6s%-16s%-11s%10s%8s%6s%12s\n", "----", "---------", "----------", "-------", "------", "---", "---------");

    while (!feof(readPtr)) {
        result = fread(&client, sizeof(struct clientData), 1, readPtr);
        if (result != 0 && client.acctNum != 0)
            fprintf(writePtr, "%-6d%-16s%-11s%10.2f%8s%6s%12.2f\n",
                    client.acctNum, client.lastName, client.firstName, client.balance,
                    client.frozen ? "FROZEN" : "ACTIVE",
                    client.vip    ? "YES"    : "NO",
                    client.loanAmount > 0 ? client.loanAmount : 0.0);
    }

    fclose(writePtr);
    puts("accounts.txt created successfully.");
}

void updateRecord(FILE *fPtr)
{
    double transaction;
    struct clientData client = {0};

    int account = getValidAccountNumber("Enter account to update ( 1 - 100 ): ");
    if (account == -1) return;

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0) { printf("Account #%d has no information.\n", account); return; }
    if (client.frozen)       { printf("Account #%d is FROZEN. Transactions not allowed.\n", account); return; }

    printf("\n%-6d%-16s%-11s%10.2f [%s%s]\n\n", client.acctNum, client.lastName, client.firstName,
           client.balance, client.vip ? "VIP " : "", client.frozen ? "FROZEN" : "ACTIVE");

    transaction = getValidDouble("Enter charge ( + ) or payment ( - ): ");

    if (client.balance + transaction < 0) { printf("Transaction denied! Insufficient balance (%.2f).\n", client.balance); return; }

    client.balance += transaction;
    printf("Updated: %-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

    checkMinBalance(&client);
    checkVIP(&client, fPtr, account);

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);
    logTransaction(client.acctNum, transaction >= 0 ? "CREDIT" : "DEBIT", transaction, client.balance);
}

void newRecord(FILE *fPtr)
{
    struct clientData client = {0};

    int accountNum = getValidAccountNumber("Enter new account number ( 1 - 100 ): ");
    if (accountNum == -1) return;

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum != 0) { printf("Account #%d already contains information.\n", client.acctNum); return; }

    if (!getValidString("Enter last name: ",  client.lastName,  14)) return;
    if (!getValidString("Enter first name: ", client.firstName,  9)) return;

    client.balance = getValidDouble("Enter opening balance: ");
    if (client.balance < 0) { puts("Opening balance cannot be negative."); return; }

    client.acctNum    = accountNum;
    client.frozen     = 0;
    client.loanAmount = 0;
    client.loanEMI    = 0;
    client.loanMonths = 0;
    client.vip        = client.balance >= VIP_THRESHOLD ? 1 : 0;

    if (client.vip) printf("*** Account starts as VIP! (balance >= %.2f) ***\n", VIP_THRESHOLD);

    fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    printf("Account #%d created for %s %s with balance %.2f\n", client.acctNum, client.firstName, client.lastName, client.balance);
    checkMinBalance(&client);
    logTransaction(client.acctNum, "NEW_ACCOUNT", client.balance, client.balance);
}

void deleteRecord(FILE *fPtr)
{
    struct clientData client = {0}, blankClient = {0};

    int accountNum = getValidAccountNumber("Enter account number to delete ( 1 - 100 ): ");
    if (accountNum == -1) return;

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0) { printf("Account %d does not exist.\n", accountNum); return; }
    if (client.loanAmount > 0) { printf("Cannot delete! Account #%d has active loan of %.2f.\n", accountNum, client.loanAmount); return; }

    char confirm;
    printf("Are you sure you want to delete account #%d (%s %s)? (y/n): ", accountNum, client.firstName, client.lastName);
    clearInputBuffer();
    confirm = getchar();

    if (tolower(confirm) != 'y') { puts("Deletion cancelled."); return; }

    logTransaction(client.acctNum, "DELETED", client.balance, 0.0);
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    printf("Account #%d deleted successfully.\n", accountNum);
}

void searchRecord(FILE *fPtr)
{
    struct clientData client;
    int found = 0, searchChoice;
    char searchTerm[20];

    printf("\nSearch by:\n  1 - Account Number\n  2 - Last Name\n  3 - First Name\n? ");

    if (scanf("%d", &searchChoice) != 1 || searchChoice < 1 || searchChoice > 3) {
        puts("Invalid search option."); clearInputBuffer(); return;
    }

    if (searchChoice == 1) {
        int account = getValidAccountNumber("Enter account number to search ( 1 - 100 ): ");
        if (account == -1) return;
        fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
        fread(&client, sizeof(struct clientData), 1, fPtr);
        if (client.acctNum != 0) {
            printf("\n%-6s%-16s%-11s%10s%8s%6s%12s\n", "Acct", "Last Name", "First Name", "Balance", "Status", "VIP", "Loan Left");
            printf("%-6d%-16s%-11s%10.2f%8s%6s%12.2f\n", client.acctNum, client.lastName, client.firstName,
                   client.balance, client.frozen ? "FROZEN" : "ACTIVE", client.vip ? "YES" : "NO",
                   client.loanAmount > 0 ? client.loanAmount : 0.0);
            found = 1;
        }
    } else {
        getValidString(searchChoice == 2 ? "Enter last name: " : "Enter first name: ", searchTerm, 19);
        for (int i = 0; searchTerm[i]; i++) searchTerm[i] = tolower(searchTerm[i]);
        rewind(fPtr);
        printf("\n%-6s%-16s%-11s%10s%8s%6s%12s\n", "Acct", "Last Name", "First Name", "Balance", "Status", "VIP", "Loan Left");
        while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1) {
            if (client.acctNum == 0) continue;
            char nameCopy[20];
            strncpy(nameCopy, searchChoice == 2 ? client.lastName : client.firstName, 19);
            nameCopy[19] = '\0';
            for (int i = 0; nameCopy[i]; i++) nameCopy[i] = tolower(nameCopy[i]);
            if (strstr(nameCopy, searchTerm)) {
                printf("%-6d%-16s%-11s%10.2f%8s%6s%12.2f\n", client.acctNum, client.lastName, client.firstName,
                       client.balance, client.frozen ? "FROZEN" : "ACTIVE", client.vip ? "YES" : "NO",
                       client.loanAmount > 0 ? client.loanAmount : 0.0);
                found = 1;
            }
        }
    }

    if (!found) puts("No matching records found.");
    else puts("Search complete.");
}

void showStatistics(FILE *fPtr)
{
    struct clientData client;
    int totalAccounts = 0, negativeCount = 0, frozenCount = 0, vipCount = 0, loanCount = 0;
    double totalBalance = 0.0, maxBalance = -1e18, minBalance = 1e18, totalLoan = 0.0;
    struct clientData richest = {0}, poorest = {0};

    rewind(fPtr);
    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1) {
        if (client.acctNum == 0) continue;
        totalAccounts++;
        totalBalance += client.balance;
        if (client.balance > maxBalance) { maxBalance = client.balance; richest = client; }
        if (client.balance < minBalance) { minBalance = client.balance; poorest = client; }
        if (client.balance  < 0)   negativeCount++;
        if (client.frozen)         frozenCount++;
        if (client.vip)            vipCount++;
        if (client.loanAmount > 0) { loanCount++; totalLoan += client.loanAmount; }
    }

    if (totalAccounts == 0) { puts("No accounts found."); return; }

    printf("\n+------------------------------------------+\n");
    printf("|         BANK STATISTICS REPORT           |\n");
    printf("+------------------------------------------+\n");
    printf("|  Total Accounts   : %-21d|\n", totalAccounts);
    printf("|  VIP Accounts     : %-21d|\n", vipCount);
    printf("|  Frozen Accounts  : %-21d|\n", frozenCount);
    printf("|  Active Loans     : %-21d|\n", loanCount);
    printf("|  Total Loan Due   : %-21.2f|\n", totalLoan);
    printf("|  Total Balance    : %-21.2f|\n", totalBalance);
    printf("|  Average Balance  : %-21.2f|\n", totalBalance / totalAccounts);
    printf("|  Highest Balance  : %-21.2f|\n", maxBalance);
    printf("|  Lowest Balance   : %-21.2f|\n", minBalance);
    printf("|  Negative Balances: %-21d|\n", negativeCount);
    printf("+------------------------------------------+\n");
    printf("|  Richest: #%-3d %-12s %-10s  |\n", richest.acctNum, richest.lastName, richest.firstName);
    printf("|  Poorest: #%-3d %-12s %-10s  |\n", poorest.acctNum, poorest.lastName, poorest.firstName);
    printf("+------------------------------------------+\n");
}

void logTransaction(unsigned int acctNum, const char *type, double amount, double balanceAfter)
{
    FILE *logPtr = fopen("transactions.log", "a");
    if (!logPtr) return;
    time_t now = time(NULL);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(logPtr, "[%s] Acct#%-4d | %-14s | Amount: %+10.2f | Balance: %10.2f\n",
            timestamp, acctNum, type, amount, balanceAfter);
    fclose(logPtr);
}

void viewTransactionHistory(void)
{
    FILE *logPtr = fopen("transactions.log", "r");
    if (!logPtr) { puts("\nNo transaction history found."); return; }
    puts("\n+------------------------------------------------------------------+");
    puts("|                   TRANSACTION HISTORY
