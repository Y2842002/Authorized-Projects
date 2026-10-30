#include <stdbool.h>//for boolean
#include<stddef.h>//for NULL pointers
#include <stdio.h>
#include <stdlib.h>
#include"SDB.h"
#include"STD.h"
Node*start=NULL;
uint8 countNodes()
{ //function to count number of nodes
    uint8 count = 0;
    Node* ptr= start;
    while (ptr != NULL)
    {
        count++;
        ptr = ptr->link;
    }
    return count;
}
bool SDB_IsFull()
{ //checks if the Database is full or not
    if(countNodes() >= MAX_STUDENT)
    {
        printf("You have reached the maximum Database members\n");
        return true ;
    }
    else
    {
        printf("You can add more members in the DataBase\n");
        return false ;
    }
}
uint8 SDB_GetUsedSize()
{//Print number of elements in the Database
    printf("Number of students in the Database %d\n", countNodes());
}
bool SDB_AddEntry()
{
    if (SDB_IsFull()) // Before adding any member
    {                //check whether the database is full or not
        printf("Database is Full\n");
        return false;
    }
    Node* ptr2 = (Node*)malloc(sizeof(Node));
    //Define a new node
    if (ptr2 == NULL)//Check the available memory
    {
        printf("Failed to allocate memory for new Student\n");
        return false;
    }
    //Below check the input from the user within the allowed range
    //Insert node from the end
    printf("Enter Student ID ** range[1-10]:\n");
    scanf("%d",&ptr2->student.Student_ID);
    if (ptr2->student.Student_ID < 1 || ptr2->student.Student_ID>10)
    {

        printf("Wrong input\nPlease follow the allowed range\n");
        free(ptr2);
        return false;
    }
    printf("Enter Student year ** range [2020-2024]:\n");
    scanf("%d", &ptr2->student.Student_year);
    if (ptr2->student.Student_year > 2024 || ptr2->student.Student_year <2020)
    {
    printf("Wrong input\nPlease follow the allowed range\n");
        free(ptr2);
        return false;
    }
    printf("Enter Course 1 ID ** range[1-10]:\n");
    scanf("%d",&ptr2->student.Course1_ID);
    if (ptr2->student.Course1_ID < 1 || ptr2->student.Course1_ID>10)
    {
          printf("Wrong input\nPlease follow the allowed range\n");
        free(ptr2);
        return false;
    }
    printf("Enter Course 1 grade ** range [0-100] :\n");
    scanf("%d", &ptr2->student.Course1_grade);
    if (ptr2->student.Course1_grade < 0 || ptr2->student.Course1_grade>100)
    {
         printf("Wrong input\nPlease follow the allowed range\n");
        free(ptr2);
        return false;
    }
    printf("Enter Course 2 ID ** range[1-10]:\n");
    scanf("%d", &ptr2->student.Course2_ID);
    if (ptr2->student.Course2_ID < 1 || ptr2->student.Course2_ID>10)
    {
         printf("Wrong input\nPlease follow the allowed range\n");
        free(ptr2);
        return false;
    }
    printf("Enter Course 2 grade ** range [0-100] :\n");
    scanf("%d", &ptr2->student.Course2_grade);
    if (ptr2->student.Course2_grade < 0 || ptr2->student.Course2_grade>100)
    {
       printf("Wrong input\nPlease follow the allowed range\n");
        free(ptr2);
        return false;
    }
    printf("Enter Course 3 ID ** range[1-10]:\n");
    scanf("%d", &ptr2->student.Course3_ID);
    if (ptr2->student.Course3_ID < 1 || ptr2->student.Course3_ID>10)
    {
       printf("Wrong input\nPlease follow the allowed range\n");
        free(ptr2);
        return false;
    }
    printf("Enter Course 3 grade ** range [0-100] :\n");
    scanf("%d", &ptr2->student.Course3_grade);
    if (ptr2->student.Course3_grade < 0 || ptr2->student.Course3_grade>100)
    {
        printf("Wrong input\nPlease follow the allowed range\n");
        free(ptr2);
        return false;
    }
    //connect the last added node to the start
    //making start the head of the node
    ptr2->link = start;
    start = ptr2;
    return true;
}
void SDB_DeleteEntry(uint32 id)
{
   uint8 b=0;
   if(countNodes()<4)
   {//Since our Minimum Database consist of 3 members
   //it wont let you delete unless you have more than 3 elements
    printf("You have reached the minimum Database members\n");
    printf("Please Add more than 3 members \n");
    return;
   }
   //This function delets the last node in the list
   // since the start is heading to the last node added
    if (start != NULL && start->student.Student_ID == id)
    {
        Node* temp = start;
        start = start->link;
       printf("%d iD was deleted successfully\n",id);
        free(temp);
    }
    //this function starts to point from the NULL position
    //till the end of the list
    Node* ptr = start;
    while (ptr != NULL && ptr->link != NULL)
    {
        if (ptr->link->student.Student_ID == id)
        {
            Node* temp = ptr->link;
            ptr->link = temp->link;
            free(temp);
            b=1;
            printf("%d ID was deleted successfully\n",id);
            break;
        }
        ptr = ptr->link;
    }
    //to terminate The delete function since the is
    // doesn't exist
    if (b=0)
    {
        printf("%d ID wasn't found \n", id);
    }
}

bool SDB_ReadEntry(uint32 id)
{
    //This Function display the info
    //of the input ID.
    printf("Please enter ID to view Data :\n");
    scanf("%d",&id);
    Node*p=start;
    while(p!=NULL)
    {
        if((p->student.Student_ID)==id)
        {
        printf("Student.Student_ID:%d\n",p->student.Student_ID);
        printf("Student.Student_year : %d\n",p->student.Student_year);
        printf("Student.Cousre1_ID : %d\n",p->student.Course1_ID);
        printf("Student.Cousre1_grade : %d\n",p->student.Course1_grade);
        printf("Student.Cousre2_ID : %d\n",p->student.Course2_ID);
        printf("Student.Cousre2_grade : %d\n",p->student.Course2_grade);
        printf("Student.Cousre3_ID : %d\n",p->student.Course3_ID);
        printf("Student.Course3_grade : %d\n",p->student.Course3_grade);
        return true;
        }
        p=p->link;
        }
        printf("%d ID is'not correct or wasn't found \n",id);
        return false;
}
void SDB_GetList(uint8*count,uint32 *list)
{
    //**This Function displays the number**
    //1)ids exist in the list.
    //2)ID of each Student.
    uint8 i=0;
    *count=0;
    Node* p = start;
    while(p != NULL && *count < MAX_STUDENT)
    {
        list[i] = p->student.Student_ID;
        p = p->link;
        i++;
    }
    *count=i;
    printf("Number of Ids in the list are: %d\n", *count);
    printf("List of IDS within the list are:\n");
    for(uint8 i = 0; i < *count; i++)
    {
        printf("%d\n", list[i]);
    }
}
bool SDB_IsIdExist(uint32 id)
{//Checks if an ID exists in the Database
    Node*p=start;
    while(p!=NULL)
    {
        if(p->student.Student_ID==id)
        {
            printf("%d ID exists in the List \n",p->student.Student_ID);
            return true;
        }
        p=p->link;
    }
    printf("%d ID wasnt found \n",id);
    return false;
}
