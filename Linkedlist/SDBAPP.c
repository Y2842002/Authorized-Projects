#include"SDB.h"
#include"STD.h"
void stars()
{
    for(uint8 i=0;i<25;i++)
    {
        printf("*");
    }
}
void cool_name()
{
    stars();
    printf("\nYousef Osama\n");
    stars();
    printf("\nAmit D59\n");
    stars();
    printf("\nWelcome to the Database made by Singlelinkedlist\n");
    stars();
}
void options()
{
    printf("\nTo Enter a student Data in the Database Press 1\n");
    printf("To Get how many students in the database Press 2\n");
    printf("To Read Student data Press 3\n");
    printf("To Get List of all Students Ids Press 4\n");
    printf("To Check if id is existed Press 5\n");
    printf("To Delete Student Data Press 6\n");
    printf("To Check Data Base is Full Press 7\n");
    printf("To Exit press 0\n");
    printf("Please Enter your Choice :\n");
}
void SDB_action(int choice)
{
    switch(choice)
     {
        case 0:
            break;
        case 1:
            SDB_AddEntry();
            break;
        case 2:
             SDB_GetUsedSize();
            break;
        case 3:
            {
            uint32 id;
            SDB_ReadEntry(id);}
            break;
        case 4:
            {uint8 count;
            uint32 list[MAX_STUDENT];
           SDB_GetList(&count,list);
            }
            break;
        case 5:
        { uint32 id ;
            printf("Enter ID to check if its existed \n");
            scanf("%d",&id);
            SDB_IsIdExist(id);
                                  }
            break;
        case 6:
            {uint32 id;
            printf("Enter ID to be Deleted\n");
            scanf("%d",&id);
            SDB_DeleteEntry(id);}
            break;
        case 7:
             SDB_IsFull();
            break;
        case 8:
            options();
            break;
        default:
            printf("Invalid choice.\n");
    }
}

void SDB_APP()
{
    cool_name();
    uint8 choice;
    options();
    scanf("%d",&choice);
   while(choice!=0)
   {
    SDB_action(choice);
    printf("Would you like to continue or quit the Database?");
    printf("\tTo Show options again press 8\n");
    scanf("%d",&choice);
    }
}

