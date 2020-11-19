#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "part3.h"
#include "main.h"

int map_direction_to_index(enum DIRECTION d)
{
	switch (d)
	{
	case EAST:
		return 0;
	case SOUTH:
		return 1;
	case WEST:
		return 2;
	case NORTH:
		return 3;
	default:
		return -1;
	}
}
int map_lane_to_index(enum LANE l)
{
	switch (l)
	{
	case LEFT:
		return 0;
	case RIGHT:
		return 1;
	default:
		return -1;
	}
}

char *getStringDEnum(enum DIRECTION d)
{
	switch (d)
	{
	case EAST:
		return "EAST";
		break;
	case SOUTH:
		return "SOUTH";
		break;
	case NORTH:
		return "NORTH";
		break;
	case WEST:
		return "WEST";
		break;

	default:
		break;
	}
}

char *getStringLEnum(enum LANE l)
{
	switch (l)
	{
	case RIGHT:
		return "RIGHT";
		break;
	case LEFT:
		return "LEFT";
		break;
	default:
		break;
	}
}

int isGoingStraight(int from, int to)
{
	int check = abs(from - to);

	if (check == 2)
		return 1;
	return 0;
}

int isGoingRight(int from, int to)
{
	int check = from - to;

	if (check == 1 || check == -3)
		return 1;
	return 0;
}

int isGoingLeft(int from, int to)
{
	int check = from - to;

	if (check == -1 || check == 3)
		return 1;
	return 0;
}

/*
	- DO NOT USE SLEEP FUNCTION
	- Define every helper function in .h file
	- Use Semaphores for synchronization purposes
 */

/**
* Declare semaphores here so that they are available to all functions.
*/
sem_t **signals;
sem_t *wait_signal;
sem_t *mutex;
sem_t *init;

sem_t *vehical_infront;
sem_t *printer;
sem_t *all_left;
int total_waiting;
int **cars_waiting_per_lane;
int *waiting_to_go_left;
/**
 * Do any initial setup work in this function. You might want to 
 * initialize your semaphores here.
 */
void initializeP3()
{

	cars_waiting_per_lane = (int **)malloc(sizeof(int *) * 4);

	signals = (sem_t **)malloc(sizeof(sem_t *) * 4);
	wait_signal = (sem_t *)malloc(sizeof(sem_t));
	mutex = (sem_t *)malloc(sizeof(sem_t));
	init = (sem_t *)malloc(sizeof(sem_t));

	vehical_infront = (sem_t *)malloc(sizeof(sem_t) * 4);
	printer = (sem_t *)malloc(sizeof(sem_t));
	all_left = (sem_t *)malloc(sizeof(sem_t) * 4);
	waiting_to_go_left = malloc(sizeof(int));
	total_waiting = 0;

	sem_init(wait_signal, 0, 0);
	sem_init(mutex, 0, 1);
	sem_init(printer, 0, 1);
	sem_init(init, 0, 1);

	for (int i = 0; i < 4; i++)
	{
		//sem_init(&signals[i], 0, 0);
		waiting_to_go_left[i] = 0;
		signals[i] = (sem_t *)malloc(sizeof(sem_t) * 2);
		cars_waiting_per_lane[i] = (int *)malloc(sizeof(int) * 2);
		sem_init(&vehical_infront[i], 0, 1);
		sem_init(&all_left[i], 0, 0);
		for (int j = 0; j < 2; j++)
		{
			sem_init(&signals[i][j], 0, 0);
			cars_waiting_per_lane[i][j] = 0;
		}
	}
}

/**
 * This is the function called for each car thread. You can check
 * how these functions are used by going over the test3 function
 * in main.c
 * If there is a car going from SOUTH to NORTH, from lane LEFT,
 * print 
 * SOUTH NORTH LEFT
 * Also, if two cars can simulateneously travel in the two lanes,
 * first print all the cars in the LEFT lane, followed by all the
 * cars in the right lane
 *
 * Input: *argu is of type struct argumentP3 defined in main.h
 */
