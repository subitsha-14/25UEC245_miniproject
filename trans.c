#include <stdio.h>

struct Bank
{
    int accNo;
    char name[30];
    float balance;
};

int main()
{
    struct Bank user;
    int choice;
    float amount;

    printf("Enter Account Number: ");
    scanf("%d", &user.accNo);

    printf("Enter Name: ");
    scanf("%s", user.name);

    printf("Enter Initial Balance: ");
    scanf("%f", &user.balance);

    while(1)
    {
        printf("\n--- BANK MENU ---\n");
        printf("1. Deposit\n");
        printf("2. Withdraw\n");
        printf("3. Check Balance\n");
        printf("4. Exit\n");

        printf("Enter Choice: ");
        scanf("%d", &choice);

        switch(choice)
        {
            case 1:
                printf("Enter Deposit Amount: ");
                scanf("%f", &amount);

                user.balance += amount;

                printf("Deposit Successful!\n");
                break;

            case 2:
                printf("Enter Withdraw Amount: ");
                scanf("%f", &amount);

                if(amount <= user.balance)
                {
                    user.balance -= amount;
                    printf("Withdrawal Successful!\n");
                }
                else
                {
                    printf("Insufficient Balance!\n");
                }
                break;

            case 3:
                printf("\nAccount Number : %d\n", user.accNo);
                printf("Name           : %s\n", user.name);
                printf("Balance        : %.2f\n", user.balance);
                break;

            case 4:
                printf("Thank You!\n");
                return 0;

            default:
                printf("Invalid Choice!\n");
        }
    }

    return 0;
}
