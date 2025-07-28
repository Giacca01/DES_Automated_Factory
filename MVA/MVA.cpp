#include "MVA.h"

int main(void){

    // stations service times
    double S[] = {60.0, 9.0, 120.0, 80.0, 50.0, 100.0};
    // Routing probabilities
    double Q[6][6] = {
        {0.0, 1.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.9, 0.1, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.2, 0.8, 0.0},
        {0.0, 1.0, 0.0, 0.0, 0.0, 0.0},
        {0.6, 0.0, 0.0, 0.0, 0.0, 0.4},
        {1, 0.0, 0.0, 0.0, 0.0, 0.0},
    };
    char stations_types[] = {'D', 'I', 'I', 'I', 'D', 'I'};

    mva(6, 30, S, Q, stations_types);
}

void mva(int M, int N, double * S, double Q[6][6], char * stations_types){
    // row i contains performance measures for traffic level i
    int total_N = N + 1;
    double ** mean_n = new double * [total_N];
    double ** mean_w = new double * [total_N];
    double ** mean_thr = new double * [total_N];
    double ** utilization = new double * [total_N];
    double ** mean_station_time = new double * [total_N];
    double * mean_response_time = new double[total_N];
    double * mean_manufacturing_time = new double[total_N];
    double * mean_cycle_time = new double[total_N];
    double * thrp_ref = new double[total_N];
    double max_Ut = 0.0;
    int bottleneck_index = 0;

    int i;
    int k;
    double sum;
    int ref_station = 0;

    //std::vector<double> V = compute_visit_counts(M, Q);

    std::vector<double> V = {1, 12.5, 1.25, 0.25, 1, 0.4};

    
    // initialization
    for (k = 0; k < M; k++) {
        mean_n[0] = new double[M];
        mean_n[0][k] = 0.0;
    }

    // computes performance indices for all possible traffic levels
    for (i = 1; i <= N; i++){
        mean_response_time[i] = 0.0;
        mean_manufacturing_time[i] = 0.0;
        mean_cycle_time[i] = 0.0;
        mean_n[i] = new double[M];
        mean_w[i] = new double[M];
        mean_thr[i] = new double[M];
        utilization[i] = new double[M];
        mean_station_time[i] = new double[M];

        printf("For traffic level %d:\n", i);
        // waiting time computation
        printf("Average waiting time: \t");
        for (k = 0; k < M; k++){
            if (stations_types[k] == 'D')
                mean_w[i][k] = S[k];
            else
                mean_w[i][k] = S[k] * (1 + mean_n[i - 1][k]);
            printf("%f\t", mean_w[i][k]);
        }
        printf("\n");

        // throughput computation for reference station
        sum = 0.0;
        for (k = 0; k < M; k++){
            sum += V[k] * mean_w[i][k];
        }
        
        mean_thr[i][ref_station] = i / sum;

        // computation of throughtput and other indices
        // that depends on it for all other stations
        for (k = 0; k < M; k++){
            mean_station_time[i][k] = V[k] * mean_w[i][k];

            if (k != ref_station){
                mean_thr[i][k] = V[k] * mean_thr[i][ref_station];
                mean_response_time[i] += mean_station_time[i][k];
            }

            if (stations_types[k] == 'D'){
                mean_n[i][k] = S[k] * mean_thr[i][k];
                utilization[i][k] =  mean_n[i][k] / i;
            } else {
                utilization[i][k] = S[k] * mean_thr[i][k];
                mean_n[i][k] = utilization[i][k] * (1 + mean_n[i - 1][k]);
            }

            if (utilization[i][k] > max_Ut){
                max_Ut = utilization[i][k];
                bottleneck_index = k;
            }
        }

        mean_manufacturing_time[i] = mean_station_time[i][1] + mean_station_time[i][2] + mean_station_time[i][3];
        mean_cycle_time[i] = mean_response_time[i] + mean_station_time[i][ref_station];


        thrp_ref[i] = mean_thr[i][0];
        printf("Mean Throughput: \t");
        for (k = 0; k < M; k++)
            printf("%.12f\t", mean_thr[i][k]);
        printf("\n");

        printf("Mean Utilization: \t");
        for (k = 0; k < M; k++)
            printf("%.12f\t", utilization[i][k]);
        printf("\n");

        printf("Mean queue length: \t");
        for (k = 0; k < M; k++)
            printf("%.12f\t", mean_n[i][k]);   
        printf("\n");     

        printf("Mean time at station: \t");
        for (k = 0; k < M; k++)
            printf("%.12f\t", mean_station_time[i][k]);
        printf("\n");

        printf("Mean response time: \t");
        printf("%.12f\t\n", mean_response_time[i]);

        printf("Mean manufacturing time: \t");
        printf("%.12f\t\n", mean_manufacturing_time[i]);

        printf("Mean cycle time: \t");
        printf("%.12f\t\n", mean_cycle_time[i]);

        printf("\n\n");
    }

    printf("Bottleneck station: \t");
    printf("%d\n", bottleneck_index);

    saveToFile(N, thrp_ref, TP_FILENAME);
    saveToFile(N, mean_response_time, RP_FILENAME);
    saveToFile(N, mean_manufacturing_time, MAN_FILENAME);
}

// Save results on file for plot generation
void saveToFile(int N, double * data, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

    for (int i = 1; i <= N; i++){
        file << std::fixed << std::setprecision(6) << i << " " << data[i] << "\n";
    }

    file.close();
    std::cout << "Data successfully saved to " << filename << std::endl;
}