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
        fin.open(input, ios::in);
	fin.seekg(0, fin.end);                   
	nums = fin.tellg();
	fin.seekg(0, fin.beg); 
	int numINarray = 0;
	int *numArray;
	
	listProc = log(nums)/log(2);
	
	int tmp = listProc;
	int members = nums / tmp;
	cout << members << endl;
	invar = numprocs - listProc;			//send to list processors
	numArray = (int *)malloc(members*sizeof(int));
	
        while(fin.good()){
            num= fin.get();

	    if(numINarray == members){
		//cout << invar <<endl;
		nums = nums - members;
		tmp--;
		if(tmp==0 || nums == 0);
		else members = nums / tmp;
            	MPI_Send(numArray, members, MPI_INT, invar, TAG, MPI_COMM_WORLD); //buffer,velikost,typ,rank prijemce,tag,komunikacni skupina
//cout << invar << " s:s " << &mynumber<<endl;
            	invar++;
		
		free(numArray);
		if(!fin.good()){
			Ginvar = members;
			break;
	    	}
		cout << members <<endl;
		numArray = (int *)malloc(members*sizeof(int));
		numINarray = 0;
	    }
	    
	    //cout<<invar<<":"<<num<<endl;             //kdo dostane kere cislo
	    numArray[numINarray++] = num;
		//cout << numINarray << endl;
		
	    
            
        }//while
        fin.close();                                
    }//nacteni souboru

    //PRIJETI HODNOTY CISLA
    //vsechny procesory(vcetne mastera) prijmou hodnotu a zahlasi ji
    if(myid >= numprocs/2){
	int delitel = ceil(numprocs/2.0);
	int memory = ceil(pow(2, delitel)/(delitel/2.0));
	mynumber = (int *)malloc(memory*sizeof(int));
	MPI_Recv(mynumber, memory, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat); //buffer,velikost,typ,rank odesilatele,tag, skupina, stat
    	cout<<"i am:"<<myid<<" my number is:";
	for(int i =0; i < memory; i++){
		cout << " ," <<mynumber[i];	
	}
	cout << endl;
	free(mynumber);
    }
 
    
    
    MPI_Finalize(); 
    return 0;

 }//main

