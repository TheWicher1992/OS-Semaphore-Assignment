#ifndef PART_1
#define PART_1

struct lift
{
    int max_capacity;
    int people_inside;
    int total_floors;
    int current_floor;
    int *waiting_out_currentfloor;
    int *waiting_in_currentfloor;
    int total_waiting;
};

void initializeP1(int numFloors, int maxNumPeople);
void *goingFromToP1(void *);
void startP1();

#endif
