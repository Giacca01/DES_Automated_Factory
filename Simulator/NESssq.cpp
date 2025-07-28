
#include "NESssq.h"
#include "NESssq_List_Manager.h"
#include "random_var_gen.h"
#include "rngs.h"

using namespace std;

/*****************************/
/* System's state definition */
/****************************/

double Stop;
double ObservPeriod;
double clock;
bool end_of_sim;
double remaining_lifetime;
int nPartsStarted;
int nPartsCompleted;

// data structures that stores station-level data
SSQ_station LOAD;
SSQ_station M1;
SSQ_station M2;
SSQ_station M3;
SSQ_station UNLOAD;
SSQ_station REC;
performance_sum system_sum;

// input queues
dll FEL;
dll Input_Queue_M1;
dll Input_Queue_M2;
dll Input_Queue_M3;
dll Input_Queue_REC;
dll AL;

int event_counter; 

/**** Regenerative method variables *****/
regeneration_sample reg_samp;
double manufacturing_time_sum;
double total_manufacturing_time;
double total_response_time;
double total_cycle_time;
int passages_counter;
int num_obs = 41;

int print_statistics;


int main(int argc, char* argv[]){
	int i;
	
	if (argc == 2)
		print_statistics = argv[1][0] - '0';
	else
		print_statistics = false;

    printf("Prova %d\n", argv[1][0] - '0');
    
    simulate();
	getchar();
      
	return 0;  
}

void simulate() {

	initialize();

	while (!DecideToStop()){
		while (num_obs > 0)
			engine(); 
	}

	report();
}

void initialize(){
	nodePtr curr_notice;

	event_counter = 0;
	clock            = 0.0; 
	remaining_lifetime = DELTA;
	nPartsStarted = 0;
	nPartsCompleted = 0;
	manufacturing_time_sum = 0.0;
	total_manufacturing_time = 0.0;
	total_response_time = 0.0;
	total_cycle_time = 0.0;

	/* Stations statistics */
	LOAD = {0, 0, 0, 0, 0, 0, {0, 0, 0}};
	M1 = {0, 0, 0, 0, 0, 0, {0, 0, 0}};
	M2 = {0, 0, 0, 0, 0, 0, {0, 0, 0}};
	M3 = {0, 0, 0, 0, 0, 0, {0, 0, 0}};
	UNLOAD = {0, 0, 0, 0, 0, 0, {0, 0, 0}};
	REC = {0, 0, 0, 0, 0, 0, {0, 0, 0}};

	/* System statistics */
	system_sum = {0, 0, 0};

	/* Regeneration method statistics */
	std::list<regeneration_pair> tmp;
	reg_samp = {0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, tmp};

	/* Future Event List */
	FEL.Head = NULL;
	FEL.Tail = NULL;
	/* Input Queue of Server */
	Input_Queue_M1.Head = NULL;
	Input_Queue_M1.Tail = NULL;
	Input_Queue_M2.Head = NULL;
	Input_Queue_M2.Tail = NULL;
	Input_Queue_M3.Head = NULL;
	Input_Queue_M3.Tail = NULL;
	Input_Queue_REC.Tail = NULL;
	Input_Queue_REC.Tail = NULL;
	/* Available List */
	AL.Head = NULL;
	AL.Tail = NULL;	


	// Initializing random streams seeds
	PlantSeeds(INITIAL_SEED);	
	
	/* Scheduling departure of all the AGVs from the loading station*/
	for (int i = 0; i < NAGV; i++){
		curr_notice = get_new_node();
		curr_notice->event.type = ARRIVAL_LOAD;
		curr_notice->event.create_time = clock;
		curr_notice->event.occur_time = clock;
		curr_notice->event.arrival_time = clock;
		curr_notice->event.service_time = 0.0;
		curr_notice->event.rem_service_time = 0.0;
		curr_notice->event.start_man = 0.0;
		curr_notice->event.arrival_load = 0.0;
		curr_notice->event.num_visits_M1 = 0;
		curr_notice->event.isTagged = (i == 0);
		curr_notice->right = NULL;
		curr_notice->left = NULL;
		sprintf(curr_notice->event.name, "J%d", i + 1);
		schedule(curr_notice);
	}
}

