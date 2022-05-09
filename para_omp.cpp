#include <iostream>
#include <math.h>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <tuple>
#include <vector>
#include <bits/stdc++.h>
#include <string>
#include <omp.h>
#include <time.h>

using namespace std;

// function declaration
double** buildDistMatrix(pair<double,double> coords[],int dimension,int NUM_OF_THREADS);
vector<tuple<int, int, double>> compSaving(double** distMat,int dimension, int NUM_OF_THREADS);
bool sortBySaving(const tuple<int, int, double>& a, const tuple<int, int, double>& b);
vector<tuple<int, int, double>> sortSavingList(vector<tuple<int, int, double>> v, int NUM_OF_THREADS);
void mergeRoutes(vector<tuple<int, int, double>> v, int max_capacity, int* demand, int NUM_OF_THREADS);
bool onSameRoute(int i, int j, vector<int> route);
bool edgeNode(int i, vector<tuple<vector<int>, int>> routes, int NUM_OF_THREADS);
tuple<int, int> overlap(int i, int j, vector<tuple<vector<int>, int>> routes);

int main(int argc, char *argv[]) {

  int NUM_OF_THREADS = atoi(argv[1]);
  //this is the overall basic megadata for the data
  int dimension,capacity;
  //declearation of the output data from the files
  pair<double,double>* loc = new pair<double,double>[1000];
  int* demand = new int[1000];
  string line;
  
  ifstream myfile ("M-n200-k16.vrp");
  if (myfile.is_open())
  {
    //Readpass the headers
    for(int i=0;i<4;i++)
      getline (myfile,line);
    
    //extract the dimesnsion and capacity data
    int startIndex = 12;
    dimension = stoi(line.substr(startIndex));
    getline (myfile,line);
    getline (myfile,line);
    startIndex = 11;
    capacity = stoi(line.substr(startIndex));
    getline (myfile,line);

    //Reading of the location data
    //Data headers: index, x, and y
    string index,x,y;
    for(int i=0;i<dimension;i++)
    {
      getline (myfile,line);
      istringstream ss(line);
      ss >> index;
      ss >> x;
      ss >> y;
      loc[stoi(index)] = make_pair(stod(x),stod(y));
    }
    getline (myfile,line);

    //Reading the demand data
    //Data headers: index, demand
    for(int i=0;i<dimension;i++)
    {
      getline (myfile,line);
      istringstream ss(line);
      ss >> index;
      ss >> x;
      demand[stoi(index)] = stoi(x);
    }
    getline (myfile,line);
    getline (myfile,line);

    //Read the location data for the start location
    istringstream ss(line);
    ss >> x;
    getline (myfile,line);
    istringstream ss2(line);
    ss2 >> y;
    loc[0] = make_pair(stoi(x),stoi(y));

    //final maniputlations of the file
    myfile.close();
  }

  else cout << "Failed to Open the File. Please try again later."; 

  //initialization for time calculation
	struct timespec start, stop; 
	double time;

    //start calculating the time
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}
  
  // read dataset
  // N = # of locations, N+1 = locations + central depot
  // coords matrix: 1d
  // demand matrix: 1d
  // central depot
  // capacity

  // step1: generate pair-wise distance matrix
  // assumption: no 2 locations overlap in the 2-D coordinates system
  double** distance = buildDistMatrix(loc,dimension,NUM_OF_THREADS);

  // step2: compute pair-wise savings
  vector<tuple<int, int, double>> savingList = compSaving(distance,dimension,NUM_OF_THREADS);

  // step3: sort saving list by savings
  sortSavingList(savingList,NUM_OF_THREADS);

  //step4: merge routes until no additional savings
  mergeRoutes(savingList, capacity, demand,NUM_OF_THREADS);

	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}	
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;

	// print out the execution time here
	cout << "Parallel execution time = " + to_string(time) + " sec.";
  
  return 0;
  
}


// step1: compute distance matrix
double** buildDistMatrix(pair<double,double> coords[],int dimension, int NUM_OF_THREADS){
  //This is the distance matrix used to calcualate the savings
  double** distMat = 0;
  distMat = new double*[dimension+1];

  //For reference, if we would like to use some arbitrary number of processors
  //#pragma omp parallel num_threads(dimension*(dimension+1)/2);
  
  #pragma omp parallel num_threads(NUM_OF_THREADS)
    #pragma omp for schedule(dynamic)
      for(int i=0;i<dimension + 1;i++) //**
      {
        distMat[i] = new double[dimension+1];
        //The distance between two coordinates used Pythagorean theorem
        for(int j=0;j<i;j++)
          distMat[i][j] = sqrt(pow(coords[i].first-coords[j].first,2) \
            + pow(coords[i].second-coords[j].second,2));
      }
  return distMat;
}

// step2: compute pair-wise savings
vector<tuple<int, int, double>> compSaving(double** distMat,int dimension, int NUM_OF_THREADS){

  vector<tuple<int, int, double>> savingList;

  //For reference, if we would like to use some arbitrary number of processors
  //#pragma omp parallel num_threads(dimension*(dimension-1)/2);
  
  #pragma omp parallel num_threads(NUM_OF_THREADS)
    #pragma omp for schedule(dynamic)
      for (int j = 1; j < dimension; j++){
        for (int i = j + 1; i < dimension + 1; i++){
              #pragma omp critical
                savingList.push_back(tuple<int, int, double>(i, j, distMat[i][0] + distMat[j][0] - distMat[i][j]));
        }
      }

  return savingList;
    
}

