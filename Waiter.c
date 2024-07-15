#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include<fcntl.h>
#include<sys/file.h>
#include<errno.h>

#define MAX_ORDER 100
#define MAX_CUSTOMER_NUMBER 5
#define TABLE_WAITER_SHM_ID_BASE 100
#define WAITER_MANAGER_SHM_ID_base 200

// Function prototypes
int total_bill_calc(int row, int arr[])
{
    int total_bill=0;

    for (int i = 0; i < row; i++)
    {       
        if(arr[i]!=0)
        {
            if(arr[i]==1)
            total_bill=total_bill+30;

            else if(arr[i]==2)
            total_bill=total_bill+40;

            else if(arr[i]==3)
            total_bill=total_bill+25;

            else if(arr[i]==4)
            total_bill=total_bill+30;

            else;
        }        
    }
    return total_bill;    
}

int check_order(int row, int waiter_RX_order[])
{
    int r=0;

    for(int i=0;i<row;i++)
    {
        if((waiter_RX_order[i]<1||waiter_RX_order[i]>4)&&waiter_RX_order[i]!=0)
        {
            r=1;//ERROR IN ORDER FOUND
            break;
        }       
    }    
    return r;
}

int main() {
    int waiter_ID, bill = 0;
    int waiter_RX_order[(MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2] = {0};
    int shm_id,total_earning=0;
    int shm_id_num=0;      
    printf("Enter Waiter ID:\n");
    scanf("%d", &waiter_ID);
    while (1) {
               
        // printf("Enter Waiter ID:\n");
        // scanf("%d", &waiter_ID);
        /*****INITIATION OF SHM WITH MANAGER*/            
        shm_id_num = shmget(WAITER_MANAGER_SHM_ID_base + waiter_ID , sizeof(int)*4, 0666 | IPC_CREAT);
        if (shm_id_num < 0) 
        {
            perror("shmget (connect channel) OPPPSSS");
            exit(EXIT_FAILURE);
        }

        int *sharedDataptr_MANAGER = (int *)shmat(shm_id_num, NULL, 0);

        if (sharedDataptr_MANAGER == (int *) -1) 
        {
            perror("shmat");
            exit(EXIT_FAILURE);
        }
        sharedDataptr_MANAGER[0]=waiter_ID;
        sharedDataptr_MANAGER[1]=total_earning;
        sharedDataptr_MANAGER[2]=0;//tooooo knoow if manngeeer reaad oor not
        sharedDataptr_MANAGER[3]=0;//to let manager that final earning of a table is calc anf u can read shm
        //printf("cmmoonnn manager %d",sharedDataptr_MANAGER[3]);//shou;d print 0 ideally
        
       
        /*****END OF SHM WITH MANAGER*/
        //printf(":maanger shm inti\n");
        shm_id = shmget(TABLE_WAITER_SHM_ID_BASE + waiter_ID, sizeof(int) * (MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2, 0666 | IPC_CREAT);//will connect to that shm with same tableid

        if (shm_id < 0) {
            perror("shmget (connect channel)");
            exit(EXIT_FAILURE);
        }

        int *sharedDataptr = (int *)shmat(shm_id, NULL, 0);

        if (sharedDataptr == (int *) -1) {
            perror("shmat");
            exit(EXIT_FAILURE);
        }        
        while (sharedDataptr[500]==0)//waiter will wait till order is taken by table
        {
            //printf("waitiing......");
            sleep(1);
        }
        
        for (int i = 0; i < (MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2; i++) {
            waiter_RX_order[i] = sharedDataptr[i];//taking customer order from shm
        }
            
        // printf("waiter print this cmmon %d",sharedDataptr[500]);
        // printf("Waiter reached to check order\n");

        int res = check_order((MAX_CUSTOMER_NUMBER * MAX_ORDER), waiter_RX_order);

        //printf("Res is %d\n", res);

        if (res == 0) {
            bill = total_bill_calc((MAX_CUSTOMER_NUMBER * MAX_ORDER), waiter_RX_order);                   

            total_earning+=bill;
            printf("The bill amount for your last order is %d INR.\n", bill);
            sharedDataptr[(MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2 - 2] = res;
        } else {
            printf("Invalid order\n");
            sharedDataptr[(MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2 - 2] = res;
            shmdt(sharedDataptr);
            //sleep(10);
            shmctl(shm_id, IPC_RMID, NULL);
            continue;//will continue for new order
        }

        while (sharedDataptr[(MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2 - 1] == -100) {    //while table is choosing to reorder or not        
            sleep(1);  // Introduce a short delay to avoid busy-waiting
        }

        if (sharedDataptr[(MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2 - 1] >= 1 &&
            sharedDataptr[(MAX_CUSTOMER_NUMBER * MAX_ORDER) + 2 - 1] <= 5) {
            printf("Waiter is taking next order\n");
            sleep(1);  // Add a short delay before detaching shared memory to ensure proper synchronization
            shmdt(sharedDataptr);//clearing shm but this proc will only continue
            shmctl(shm_id, IPC_RMID, NULL);
            continue;
        } 
        else 
        {
            //printf("Waiter terminating\n");
            sleep(1);  // Add a short delay before detaching shared memory to ensure proper synchronization
            shmdt(sharedDataptr);
            shmctl(shm_id, IPC_RMID, NULL);
            //exit(EXIT_SUCCESS); 
            printf("Total bill amount %d INR\n",total_earning);           
            sharedDataptr_MANAGER[1]=total_earning;
            sharedDataptr_MANAGER[3]=699;//ready to read shm for manager

            while(sharedDataptr_MANAGER[2]==0)//waiting for manager to read the shm
            {
                //printf("waiter waitng for manager to readdddd");
                sleep(1);
            }
            if (sharedDataptr_MANAGER[2] == -1000) //manager read
            {
                shmdt(sharedDataptr_MANAGER);
                //total_earning=0;
                if(shmctl(shm_id_num,IPC_RMID,NULL)==-1)
                {
                    printf("error in shmctl");
                }
                printf("Waiter leaves...\n ");
                break;
            } 
            /*****TERMINATION OF SHM WITH MANAGER*/            
        }
    }

    

    return 0;
}