void engine(void){
	int event_type;
	double  oldclock;
	double  interval;
	nodePtr new_event;

	new_event = event_pop();

	// checks whether the current event is a regeneration point
	RegPoint(new_event->event);
	oldclock = clock;
	clock = new_event->event.occur_time;
	interval = clock - oldclock;

	if (LOAD.nsys > 0) {
		LOAD.Area_n = LOAD.Area_n + LOAD.nsys * interval;
		LOAD.Area_q = LOAD.Area_q + LOAD.nsys * interval;
	}

	if (M1.nsys > 0) {
		M1.Area_n = M1.Area_n + M1.nsys * interval;
		M1.Area_q = M1.Area_q + (M1.nsys - 1) * interval;
		M1.Area_y = M1.Area_y + interval;		
	}

	if (M2.nsys > 0) {
		M2.Area_n = M2.Area_n + M2.nsys * interval;
		M2.Area_q = M2.Area_q + (M2.nsys - 1) * interval;
		M2.Area_y = M2.Area_y + interval;		
	}

	if (M3.nsys > 0) {
		M3.Area_n = M3.Area_n + M3.nsys * interval;
		M3.Area_q = M3.Area_q + (M3.nsys - 1) * interval;
		M3.Area_y = M3.Area_y + interval;		
	}

	if (UNLOAD.nsys > 0) {
		UNLOAD.Area_n = UNLOAD.Area_n + UNLOAD.nsys * interval;
		UNLOAD.Area_q = UNLOAD.Area_q + UNLOAD.nsys * interval;
	}

	if (REC.nsys > 0) {
		REC.Area_n = REC.Area_n + REC.nsys * interval;
		REC.Area_q = REC.Area_q + (REC.nsys - 1) * interval;
		REC.Area_y = REC.Area_y + interval;
	}
		
	event_type = new_event->event.type;	
	
	switch(event_type){
		case ARRIVAL_LOAD: 
			arrival_load(new_event);
			break;
		case DEPARTURE_LOAD: 
			departure_load(new_event);
			break;
		case ARRIVAL_M1: 
			arrival_m1(new_event);
			break;
		case DEPARTURE_M1:
			departure_m1(new_event);
			break;
		case ARRIVAL_M2: 
			arrival_m2(new_event);
			break;
		case DEPARTURE_M2:
			departure_m2(new_event);
			break;
		case ARRIVAL_M3: 
			arrival_m3(new_event);
			break;
		case DEPARTURE_M3:
			departure_m3(new_event);
			break;
		case ARRIVAL_UNLOAD: 
			arrival_unload(new_event);
			break;
		case DEPARTURE_UNLOAD:
			departure_unload(new_event);
			break;
		case ARRIVAL_RECHARGE: 
			arrival_recharge(new_event);
			break;
		case DEPARTURE_RECHARGE:
			departure_recharge(new_event);
			break;
		case PRINT_STATE:
			print_state();
			break;
	}
		

	event_counter++;

	if (event_counter == 30){
		nodePtr curr_notice = get_new_node();
		curr_notice->event.type = PRINT_STATE;
		curr_notice->event.create_time = clock;
		curr_notice->event.occur_time = clock;
		curr_notice->event.arrival_time = clock;
		curr_notice->event.service_time = 0.0;
		curr_notice->event.rem_service_time = 0.0;
		curr_notice->event.start_man = 0.0;
		curr_notice->event.arrival_load = 0.0;
		curr_notice->event.num_visits_M1 = 0;
		curr_notice->right = NULL;
		curr_notice->left = NULL;
		sprintf(curr_notice->event.name, "PRINT_STATE");
		schedule(curr_notice);
	}
}

void print_state(){
	printf("\n-------------------------------------------------------------------------");
	printf("\nSYSTEM STATE AFTER 30 EVENTS");
    printf("\n-------------------------------------------------------------------------");
	printf("\nSystem Clock: %8.6f", clock);
	printf("\nEvent counter: %d", event_counter);
	printf("\nM1 tool remaining lifetime: %8.6f", remaining_lifetime);
	printf("\nNumber of parts started: %d", nPartsStarted);
	printf("\nNumber of parts completed: %d", nPartsCompleted);
	printf("\nTotal system delay: %8.6f", system_sum.delay);
	printf("\nTotal waiting delay: %8.6f", system_sum.wait);
	printf("\nTotal service delay: %8.6f", system_sum.service);

	printf("\n\n****Loading Station State****");
	print_station_state(LOAD);

	printf("\n\n****M1 Station State****");
	print_station_state(M1);
	printf("\n****Input Queue M1:****");
	print_list(&Input_Queue_M1);

	printf("\n\n****M2 Station State****");
	print_station_state(M2);
	printf("\n****Input Queue M2:****");
	print_list(&Input_Queue_M2);

	printf("\n\n****M3 Station State****");
	print_station_state(M3);
	printf("\n****Input Queue M3:****");
	print_list(&Input_Queue_M3);

	printf("\n\n****Unloading Station State****");
	print_station_state(UNLOAD);

	printf("\n\n****Recharge Station State****");
	print_station_state(REC);
	printf("\n****Input Queue Recharge:****");
	print_list(&Input_Queue_REC);

	printf("\n****Future Event List:****");
	print_FEL();
}

