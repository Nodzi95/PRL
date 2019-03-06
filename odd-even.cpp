/*
 * algorithm: odd-even transposition sort (alg. ~40 lines long)
 * author: jakub zak
 *
 */

 #include <mpi.h>
 #include <iostream>
 #include <fstream>
 #include <math.h>
 
 using namespace std;

 #define TAG 0


void swap(int array[], int left, int right){
	int tmp = array[right];
	array[right] = array[left];
	array[left] = tmp;
}

void siftDown(int array[], int bottom, int top){
	int temp = array[top];
	int succ = top*2+1;
	if(succ < bottom && array[succ] < array[succ+1]) succ++;

	while(succ <= bottom && temp < array[succ]){
		array[top] = array[succ];
		top = succ;
		succ = succ*2+1;
		if(succ < bottom && array[succ] < array[succ+1]) succ++;
	}
	array[top] = temp;
}

void heapSort(int array[], int size){
	for(int i = size/2 -1; i >= 0; i--){
		siftDown(array, size -1, i);
	}
	for(int i = size -1; i >0; i--){
		swap(array, 0, i);
		siftDown(array, i-1, 0);
	}
}


 int main(int argc, char *argv[])
 {
    int numprocs;               //pocet procesoru
    int myid;                   //muj rank
    int neighnumber;            //hodnota souseda
    int *mynumber;               //moje hodnota
    int Ginvar;
    bool test = false;
    MPI_Status stat;            //struct- obsahuje kod- source, tag, error

    //MPI INIT
    MPI_Init(&argc,&argv);                          // inicializace MPI 
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // zjistíme, kolik procesů běží 
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);           // zjistíme id svého procesu 
 
    //NACTENI SOUBORU
    /* -proc s rankem 0 nacita vsechny hodnoty
     * -postupne rozesle jednotlive hodnoty vsem i sobe
    */
    if(myid == 0){
		int listProc = 0;
		int nums = 0;
	    char input[]= "numbers";                          //jmeno souboru    
	    int num;                                     //hodnota pri nacitani souboru
	    int invar= 0;                                   //invariant- urcuje cislo proc, kteremu se bude posilat
	    fstream fin;                                    //cteni ze souboru
	    int NUMS = 0;				
	    fin.open(input, ios::in);
		fin.seekg(0, fin.end);                   
		nums = fin.tellg();
		NUMS = nums;
		fin.seekg(0, fin.beg); 
		int numINarray = 0;
		int *numArray;
		//cout << "list " <<nums << endl;
		if(nums <= 1){
			if(nums == 1){
				int num = fin.get();
				cout << num << endl;
				cout << num << endl;
			}
			fin.close();
			MPI_Finalize(); 
    		return 0;
		}
		listProc = log(nums)/log(2);
		

		int tmp = listProc;
		int members = nums / tmp;
		//cout << members << endl;
		invar = numprocs - listProc;					
		numArray = (int *)malloc(members*sizeof(int));
		
	    while(fin.good()){
	        num= fin.get();

		    if(numINarray == members){
				//cout << invar <<endl;
				if(NUMS <= 3){
					heapSort(numArray, NUMS);
					cout<<endl;
					for(int i =0; i < NUMS; i++){
						cout <<numArray[i]<< endl;
					}
					fin.close();
					MPI_Finalize(); 
		    		return 0;
				}
				nums = nums - members;
				tmp--;

		        MPI_Send(numArray, members, MPI_INT, invar, TAG, MPI_COMM_WORLD); //buffer,velikost,typ,rank prijemce,tag,komunikacni skupina
				//cout << invar << " s:s " << &mynumber<<endl;
		        //cout << invar << " : "<< members <<endl;
		        invar++;
				if(tmp==0 || nums == 0);
				else members = nums / tmp;

				free(numArray);
				if(!fin.good()){
					Ginvar = members;
					break;
			    }
				
				numArray = (int *)malloc(members*sizeof(int));
				numINarray = 0;
		    }
		    cout << num << " ";
			//cout<<invar<<":"<<num<<endl;             //kdo dostane kere cislo
			numArray[numINarray++] = num;
			//cout << numINarray << endl;    
	    }//while
	    
	    fin.close();                                
    }//nacteni souboru

    //PRIJETI HODNOTY CISLA
    //listove procesory prijmou sekvence cisel od mastera
    if(myid >= numprocs/2 && numprocs != 0){
		int delitel = ceil(numprocs/2.0);
		int memory = ceil(pow(2, delitel)/(delitel/2.0));					//kolik muzu maximalni prijmout
		int numbersReceived = 0;											//kolik jsem prijal

		mynumber = (int *)malloc(memory*sizeof(int));
		MPI_Recv(mynumber, memory, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat); //buffer,velikost,typ,rank odesilatele,tag, skupina, stat
	    MPI_Get_count(&stat, MPI_INT, &numbersReceived);
	    //cout << numbersReceived << endl;
	    //SORTING
	    heapSort(mynumber, numbersReceived);
	    
		//cout << endl;
		if(myid%2 == 0){
			//cout << myid<<": sending to " << (myid-1)/2 << endl;
			MPI_Send(mynumber, numbersReceived, MPI_INT, (myid-1)/2, TAG, MPI_COMM_WORLD);
		}
		else{
			//cout << myid<<": sending to " << myid/2 << endl;
			MPI_Send(mynumber, numbersReceived, MPI_INT, myid/2, TAG, MPI_COMM_WORLD);
		}
		free(mynumber);
    }

    if(myid < numprocs/2){
    	for(int i = numprocs/2 -1; i >= 0; i--){
    		if(myid == i){
    			int delitel = ceil(numprocs/2.0);
				int memory = pow(2, delitel+1)-1;
				int numbersReceived = 0;											//kolik jsem prijal
				int numbersReceived1 = 0;
				int numbersReceived2 = 0;
		    	//cout << "myid: " << myid <<" can receive: " << memory <<endl;
		    	
		    	int *mynumber1 = (int *)malloc(memory*sizeof(int));
		    	int *mynumber2 = (int *)malloc(memory*sizeof(int));
		    	//cout << myid<<": waiting for: " << i*2+1 << ", " << i*2+2 << endl;
    			if(i*2 +1 < numprocs-1){
    				MPI_Recv(mynumber1, memory, MPI_INT, i*2+1, TAG, MPI_COMM_WORLD, &stat); //buffer,velikost,typ,rank odesilatele,tag, skupina, stat
	    			MPI_Get_count(&stat, MPI_INT, &numbersReceived1);
	    		}
		    	if(i*2+2 < numprocs ){
		    		//cout << i<< ": waiting for " << i*2 +2 << endl;
		    		MPI_Recv(mynumber2, memory, MPI_INT, i*2+2, TAG, MPI_COMM_WORLD, &stat); //buffer,velikost,typ,rank odesilatele,tag, skupina, stat
	    			MPI_Get_count(&stat, MPI_INT, &numbersReceived2);
		    	}
		    	int arr1 = 0;
		    	int arr2 = 0;
		    	mynumber = (int *)malloc((numbersReceived1 + numbersReceived2)*sizeof(int));
		    	for(int i = 0; i < numbersReceived1 + numbersReceived2; i++){
		    		if(arr1 < numbersReceived1){
		    			if(arr2 < numbersReceived2){
		    				if(mynumber1[arr1] <= mynumber2[arr2]){
				    			mynumber[i] = mynumber1[arr1++];
				    		}
				    		else{
				    			mynumber[i] = mynumber2[arr2++];
				    		}
		    			}
		    			else{
		    				mynumber[i] = mynumber1[arr1++];
		    			}
		    		}
		    		else{
		    			mynumber[i] = mynumber2[arr2++];
		    		}
		    		
		    		//cout << mynumber[i] << ", ";
		    	}
		    	//cout << endl;
		    	if(myid !=0){
		    		if(myid%2 == 0){
						//cout << myid<<": sending to " << (myid-1)/2 << endl;
						MPI_Send(mynumber, numbersReceived1 +numbersReceived2, MPI_INT, (myid-1)/2, TAG, MPI_COMM_WORLD);
					}
					else{
						//cout << myid<<": sending to " << myid/2 << endl;
						MPI_Send(mynumber, numbersReceived1 +numbersReceived2, MPI_INT, myid/2, TAG, MPI_COMM_WORLD);
					}
		    	}
		    	else{
		    		cout<<endl;
					for(int i =0; i < numbersReceived1+numbersReceived2; i++){
						cout <<mynumber[i]<< endl;
					}
		    	}
		    	
				free(mynumber);
				free(mynumber1);
				free(mynumber2);
    		}


    	}
    	
    	
    }
 
    
    
    MPI_Finalize(); 
    return 0;

 }//main

