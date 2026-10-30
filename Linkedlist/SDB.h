#include"STD.h"
#include <stdbool.h>
#ifndef SDB_H_INCLUDED
#define SDB_H_INCLUDED
#define MAX_STUDENT 10

typedef struct SimpleDb
{
    uint32 Student_ID;
    uint32 Student_year;

    uint32 Course1_ID;
    uint32 Course1_grade;

    uint32 Course2_ID;
    uint32 Course2_grade;

    uint32 Course3_ID;
    uint32 Course3_grade;
}student;
typedef struct n
{
    student student;
    struct n*link;
}Node;

uint8 countNodes();
bool SDB_IsFull();
uint8 SDB_GetUsedSize();
void SDB_DeleteEntry(uint32 id);
bool SDB_ReadEntry(uint32 id);
void SDB_GetList(uint8*count,uint32 *list);
bool SDB_IsIdExist(uint32 id);
#endif // SDB_H_INCLUDED