void print_station_state(SSQ_station s){
	printf("\nNumber of customers currently at station: %d", s.nsys);
	printf("\nNumber of customers arrived at station: %d", s.narr);
	printf("\nNumber of customers completed by station: %d", s.ncomp);
	printf("\nArea under n function: %8.6f", s.Area_n);
	printf("\nArea under q function: %8.6f", s.Area_q);
	printf("\nArea under q function: %8.6f", s.Area_y);
	printf("\nTotal delay introduced: %8.6f", s.station_sum.delay);
	printf("\nTotal waiting time: %8.6f", s.station_sum.wait);
	printf("\nTotal service time: %8.6f", s.station_sum.service);
}


void arrival_load(nodePtr node_event){
	total_cycle_time += (clock - node_event->event.arrival_load);
	total_response_time += (clock - node_event->event.start_man);

	// AGV statistics reset
	node_event->event.rem_service_time = 0.0;
	node_event->event.start_man = 0.0;
	node_event->event.num_visits_M1 = 0;
	node_event->event.service_time = GetServiceLoad();

	LOAD.nsys++;
	LOAD.narr++;

	node_event->event.create_time = clock;
	node_event->event.type = DEPARTURE_LOAD;
	node_event->event.occur_time = clock + node_event->event.service_time;
	node_event->event.arrival_load = clock;
	schedule(node_event);
}

void departure_load(nodePtr node_event){
	nodePtr next_job;
	double  interval;

	nPartsStarted++;
	system_sum.service += node_event->event.service_time;
	LOAD.station_sum.delay += node_event->event.service_time;
	LOAD.station_sum.wait += node_event->event.service_time;
	LOAD.ncomp++;
	LOAD.nsys--;

	node_event->event.type = ARRIVAL_M1;
	node_event->event.create_time = clock;
	node_event->event.arrival_time = clock;
	node_event->event.service_time = 0.0;
	node_event->event.rem_service_time = 0.0;
	node_event->event.occur_time = clock;
	node_event->event.start_man = node_event->event.arrival_time;
	schedule(node_event);
}

void arrival_m1(nodePtr node_event){
	// setting create time for departure/no event
	node_event->event.create_time = clock;
	if (node_event->event.rem_service_time > 0.0)
		node_event->event.num_visits_M1 = node_event->event.num_visits_M1 + 1;
	else
		node_event->event.num_visits_M1 = 1;

	M1.nsys++;

	if (node_event->event.num_visits_M1 == 1) {
		node_event->event.service_time = GetServiceM1();
		node_event->event.rem_service_time = node_event->event.service_time;
		M1.narr++;
	}

	// the current job is the only one in the system
	if (M1.nsys == 1) {
		node_event->event.type = DEPARTURE_M1;
		// the grinding operation is completed before the tool wears out
		if (node_event->event.rem_service_time <= remaining_lifetime){
			node_event->event.occur_time = clock + node_event->event.rem_service_time;
			remaining_lifetime -= node_event->event.rem_service_time;
			node_event->event.rem_service_time = 0.0;
		} else {
			// the part leaves the station without completing its manufacturing process
			node_event->event.occur_time = clock + remaining_lifetime;
			node_event->event.rem_service_time -= remaining_lifetime;
			remaining_lifetime = 0.0;
		}

		schedule(node_event);
	}
	else {
		node_event->event.type = NO_EVENT;
		node_event->event.occur_time = 0.0;
		enqueue(node_event, &Input_Queue_M1);
	}
}

