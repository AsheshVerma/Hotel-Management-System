#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define MAX_ORDER 100
#define MAX_CUSTOMER_NUMBER 5
#define TABLE_WAITER_SHM_ID_BASE 100

void print_menu() {
    
    printf("        MENU     \n");
    printf("1. Veg Burger 30 INR\n");
    printf("2. Chicken Burger 40 INR\n");
    printf("3. Ostrich Eggs 25 INR\n");
    printf("4. Egg Frankie 30 INR\n");
    printf("\n");
}

void convert_to_1d(int row, int cols, int a[][100], int arr[]) {
    int z = 0;
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < cols; j++) {
            arr[z] = a[i][j];
            z++;
        }
    }
    arr[z] = -69;
    arr[z + 1] = -100;
    arr[z+2]=-1000;
}

int main() {
    int table_num, customer_num;
    printf("Enter Table Number:\n");
    scanf("%d", &table_num);
    printf("Enter Number of Customers at Table (maximum no. of customers can be 5):\n");
    scanf("%d", &customer_num);
    print_menu();
    while (1) { 
     
        int table_RX_order[MAX_CUSTOMER_NUMBER][MAX_ORDER] = {0};//2d arr to rx order
        int arr[(MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2] = {0};    //convert that order to 1d arr   

        int pipefd[customer_num][2];//2d arr for multiple pipes

        for (int i = 0; i < customer_num; i++) {
            if (pipe(pipefd[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < customer_num; i++) {
            pid_t customer_id = fork();// making customer proc

            if (customer_id < 0) {
                perror("fork fail ERR");
                exit(1);
            }

            wait(NULL);

            if (customer_id == 0) {
                int food_item_no, j = 0;
                int customer_order_TX[MAX_ORDER] = {0};                
                printf("Customer no. %d\n", i+1);
                printf("Enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done:\n");

                while (1) { //taking food order for a customer
                    scanf("%d", &food_item_no);

                    if (food_item_no == -1)
                        break;

                    customer_order_TX[j] = food_item_no;
                    j++;
                }

                close(pipefd[i][0]);
                write(pipefd[i][1], customer_order_TX, sizeof(customer_order_TX));
                close(pipefd[i][1]);

                exit(EXIT_SUCCESS);//terminating customer proc
            }
        }

        for (int i = 0; i < customer_num; i++) {
            close(pipefd[i][1]);
            read(pipefd[i][0], table_RX_order[i], sizeof(table_RX_order[i]));//reading tabletxorder into tablerxorder by main proc
            close(pipefd[i][0]);
        }

        convert_to_1d(MAX_CUSTOMER_NUMBER, MAX_ORDER, table_RX_order, arr);

        int shm_id = 0;

        shm_id = shmget(TABLE_WAITER_SHM_ID_BASE + table_num, sizeof(int) * (MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2, 0666 | IPC_CREAT);

        if (shm_id < 0) {
            perror("shmget (connect channel)");
            exit(EXIT_FAILURE);
        }

        int *sharedDataPtr = (int *)shmat(shm_id, NULL, 0);

        if (sharedDataPtr == (int *) -1) {
            perror("shmat");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < (MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2; i++) {
            sharedDataPtr[i] = arr[i];
        }

        while (sharedDataPtr[(MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2 - 2] == -69) {
            sleep(1);//to avoid continous busy wait
        }

        //printf("Res bool %d\n", sharedDataPtr[(MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2 - 2]);

        if (sharedDataPtr[(MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2 - 2] == 1) {
            shmdt(sharedDataPtr);
            continue;
        } else {
            printf("Enter Number of Customers at Table (maximum no. of customers can be 5):\n");
            int choice;
            scanf("%d", &choice);//taking choice of customer if they wanna order more

            if (choice == -1) {
                sharedDataPtr[(MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2 - 1] = choice;
                shmdt(sharedDataPtr);
                printf("Customers leave the table...\n");
                exit(EXIT_SUCCESS);
            } else if (choice >= 1 && choice <= 5) {
                customer_num=choice;
                sharedDataPtr[(MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2 - 1] = choice;
                shmdt(sharedDataPtr);
                customer_num = choice;
            }
        }
    }
    

    return 0;
}