// step3: sort savings
bool sortBySaving(const tuple<int, int, double>& a, const tuple<int, int, double>& b){
    return (get<2>(a) > get<2>(b));
}

vector<tuple<int, int, double>> sortSavingList(vector<tuple<int, int, double>> v, int NUM_OF_THREADS){

  int len = v.size();
  
  #pragma omp parallel num_threads(NUM_OF_THREADS)
    #pragma omp for schedule(dynamic)
    for (int i = 0; i < NUM_OF_THREADS; i++){
      sort(v.begin() + floor(i * len/NUM_OF_THREADS), v.begin() + floor((i+1) * len/NUM_OF_THREADS), sortBySaving);
    }
  
  return v;
    
}

// step4: merge routes according to list of savings
void mergeRoutes(vector<tuple<int, int, double>> v, int max_capacity, int* demand, int NUM_OF_THREADS){

  // list of routes in form (<list of intermediate nodes>, current capacity)
  vector<tuple<vector<int>, int>> routes;
  // loop through saving list
  for (int k = 0; k < v.size(); k++){
    int i = get<0>(v[k]);
    int j = get<1>(v[k]);
      
    // if i-j over max capacity, seperate them as 2 stanalone routes
    if (demand[i] + demand[j] > max_capacity) {
        routes.push_back(tuple<vector<int>, int>(vector<int>{i}, demand[i]));
        routes.push_back(tuple<vector<int>, int>(vector<int>{j}, demand[j]));
    } else {
      // if overlap of routes exists, merge according to the overlap
      if (get<0>(overlap(i, j, routes)) == 1){
        int position = get<1>(overlap(i, j, routes));
        vector<int> route = get<0>(routes[position]);
        int curr_capacity = get<1>(routes[position]);
        if (i == route[0]){
          if (curr_capacity + demand[j] <= max_capacity){
              route.insert(route.begin(), j);
          } else {
              routes.push_back(tuple<vector<int>, int>(vector<int>{j}, demand[j]));
          }
        } else if (i == route.back()){
          if (curr_capacity + demand[j] <= max_capacity){
              route.push_back(j);
          } else {
              routes.push_back(tuple<vector<int>, int>(vector<int>{j}, demand[j]));
          }
        } else if (j == route[0]){
          if (curr_capacity + demand[i] <= max_capacity){
              route.insert(route.begin(), i);
          } else {
              routes.push_back(tuple<vector<int>, int>(vector<int>{i}, demand[i]));
          }
        } else if (j == route.back()){
          if (curr_capacity + demand[i] <= max_capacity){
              route.push_back(i);
          } else {
              routes.push_back(tuple<vector<int>, int>(vector<int>{i}, demand[i]));
          }
        }
          
      } else if (get<0>(overlap(i, j, routes)) == 0){ // no overlap, brute-force merge
        int done = 0;
        // find the first eligible route to merge
          for (int l = 0; l < routes.size(); l++){
            vector<int> route = get<0>(routes[l]);
            int curr_capacity = get<1>(routes[l]);
            if (done == 0 && curr_capacity + demand[i] + demand[j] <= max_capacity){
                route.push_back(i); // merge i-j to this route
                route.push_back(j);
                done = 1;
            }
          }
    
        // if no route-merge, then make i-j a new route
          routes.push_back(tuple<vector<int>, int>(vector<int>{i, j}, demand[i] + demand[j]));
  
      }
        
    }
    
  }
  
}

bool onSameRoute(int i, int j, vector<int> route){
  
  bool found = 0;
  if (find(route.begin(), route.end(), i) != route.end() && find(route.begin(), route.end(), j) != route.end()){
      found = 1;
  }
  
  return found;
  
}

// true if either on the edge of a route or standalone
bool edgeNode(int i, vector<tuple<vector<int>, int>> routes, int NUM_OF_THREADS){
  bool found = 0;
  #pragma omp parallel num_threads(NUM_OF_THREADS)
    #pragma omp for schedule(dynamic)
    for (int k = 0; k < routes.size(); k++){
      vector<int> route = get<0>(routes[k]);
      if (find(route.begin(), route.end(), i) != route.end()){
        if (route[0] == i || route.back() == i) {
        found = 1;
        } else {found = 0;}
      } else {found = 1;}
      
    }
  return found;
}

tuple<int, int> overlap(int i, int j, vector<tuple<vector<int>, int>> routes){

  tuple<int, int> result;
  result = make_tuple(0, -1);
  // return value in the form of (fonud, position in route list)
  // found = {0, 1, 2}, representing "not found", "found", "found with both nodes overlap"
  // position = {k, -1}, -1 represents "not found"

    for (int k = 0; k < routes.size(); k++){
      vector<int> route = get<0>(routes[k]);
      
      if (find(route.begin(), route.end(), i) != route.end() || find(route.begin(), route.end(), j) != route.end()){
        if (onSameRoute(i, j, route)){
            result = make_tuple(2, k);
        } else { 
            result = make_tuple(1, k); 
        }
      }
    }
  
  return result;
  
}