void departure_m1(nodePtr node_event){
	nodePtr next_job;
	double delay = 0.0;
	double wait = 0.0;
	double tot_prob = 0.0;

	M1.nsys--;
	wait = clock - node_event->event.arrival_time;
	system_sum.wait += wait;
	M1.station_sum.wait += wait;

	// the tool of machine M1 is replaced after each operation
	remaining_lifetime = DELTA;

	if (M1.nsys > 0) {
		
		next_job = dequeue(&Input_Queue_M1);
		next_job->event.type = DEPARTURE_M1;
		// the M1 machine is ready to process another part
		// but its processing may not end before the tool wears out
		if (next_job->event.rem_service_time <= remaining_lifetime) {
			next_job->event.occur_time = clock + next_job->event.rem_service_time;
			remaining_lifetime -= next_job->event.rem_service_time;
			next_job->event.rem_service_time = 0.0;
		} else {
			next_job->event.occur_time = clock + remaining_lifetime;
			next_job->event.rem_service_time -= remaining_lifetime;
			remaining_lifetime = 0.0;
		}

		delay = clock - next_job->event.arrival_time;
		system_sum.delay += delay;
		M1.station_sum.delay += delay;

		schedule(next_job);
	}
	
	// if the whole processing of the part has been completed
	// its arrival to station M2 is completed
	if (node_event->event.rem_service_time == 0.0){
		M1.ncomp++;

		system_sum.service += node_event->event.service_time;
		M1.station_sum.service += node_event->event.service_time;

		tot_prob = Q_11 + Q_12;
		if (GetRouteProbM1() <= Q_11){
			node_event->event.type = ARRIVAL_M1;
			node_event->event.create_time = clock;
			node_event->event.occur_time = clock;
			node_event->event.arrival_time = clock;
		} else {
			node_event->event.type = ARRIVAL_M2;
			node_event->event.create_time = clock;
			node_event->event.arrival_time = clock;
			node_event->event.service_time = 0.0;
			node_event->event.occur_time = clock;
		}
	} else {
		node_event->event.type = ARRIVAL_M1;
		node_event->event.create_time = clock;
		node_event->event.occur_time = clock;
		node_event->event.arrival_time = clock;
	}
	schedule(node_event);
}

void arrival_m2(nodePtr node_event){
	M2.nsys++;
	M2.narr++;
	node_event->event.create_time = clock;
	node_event->event.service_time = GetServiceM2();

	if (M2.nsys == 1) {
		node_event->event.type = DEPARTURE_M2;
		node_event->event.occur_time = clock + node_event->event.service_time;
		schedule(node_event);
	}
	else {
		node_event->event.type = NO_EVENT;
		node_event->event.occur_time = 0.0;
		enqueue(node_event, &Input_Queue_M2);
	}
}

void departure_m2(nodePtr node_event){
	nodePtr next_job;
	double delay = 0.0;
	double wait = 0.0;
	double tot_prob = 0.0;

	M2.nsys--;
	M2.ncomp++;
	system_sum.service += node_event->event.service_time;
	M2.station_sum.service += node_event->event.service_time;
	wait = clock - node_event->event.arrival_time;
	system_sum.wait += wait;
	M2.station_sum.wait += wait;

	if (M2.nsys > 0) {
		next_job = dequeue(&Input_Queue_M2);
		next_job->event.type = DEPARTURE_M2;
		next_job->event.occur_time = clock + next_job->event.service_time;
		delay = clock - next_job->event.arrival_time;
		system_sum.delay += delay;
		M2.station_sum.delay += delay;
		schedule(next_job);
	}
	
	tot_prob = Q_24 + Q_23;
	if (GetRouteProbM2() <= Q_24)
		node_event->event.type = ARRIVAL_UNLOAD;
	else
		node_event->event.type = ARRIVAL_M3;

	node_event->event.create_time = clock;
	node_event->event.arrival_time = clock;
	node_event->event.service_time = 0.0;
	node_event->event.occur_time = clock;

	schedule(node_event);
}

void arrival_m3(nodePtr node_event){
	M3.nsys++;
	M3.narr++;
	node_event->event.create_time = clock;
	node_event->event.service_time = GetServiceM3();

	if (M3.nsys == 1) {
		node_event->event.type = DEPARTURE_M3;
		node_event->event.occur_time = clock + node_event->event.service_time;
		schedule(node_event);
	}
	else {
		node_event->event.type = NO_EVENT;
		node_event->event.occur_time = 0.0;
		enqueue(node_event, &Input_Queue_M3);
	}
}

