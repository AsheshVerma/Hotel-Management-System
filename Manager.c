#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include<fcntl.h>
#include<sys/file.h>
#include<errno.h>

#define WAITER_MANAGER_SHM_ID_base 200

int main()
{
    int total_instance_table_to_be_executed=0;
    int shm_id_num=0;
    int total_hotel_earning=0,total_wages_of_waiter=0,net_profit=0;
    printf("Enter the Total Number Tables at the Hotel\n");
    scanf("%d",&total_instance_table_to_be_executed);//this many rows will be there in earnings.txt-----1 for each table

    int earnings_from_each_Table_arr[total_instance_table_to_be_executed];
    
    for(int i=0;i<total_instance_table_to_be_executed;i++)
    {
        earnings_from_each_Table_arr[i]=-1;
    }
    //printf("intialized earning arr\n");
    sleep(10);

    while(1)
    {        
        //printf("entered while loop\n");
        
        for(int i=0;i<total_instance_table_to_be_executed;i++)
        {
            //printf("entered for loop for waiter %d\n",i+1);
            shm_id_num = shmget(WAITER_MANAGER_SHM_ID_base + (i+1) , sizeof(int)*4, 0666 | IPC_CREAT);
            if (shm_id_num < 0) 
            {
                printf("shmget (connect channel)");
                exit(EXIT_FAILURE);
            }

            int *sharedDataptr_MANAGER = (int *)shmat(shm_id_num, NULL, 0);

            if (sharedDataptr_MANAGER == (int *) -1) 
            {
                printf("shmat");
                exit(EXIT_FAILURE);
            }
            if(sharedDataptr_MANAGER[3]==0)//checking if ready to read shm or not
            {
                while(sharedDataptr_MANAGER[3]==0)//it will wait sequentially
                {
                    sleep(2);
                }
                
                earnings_from_each_Table_arr[i]=0;
                earnings_from_each_Table_arr[i]+=sharedDataptr_MANAGER[1];
                
                //printf("read val by mgmt from shm");
            }            
            else
            {                
                earnings_from_each_Table_arr[i]=0;
                earnings_from_each_Table_arr[i]+=sharedDataptr_MANAGER[1];
                sharedDataptr_MANAGER[2]=-1000;
                //printf("read val by mgmt from shm");
            }
        }

        for(int i=0;i<total_instance_table_to_be_executed;i++)//so that all waiter terminate at the same time although we r reading shm sequentially 1..2 ..3.....
        {
           shm_id_num = shmget(WAITER_MANAGER_SHM_ID_base + (i+1) , sizeof(int)*4, 0666 | IPC_CREAT);
            if (shm_id_num < 0) 
            {
                printf("shmget (connect channel)");
                exit(EXIT_FAILURE);
            }

            int *sharedDataptr_MANAGER = (int *)shmat(shm_id_num, NULL, 0);

            if (sharedDataptr_MANAGER == (int *) -1) 
            {
                printf("shmat");
                exit(EXIT_FAILURE);
            }  
            sharedDataptr_MANAGER[2]=-1000;          
        }        
        break;
    }
    //printf("exited while lopp ,now writng into file start\n");


    for(int i=0;i<total_instance_table_to_be_executed;i++)
    {
        total_hotel_earning+=earnings_from_each_Table_arr[i];
    }
    printf("Total Earnings of Hotel: %d INR\n",total_hotel_earning);
    total_wages_of_waiter=0.4*total_hotel_earning;//40% of total hotel earning
    printf("Total Wages of Waiters: %d INR\n",total_wages_of_waiter);
    net_profit=total_hotel_earning-total_wages_of_waiter;
    printf("Total Profit: %d INR\n",net_profit);

    /*****writing into file start*/
    for(int i=0;i<total_instance_table_to_be_executed+1;i++)
    {       

        int fd=open("earnings.txt",O_WRONLY|O_CREAT|O_APPEND,0644);
        if(fd==-1)
        {
            printf("error opening file");
            exit(EXIT_FAILURE);
        }

        if(flock(fd,LOCK_EX)==-1)
        {
            printf("error locking file");
            close(fd);
            exit(EXIT_FAILURE);
        }

        FILE *Earnings_file=fopen("earnings.txt","a");
        if(Earnings_file==NULL)
        {
            printf("error opening file");
            close(fd);
            exit(EXIT_FAILURE);
        }
        /*PRINTING LAST STAMENTS IN FILE*/
        if(i==total_instance_table_to_be_executed)
        {
            fprintf(Earnings_file,"Total Earnings of Hotel: %d\n",total_hotel_earning);
            fprintf(Earnings_file,"Total Wages of Waiters: %d\n",total_wages_of_waiter);            
            fprintf(Earnings_file,"Total Profit: %d\n",net_profit);
            fflush(Earnings_file);
            fclose(Earnings_file);

            if(flock(fd,LOCK_UN)==-1)
            {
                printf("error unlocking file");
                close(fd);
                exit(EXIT_FAILURE);
            }
            close(fd);
            break;
        }
        /*PRINTING LAST STAMENTS IN FILE*/

        fprintf(Earnings_file,"Earning from Table %d : %d\n",i+1,earnings_from_each_Table_arr[i]);

        fflush(Earnings_file);
        fclose(Earnings_file);

        if(flock(fd,LOCK_UN)==-1)
        {
            printf("error unlocking file");
            close(fd);
            exit(EXIT_FAILURE);
        }
        close(fd);
        /*****writing into file end*/       
    }
      
    printf("Thank You for visiting the Hotel\n");

    return 0;         
}