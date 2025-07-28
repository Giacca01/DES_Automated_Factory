#ifndef __NESssq__
#define __NESssq__

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <list>

#define EXTENDED false
#define DEBUG false

#define NAGV 10

// system's events
#define NO_EVENT -1

#define ARRIVAL_LOAD 0
#define DEPARTURE_LOAD 1

#define ARRIVAL_M1 2
#define DEPARTURE_M1 3 

#define ARRIVAL_M2 4
#define DEPARTURE_M2 5

#define ARRIVAL_M3 6
#define DEPARTURE_M3 7

#define ARRIVAL_UNLOAD 8
#define DEPARTURE_UNLOAD 9

#define ARRIVAL_RECHARGE 10
#define DEPARTURE_RECHARGE 11 

#define PRINT_STATE 12 

#define START      0.0
#define ENDTIME 10

// model parameters
#define ZL 60
#define ALPHA 0.8
#define BETA 0.2
#define MI_1 50
#define MI_2 250
#define DELTA 10000
#define S1 9
#define S2 120
#define S3 80
#define ZU 50
#define SR 100


// Routing Probabilities
#define Q_11 0.9
#define Q_12 0.1
#define Q_24 0.8
#define Q_23 0.2
#define Q_40 0.6
#define Q_45 0.4

#define INITIAL_SEED 123456789
#define PERCENTILE 1.960
#define PRECISION 0.0500


typedef struct node* nodePtr;

typedef struct DLL{
	nodePtr Head;
	nodePtr Tail;
}dll;

typedef struct {
	int type;
	char name[256];
	// event creation time
	double create_time;
	// event occurrence time
	double occur_time;
	// time of the arrival to the current station
	double arrival_time;
	double service_time;
	double rem_service_time;
	double start_man;
	double arrival_load;
	int num_visits_M1;
	bool isTagged;
} event_notice;


struct node{
	event_notice event;
	nodePtr left;
	nodePtr right;
};

typedef struct {
	double man_time_sum;
	int passages_counter;
} regeneration_pair;

typedef struct {
	int num_cycles;
	double S_a;
	double S_v;
	double S_aa;
	double S_av;
	double S_vv;
	double width;
	double mi;
	double stdev;
	double precision;
	std::list<regeneration_pair> observations;
} regeneration_sample;

// sum of delay, wait and service times
typedef struct {
    double delay;
    double wait;
    double service;
} performance_sum;

typedef struct {
	// n counts the number of jobs in the system
	double Area_n;
	// q counts the number of jobs in the queue
	double Area_q;
	// y counts the number of jobs in service
	double Area_y;
	int nsys;
	int narr;
	int ncomp;
	performance_sum station_sum;
} SSQ_station;



void simulate(void);
void initialize(void);
void engine(void);

void print_state();
void print_station_state(SSQ_station);

void arrival_load(nodePtr);
void departure_load(nodePtr);

void arrival_m1(nodePtr);
void departure_m1(nodePtr);

void arrival_m2(nodePtr);
void departure_m2(nodePtr);

void arrival_m3(nodePtr);
void departure_m3(nodePtr);

void arrival_unload(nodePtr);
void departure_unload(nodePtr);

void arrival_recharge(nodePtr);
void departure_recharge(nodePtr);

double GetRouteProbM1();
double GetRouteProbM2();
double GetRouteProbRec();

double GetServiceLoad();
double GetServiceM1();
double GetServiceM2();
double GetServiceM3();
double GetServiceUnload();
double GetServiceRecharge();


void report(void);
void print_results(void);
void print_station_results(SSQ_station);


nodePtr get_new_node();
void  return_node(nodePtr item);
nodePtr pop_from_AL(void);
void push_on_AL(nodePtr item);
void schedule(nodePtr new_node_event);
nodePtr event_pop(void);
void print_FEL();
void print_list(dll * doublell);


bool RegPoint(event_notice);
void CollectRegStatistics();
void ResetMeasures();
void ComputeConfidenceInterval();
bool DecideToStop();

#endif