void departure_m3(nodePtr node_event){
	nodePtr next_job;
	double delay = 0.0;
	double wait = 0.0;

	/* Update statistics */
	M3.nsys--;
	M3.ncomp++;
	system_sum.service += node_event->event.service_time;
	M3.station_sum.service += node_event->event.service_time;
	wait = clock - node_event->event.arrival_time;
	system_sum.wait += wait;
	M3.station_sum.wait += wait;

	if (M3.nsys > 0) {
		next_job = dequeue(&Input_Queue_M3);
		next_job->event.type = DEPARTURE_M3;
		next_job->event.occur_time = clock + next_job->event.service_time;
		delay = clock - next_job->event.arrival_time;
		system_sum.delay += delay;
		M3.station_sum.delay += delay;
		schedule(next_job);
	}
	
	node_event->event.type = ARRIVAL_M1;
	node_event->event.create_time = clock;
	node_event->event.arrival_time = clock;
	node_event->event.service_time = 0.0;
	node_event->event.occur_time = clock;
	schedule(node_event);
}

void arrival_unload(nodePtr node_event){
	UNLOAD.narr++;
	UNLOAD.nsys++;
	nPartsCompleted++;
	node_event->event.service_time = GetServiceUnload();
	node_event->event.create_time = clock;
	node_event->event.type = DEPARTURE_UNLOAD;
	node_event->event.occur_time = clock + node_event->event.service_time;
	total_manufacturing_time += (clock - node_event->event.start_man);
	
	if (node_event->event.isTagged){
		manufacturing_time_sum += (clock - node_event->event.start_man);
		passages_counter++;
	}
	schedule(node_event);
}

void departure_unload(nodePtr node_event){
	double tot_prob = 0.0;

	UNLOAD.station_sum.delay += node_event->event.service_time;
	UNLOAD.station_sum.wait += node_event->event.service_time;
	UNLOAD.nsys--;
	UNLOAD.ncomp++;

	tot_prob = Q_40 + Q_45;
	if (GetRouteProbRec() <= Q_40) {
		node_event->event.type = ARRIVAL_LOAD;
	} else {
		node_event->event.type = ARRIVAL_RECHARGE;
	}

	node_event->event.create_time = clock;
	node_event->event.arrival_time = clock;
	node_event->event.service_time = 0.0;
	node_event->event.occur_time = clock;

	schedule(node_event);
}

void arrival_recharge(nodePtr node_event){
	REC.nsys++;
	REC.narr++;
	node_event->event.create_time = clock;
	node_event->event.service_time = GetServiceRecharge();

	if (REC.nsys == 1) {
		node_event->event.type = DEPARTURE_RECHARGE;
		node_event->event.occur_time = clock + node_event->event.service_time;
		schedule(node_event);
	}
	else {
		node_event->event.type = NO_EVENT;
		node_event->event.occur_time = 0.0;
		enqueue(node_event, &Input_Queue_REC);
	}
}

void departure_recharge(nodePtr node_event){
	nodePtr next_job;
	double delay = 0.0;
	double wait = 0.0;
	
	REC.nsys--;
	REC.ncomp++;
	system_sum.service += node_event->event.service_time;
	REC.station_sum.service += node_event->event.service_time;
	wait = clock - node_event->event.arrival_time;
	system_sum.wait += wait;
	REC.station_sum.wait += wait;

	if (REC.nsys > 0) {
		next_job = dequeue(&Input_Queue_REC);
		next_job->event.type = DEPARTURE_RECHARGE;
		next_job->event.occur_time = clock + next_job->event.service_time;
		delay = clock - next_job->event.arrival_time;
		system_sum.delay += delay;
		REC.station_sum.delay += delay;
		schedule(next_job);
	}

	node_event->event.type = ARRIVAL_LOAD;
	node_event->event.create_time = clock;
	node_event->event.arrival_time = clock;
	node_event->event.service_time = 0.0;
	node_event->event.occur_time = clock;
	schedule(node_event);
}

/*** Service times generation ***/
double GetServiceLoad()
{ 
	SelectStream(10);
  	return Neg_Exp(ZL);
}

double GetServiceM1(){
	SelectStream(50);
	
	// we change the griding time distribution
	// according to the considered version of the system
	if (EXTENDED)
		return Hyper_Exp(ALPHA, BETA, MI_1, MI_2);
	else 
		return Neg_Exp(S1);
}

double GetServiceM2(){
	SelectStream(77);
	return Neg_Exp(S2);
}

double GetServiceM3(){
	SelectStream(123);
	return Neg_Exp(S3);
}

