#include "random_var_gen.h"
#include "rngs.h"
#include <math.h>

double Neg_Exp(double lambda){
	// generates a random number between 0 and 1
	double par = 1 / lambda;

	double u = Random();
	// trasforms the uniform into an exponential
	return -log(u) / par;
}

double Hyper_Exp(double alpha, double beta, double mi1, double mi2){
	double par3 = 1 / mi1;
	double par4 = 1 / mi2;

	double y = Random();

	beta += alpha;

	if (y <= alpha)
		return Neg_Exp(mi1);
	else
		return Neg_Exp(mi2);
}

double Route_Prob(){
	return Random();
}