void *goingFromToP3(void *argu)
{
	// Some code to help in understanding argu

	//sem_wait(mutex);
	struct argumentP3 *car = (struct argumentP3 *)argu;
	enum DIRECTION d_from = car->from;
	enum DIRECTION d_to = car->to;
	enum LANE l_lane = car->lane;
	int id = car->user_id;

	char *s_to = getStringDEnum(d_to);
	char *s_from = getStringDEnum(d_from);
	char *s_lane = getStringLEnum(l_lane);

	int to = map_direction_to_index(d_to);
	int from = map_direction_to_index(d_from);
	int lane = map_lane_to_index(l_lane);

	// printf("%s %s %s\n", s_from, s_to, s_lane);
	// fflush(stdout);

	int Straight = isGoingStraight(from, to);
	int Left = isGoingLeft(from, to);
	int Right = isGoingRight(from, to);
	//sem_post(mutex);

	if (Straight == 1)
	{
		sem_wait(&vehical_infront[from]);
		// sem_wait(printer);
		// printf("LOCK ACQUIRED S %s\n", s_from);
		// fflush(stdout);
		// sem_post(printer);
		sem_wait(mutex);
		total_waiting++;
		cars_waiting_per_lane[from][lane]++;
		sem_post(mutex);

		// printf("no vehice infront\n");
		// fflush(stdout);
		sem_wait(&signals[from][lane]);

		sem_wait(mutex);
		total_waiting--;
		cars_waiting_per_lane[from][lane]--;
		sem_wait(printer);
		printf("%s %s %s\n", s_from, s_to, s_lane);

		fflush(stdout);

		sem_post(printer);
		sem_post(&vehical_infront[from]);

		sem_post(mutex);

		sem_post(wait_signal);
		sem_post(&all_left[from]);
	}
	if (Left == 1)
	{
		sem_wait(mutex);
		total_waiting++;
		waiting_to_go_left[from]++;
		sem_post(mutex);

		sem_wait(&vehical_infront[from]);
		// sem_wait(printer);
		// printf("LOCK ACQUIRED L\n");
		// fflush(stdout);
		// sem_post(printer);

		sem_post(&vehical_infront[from]);

		sem_wait(mutex);
		total_waiting--;
		waiting_to_go_left[from]--;
		sem_wait(printer);
		printf("%s %s %s\n", s_from, s_to, s_lane);
		fflush(stdout);
		sem_post(printer);
		sem_post(mutex);
		sem_post(&all_left[from]);
	}
	if (Right == 1)
	{
		sem_wait(mutex);
		total_waiting++;
		cars_waiting_per_lane[from][lane]++;
		sem_post(mutex);
		// printf("waiting at signal %d lane %d\n", from, lane);
		// fflush(stdout);
		sem_wait(&signals[from][lane]);

		sem_wait(mutex);
		total_waiting--;
		cars_waiting_per_lane[from][lane]--;

		sem_post(mutex);
		sem_wait(printer);
		printf("%s %s %s\n", s_from, s_to, s_lane);
		fflush(stdout);
		sem_post(printer);
		sem_post(wait_signal);
	}
}

/**
 * startP3 is called once all cars have been initialized. The logic of the traffic signals
 * will go here
 */

void *traffic_signal()
{
	int count = 0;

	while (1)
	{

		for (int signal_i = 0; signal_i < 4; signal_i++)
		{
			count = 0;
			sem_wait(mutex);
			int waiting_left = cars_waiting_per_lane[signal_i][0];
			int waiting_right = cars_waiting_per_lane[signal_i][1];
			int to_go_left = waiting_to_go_left[signal_i];
			sem_post(mutex);
			for (int i = 0; i < waiting_left + to_go_left; i++)
			{
				if (count < 5 && i < waiting_left)
				{
					sem_wait(printer);
					//	printf("signalled %d LEFT\n", signal_i);
					fflush(stdout);
					sem_post(printer);
					sem_post(&signals[signal_i][0]);

					sem_wait(wait_signal);

					count++;
				}

				sem_wait(&all_left[signal_i]);
			}
			for (int i = 0; i < waiting_right; i++)
			{
				if (count < 5)
				{
					sem_wait(printer);
					//	printf("signalled %d RIGHT\n", signal_i);
					fflush(stdout);
					sem_post(printer);
					sem_post(&signals[signal_i][1]);

					sem_wait(wait_signal);
					count++;
				}
				else
				{
					break;
				}
			}
		}

		if (total_waiting == 0)
			break;
	}
}

void startP3()
{
	sleep(1);
	pthread_t signal;

	pthread_create(&signal, NULL, traffic_signal, NULL);
}