double GetServiceUnload(){
	SelectStream(30);
	return Neg_Exp(ZU);
}

double GetServiceRecharge(){
	SelectStream(60);
	return Neg_Exp(SR);
}

double GetRouteProbM1(){
	SelectStream(200);
	return Route_Prob();
}

double GetRouteProbM2(){
	SelectStream(95);
	return Route_Prob();
}

double GetRouteProbRec(){
	SelectStream(170);
	return Route_Prob();
}


/*** List management operations ***/
nodePtr get_new_node() {
	// before allocating a new block
	// we check whether there already exist a free one
	nodePtr res = pop_from_AL();

	if (res == NULL){
		event_notice ev = {0, " ", 0.0, 0.0, 0.0, 0.0};
		res = new node{ev, NULL, NULL};
	}

	return res;
}

/* function for clearing the pointers and the contents of a node of a list*/
void return_node(nodePtr item){

	if (item->left != NULL)
		item->left->right = NULL;

	if (item->right != NULL)
		item->right->left = NULL;

	
	item->left = NULL;
	item->right = NULL;

	item->event.type = 0;
	item->event.create_time = 0.0;
	item->event.occur_time = 0.0;
	item->event.arrival_time = 0.0;
	item->event.service_time = 0.0;

	push_on_AL(item);
}

/* function for getting a memory block from the Available List managed as a stack */
nodePtr pop_from_AL(){
	nodePtr tmp = NULL;

	if (AL.Head != NULL){
		tmp = AL.Head;
		if (AL.Head->right != NULL)
			AL.Head->right->left = NULL;
		AL.Head = tmp->right;
		tmp->right = NULL;
	}

	return tmp;
}

/* function for storing a memory block in the Available List managed as a stack */
void push_on_AL(nodePtr item){
	
	if (AL.Head == NULL) {
		AL.Head = item;
		AL.Tail = item;
	} else {
		AL.Head->left = item;
		item->right = AL.Head;
		AL.Head = item;
	}
}

/* function for the ordered insertion of a new node in the Future Event List (FEL).
	- The ordering key is represented by the occurrence time.
	- The head of FEL points to the node with the earliest occurr time.
	- The tail of FEL points to the node with the largest occurr time.
*/
void schedule(nodePtr new_node_event){
	nodePtr tmp = FEL.Head;
	nodePtr prev = NULL;
	bool found = false;
	int i = 0;
	
	event_notice ev = new_node_event->event;
	if (tmp != NULL){
		while (tmp != NULL && tmp->event.occur_time < new_node_event->event.occur_time){
			prev = tmp;
			tmp = tmp->right;
		}

		if (prev != NULL){
			if (prev->right == NULL){
				new_node_event->right = NULL;
				FEL.Tail = new_node_event;
			} else {
				prev->right->left = new_node_event;
				new_node_event->right = prev->right;
			}
			prev->right = new_node_event;
			new_node_event->left = prev;
		} else {
			FEL.Head->left = new_node_event;
			new_node_event->right = FEL.Head;
			FEL.Head = new_node_event;
		}
	} else {
		FEL.Head = new_node_event;
		FEL.Tail = new_node_event;
	}
}

/* function to remove the first element from the Future event List (FEL)*/
nodePtr event_pop() {
	nodePtr tmp = NULL;

	if (FEL.Head != NULL) {
		tmp = FEL.Head;
		FEL.Head = FEL.Head->right;
		tmp->right = NULL;
		tmp->left = NULL;
		if (FEL.Head != NULL)
			FEL.Head->left = NULL;
	}

	return tmp;
}

/* function for printing the contents of the Future Event List */
void print_FEL(){
	print_list(&FEL);
}

/* function for printing the contents of a generic Doubly Linked List */
void print_list(dll * dll){
	printf("\n");
	if (dll == NULL) {
		printf("The list is empty\n");
	} else {
		nodePtr tmp = dll->Head;
		event_notice ev;
		while (tmp != NULL){
			ev = tmp->event;

			printf("Event Type: %d\n", ev.type);
			printf("Event Name: %s\n", ev.name);
			printf("Event Creation Time: %8.6f\n", ev.create_time);
			printf("Event Occurrence Time: %8.6f\n", ev.occur_time);
			printf("Event Arrival Time: %8.6f\n", ev.arrival_time);
			printf("Event Service Time: %8.6f\n", ev.service_time);
			printf("Event Remaining Service Time: %8.6f\n", ev.rem_service_time);
			printf("\n");

			tmp = tmp->right;
		}
	}
}

