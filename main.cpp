#include <iostream>
#include <math.h>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <tuple>
#include <vector>
#include <bits/stdc++.h>
#include <string>
#include <time.h>

using namespace std;

// function declaration
double** buildDistMatrix(pair<double,double> coords[],int dimension);
vector<tuple<int, int, double>> compSaving(double** distMat,int dimension);
bool sortBySaving(const tuple<int, int, double>& a, const tuple<int, int, double>& b);
vector<tuple<int, int, double>> sortSavingList(vector<tuple<int, int, double>> v);
void mergeRoutes(vector<tuple<int, int, double>> v, int max_capacity, int* demand);
bool onSameRoute(int i, int j, vector<int> route);
bool edgeNode(int i, vector<tuple<vector<int>, int>> routes);
tuple<int, int> overlap(int i, int j, vector<tuple<vector<int>, int>> routes);

int main() {
  
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
  // dimension = # of locations, dimension+1 = locations + central depot
  // coords matrix: 1d
  // demand matrix: 1d

  // step1: generate pair-wise distance matrix
  // assumption: no 2 locations overlap in the 2-D coordinates system
  double** distance = buildDistMatrix(loc,dimension);

  // step2: compute pair-wise savings
  vector<tuple<int, int, double>> savingList = compSaving(distance,dimension);

  // step3: sort saving list by savings
  sortSavingList(savingList);

  //step4: merge routes until no additional savings
  mergeRoutes(savingList, capacity, demand);

  if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}	
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;

	// print out the execution time here
	cout << "Serial execution time = " + to_string(time) + " sec.";
  
  return 0;
  
}


// step1: compute distance matrix
double** buildDistMatrix(pair<double,double> coords[],int dimension){
  //This is the distance matrix used to calcualate the savings
  double** distMat = 0;
  distMat = new double*[dimension+1];
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
vector<tuple<int, int, double>> compSaving(double** distMat,int dimension){

  vector<tuple<int, int, double>> savingList;
  
  for (int j = 0; j < dimension + 1; j++){ //**
    for (int i = 0; i < dimension + 1; i++){ //**
      if (i > j){
        double saving = distMat[i][0] + distMat[j][0] - distMat[i][j];
        savingList.push_back(tuple<int, int, double>(i, j, saving));
      }
    }
  }

  return savingList;
    
}

// step3: sort savings
vector<tuple<int, int, double>> sortSavingList(vector<tuple<int, int, double>> v) {
  
  int size = v.size();
  for (int step = 0; step < size; ++step) {
    for (int i = 0; i < size - step; ++i) {
      if (get<2>(v[i]) < get<2>(v[i + 1])) {
        int temp = get<2>(v[i]);
        get<2>(v[i]) = get<2>(v[i+1]);
        get<2>(v[i+1]) = temp;
      }
    }
  }
  return v;
}

// step4:
void mergeRoutes(vector<tuple<int, int, double>> v, int max_capacity, int* demand){

  // list of routes in form (<list of intermediate nodes>, current capacity)
  vector<tuple<vector<int>, int>> routes;
  int k = 0;

  // loop through saving list
  while (k < v.size()){
    
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
  
        int l = 0;
        bool done = 0;
        while (l < routes.size() && done != 1){ // find the first eligible route to merge
          vector<int> route = get<0>(routes[l]);
          int curr_capacity = get<1>(routes[l]);
          if (curr_capacity + demand[i] + demand[j] <= max_capacity){
            route.push_back(i); // merge i-j to this route
            route.push_back(j);
            done = 1;
          }
          l++;
        }
  
        // if no route-merge, then make i-j a new route
        routes.push_back(tuple<vector<int>, int>(vector<int>{i, j}, demand[i] + demand[j]));
        
          
      }
      
    }
    k++;
    
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
bool edgeNode(int i, vector<tuple<vector<int>, int>> routes){
  bool found = 0;
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

  // return value in the form of (fonud, position in route list)
  // found = {0, 1, 2}, representing "not found", "found", "found with both nodes overlap"
  // position = {k, -1}, -1 represents "not found"
  int k = 0;
  while (k < routes.size()){
    vector<int> route = get<0>(routes[k]);
    if (find(route.begin(), route.end(), i) != route.end() || find(route.begin(), route.end(), j) != route.end()){
      if (onSameRoute(i, j, route)){
        return tuple<int, int>(2, k);
      } else {return tuple<int, int>(1, k);}
    }
    k++;
  }
  
  return tuple<int, int>(0, -1);
  
}