void print_results(){
		
	printf("\n-------------------------------------------------------------------------");
	printf("\nSIMULATION RUN STATISTICS");
    printf("\n-------------------------------------------------------------------------");
    printf("\nNumber of processed events            = %d",event_counter);
	printf("\nLength of Observation Period          = %10.6f", ObservPeriod);
	printf("\n-------------------------------------------------------------------------");	
   
	printf("\n-------------------------------------------------------------------------");
	printf("\nSYSTEM-LEVEL STATISTICS");
    printf("\n-------------------------------------------------------------------------");
	printf("\nParts started: %ld\n", nPartsStarted);
	printf("\nParts completed: %ld\n", nPartsCompleted);
	printf("\nAverage manufacturing time (total sum) = %8.6f\n", total_manufacturing_time / nPartsCompleted);
	printf("\nAverage manufacturing time (averages' sum) = %8.6f\n", 
		(M1.station_sum.wait / M1.ncomp) +
		(M2.station_sum.wait / M2.ncomp) +
		(M3.station_sum.wait / M3.ncomp)
	);
	printf("\nAverage response time (total sum) = %8.6f\n", total_response_time / nPartsCompleted);
	printf("\nAverage response time (averages' sum) = %8.6f\n",
		(M1.station_sum.wait / M1.ncomp) +
		(M2.station_sum.wait / M2.ncomp) +
		(M3.station_sum.wait / M3.ncomp) +
		(UNLOAD.station_sum.wait / UNLOAD.ncomp) +
		(REC.station_sum.wait / REC.ncomp)
	);
	printf("\nAverage cycle time (total sum) = %8.6f\n", total_cycle_time / nPartsCompleted);
	printf("\nAverage cycle time (averages' sum) = %8.6f\n",
		(LOAD.station_sum.wait / LOAD.ncomp) +
		(M1.station_sum.wait / M1.ncomp) +
		(M2.station_sum.wait / M2.ncomp) +
		(M3.station_sum.wait / M3.ncomp) +
		(UNLOAD.station_sum.wait / UNLOAD.ncomp) +
		(REC.station_sum.wait / REC.ncomp)
	);

	printf("\n-------------------------------------------------------------------------");
	printf("\nLOAD STATION STATISTICS");
    printf("\n-------------------------------------------------------------------------");
	print_station_results(LOAD);

	printf("\n-------------------------------------------------------------------------");
	printf("\nM1 STATION STATISTICS");
    printf("\n-------------------------------------------------------------------------");
	print_station_results(M1);

	printf("\n-------------------------------------------------------------------------");
	printf("\nM2 STATION STATISTICS");
    printf("\n-------------------------------------------------------------------------");
	print_station_results(M2);

	printf("\n-------------------------------------------------------------------------");
	printf("\nM3 STATION STATISTICS");
    printf("\n-------------------------------------------------------------------------");
	print_station_results(M3);

	printf("\n-------------------------------------------------------------------------");
	printf("\nUNLOAD STATION STATISTICS");
    printf("\n-------------------------------------------------------------------------");
	print_station_results(UNLOAD);

	printf("\n-------------------------------------------------------------------------");
	printf("\nRECHARGE STATION STATISTICS");
    printf("\n-------------------------------------------------------------------------");
	print_station_results(REC);

	printf("\n-------------------------------------------------------------------------");
	printf("\nMANUFACTURING TIME CONFIDENCE INTERVAL");
    printf("\n-------------------------------------------------------------------------");
	printf("\nPrecision: %8.6f", reg_samp.precision);
	printf("\nMean value: %.4f", reg_samp.mi);
	printf("\nConfidence interval: (%8.6f, %8.6f)\n", reg_samp.mi - reg_samp.width, reg_samp.mi + reg_samp.width);
	
}

// Function that prints station-level performance measures
void print_station_results(SSQ_station st){
	printf("\n%ld arrived jobs\n", st.narr);
	printf("\n%ld completed jobs\n", st.ncomp);
	printf("Average service time .... = %8.6f\n", st.station_sum.service / st.ncomp);
	printf("Average delay ........... = %8.6f\n", st.station_sum.delay / st.ncomp);
	printf("Average wait ............ = %8.6f\n", st.station_sum.wait / st.ncomp);
  	printf("Input rate = %8.6f\n", st.narr / ObservPeriod);
  	printf("Service rate = %8.6f\n", st.ncomp / st.station_sum.service);
  	printf("Throughput = %8.6f\n", st.ncomp / ObservPeriod);
  	printf("Utilization = %8.6f\n", st.station_sum.service / ObservPeriod);
	printf("Average number of customers in the system = %8.6f\n", st.Area_n / ObservPeriod);
  	printf("Average number of customers waiting in queue = %8.6f\n", st.Area_q / ObservPeriod);
	printf("Average number of customers in service = %8.6f\n", st.Area_y / ObservPeriod);
}

void report(){
	
    ObservPeriod = Stop - START;

	print_results();
	
	destroy_list(FEL.Head);
	destroy_list(Input_Queue_M1.Head);
	destroy_list(Input_Queue_M2.Head);
	destroy_list(Input_Queue_M3.Head);
	destroy_list(Input_Queue_REC.Head);
	destroy_list(AL.Head);
}

// detects a new regeneration point
bool RegPoint(event_notice ev) {
	bool is_reg_point = false;

	if (EXTENDED)
		is_reg_point = ev.type == ARRIVAL_M1 && M1.nsys == 0 && ev.num_visits_M1 == 0 && ev.isTagged;
	else
		is_reg_point = ev.type == ARRIVAL_M1 && ev.num_visits_M1 == 0 && ev.isTagged;

	if (is_reg_point) {
		reg_samp.num_cycles++;
		num_obs--;
		if (reg_samp.num_cycles > 1)
			CollectRegStatistics();
	}

	return is_reg_point;
}

// collects statics on the last regeneration cycle
void CollectRegStatistics(){
	regeneration_pair rp = {manufacturing_time_sum, passages_counter};
	reg_samp.observations.push_back(rp);
	reg_samp.S_a += manufacturing_time_sum;
	reg_samp.S_v += passages_counter;
	reg_samp.S_aa += manufacturing_time_sum * manufacturing_time_sum;
	reg_samp.S_av += manufacturing_time_sum * passages_counter;
	reg_samp.S_vv += passages_counter * passages_counter;

	if (print_statistics){
		printf("\n-------------------------------------------------------------------------");
		printf("\nMEASUREMENTS FOR THE %d REGENERATION CYCLES", reg_samp.num_cycles);
    	printf("\n-------------------------------------------------------------------------");
		printf("\nManufacturing time sum: %8.6f", manufacturing_time_sum);
		printf("\nNumber of passages in the subsystem: %d", passages_counter);
		printf("\nreg_samp.S_a: %8.6f", reg_samp.S_a);
		printf("\nreg_samp.S_v: %8.6f", reg_samp.S_v);
		printf("\nreg_samp.S_aa: %8.6f", reg_samp.S_aa);
		printf("\nreg_samp.S_av: %8.6f", reg_samp.S_av);
		printf("\nreg_samp.S_vv: %8.6f", reg_samp.S_vv);
	}

	ResetMeasures();
}

void ResetMeasures(){
	manufacturing_time_sum = 0.0;
	passages_counter = 0.0;
}

void ComputeConfidenceInterval(){
	double mi;
	double width;
	double term_one;
	double term_two;


	reg_samp.mi = reg_samp.S_a / reg_samp.S_v;
	term_one = sqrt(reg_samp.num_cycles / (float)(reg_samp.num_cycles - 1));
	reg_samp.stdev = sqrt((1 / (float)(reg_samp.num_cycles - 1)) * (reg_samp.S_aa - (2 * reg_samp.mi * reg_samp.S_av) + (reg_samp.mi * reg_samp.mi * reg_samp.S_vv)));
	term_two = sqrt(reg_samp.S_aa - (2 * reg_samp.mi * reg_samp.S_av) + (reg_samp.mi * reg_samp.mi * reg_samp.S_vv)) / reg_samp.S_v;
	reg_samp.width = PERCENTILE * term_one * term_two;
	reg_samp.precision = reg_samp.width / reg_samp.mi; 
}

bool DecideToStop(){
	bool isOver = false;

	if (num_obs == 0){
		ComputeConfidenceInterval();

		if (reg_samp.precision <= PRECISION + 0.0005){
			Stop = clock;
			isOver = true;
		} else {
			// computes new target value for sample size
			num_obs = floor(pow((PERCENTILE * reg_samp.stdev) / (reg_samp.mi * PRECISION), 2)) - reg_samp.num_cycles;
		}
	}
	
	return isOver;
}
