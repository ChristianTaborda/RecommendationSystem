/*
* Autores:
* - Cristian Vallecilla
* - Christian Taborda
* - Santiago Hernandez
* 
* 
* Nombre: main.cpp
* Descripción: implementación del sistema de recomendación
* Fecha de creación: 6 / 5 / 2018
* Fecha de modificación: 24 / 5 / 2018
*/

// COMPILACION: g++ -c main.cpp
//              g++ -o exe main.o
//              ./exe

#include <iostream>
#include <time.h>
#include <cstdlib>

using namespace std;

//Transpone una matriz:
void transponer(double ** Q, int M, int K, double ** T){
	for(int x=0; x<K; x++){
		for(int y=0; y<M; y++){
			T[x][y] = Q[y][x];
		}
	}
}

//Retorna la i-ésima fila de una matriz:
void obtenerFila(double ** P, int i, int K, double * fila){
	for(int x=0; x<K; x++){
		fila[x] = P[i][x];
	}
}

//Retorna la i-ésima columna de una matriz:
void obtenerColumna(double ** Q, int j, int K, double * columna){
	for(int x=0; x<K; x++){
		columna[x] = Q[x][j];
	}
}

//Calcula el producto punto entre dos vectores:
double productoPunto(double * A, double * B, int K){
	double producto = 0;
	for(int x=0; x<K; x++){
		producto += A[x]*B[x];
	}
}

//Calcula el producto entre dos matrices:
void productoMatricial(double ** P, double ** Q, int N, int K, int M, double ** producto, double * fila, double * columna){
	for(int x=0; x<N; x++){
		for(int y=0; y<M; y++){
			obtenerFila(P,x,K,fila);
			obtenerColumna(Q,y,K,columna);
			producto[x][y] = productoPunto(fila,columna,K);
		}
	}
}

//Factoriza la matriz: (Predice los valores en cero):	
void factorizar(double ** R, double ** P, double ** Q, int K, int steps, double alfa, double beta, int N, int M){
	
	double eij, e, aux;
	
	double ** eR = new double * [N];
	for(int x=0; x<N; x++){
		eR[x] = new double[M];
	}
	
	double ** T = new double * [K];
	for(int x=0; x<K; x++){
		T[x] = new double[M];
	}
	
	double * fila = new double[K];
	double * columna = new double[K];
	
	transponer(Q,M,K,T);
	
	for(int step=0; step<steps; step++){
		for(int i=0; i<N; i++){
			for(int j=0; j<M; j++){
				if(R[i][j] > 0){
					
					obtenerFila(P,i,K,fila);
					obtenerColumna(T,j,K,columna);
					eij = R[i][j] - productoPunto(fila,columna,K);
					
					for(int k=0; k<K; k++){
						
						P[i][k] = P[i][k] + alfa*(2*eij*T[k][j] - beta*P[i][k]);
						T[k][j] = T[k][j] + alfa*(2*eij*P[i][k] - beta*T[k][j]);
						
					}
				}
			}
		}
		
		productoMatricial(P,T,N,K,M,eR,fila,columna);
		e = 0;
		
		for(int i=0; i<N; i++){
			for(int j=0; j<M; j++){
				if(R[i][j] > 0){
					
					obtenerFila(P,i,K,fila);
					obtenerColumna(T,j,K,columna);
					aux = R[i][j] - productoPunto(fila,columna,K);
					e = e + aux*aux;
					
					for(int k=0; k<K; k++){
						
						e = e + (beta/2) * (P[i][k]*P[i][k] + T[k][j]*T[k][j]);
						
					}
				}
			}
		}
		
		if(e < 0.001){
			break;
		}
		
	}
	
	transponer(T,K,M,Q);
	productoMatricial(P,T,N,K,M,R,fila,columna);
	
} 

int main(){
	
	//Semilla para el random:
	srand48(time(NULL));
	
	//Dimensiones de R (K= Factores latentes):
	int N = 5, M = 4, K = 2;
	
	//Matriz R a factorizar:	
	double ** R = new double * [N];
	for(int x=0; x<N; x++){
		R[x] = new double[M];
	}
	
	R[0][0] = 5;
	R[0][1] = 3;
	R[0][2] = 0;
	R[0][3] = 1;
	R[1][0] = 4;
	R[1][1] = 0;
	R[1][2] = 0;
	R[1][3] = 1;
	R[2][0] = 1;
	R[2][1] = 1;
	R[2][2] = 0;
	R[2][3] = 5;
	R[3][0] = 1;
	R[3][1] = 0;
	R[3][2] = 0;
	R[3][3] = 4;
	R[4][0] = 0;
	R[4][1] = 1;
	R[4][2] = 5;
	R[4][3] = 4;
	
	//Imprimir matriz R:
	cout << endl;
	cout << "Matriz R: " << endl;
	for(int x=0; x<N; x++){
		for(int y=0; y<M; y++){
			cout << R[x][y] << " ";
		}
		cout << endl;
	}
	
	//Matriz P (random entre 0 y 1) 
	double ** P = new double * [N];
	for(int x=0; x<N; x++){
		P[x] = new double[K];
	}
	for(int x=0; x<N; x++){
		for(int y=0; y<K; y++){
			P[x][y] = drand48() * 1;
		}
	}
	
	//Imprimir matriz P:
	cout << endl;
	cout << "Matriz P: " << endl;
	for(int x=0; x<N; x++){
		for(int y=0; y<K; y++){
			cout << P[x][y] << " ";
		}
		cout << endl;
	}
	
	//Matriz Q (random entre 0 y 1)
	double ** Q = new double * [M];
	for(int x=0; x<M; x++){
		Q[x] = new double[K];
	}
	for(int x=0; x<M; x++){
		for(int y=0; y<K; y++){
			Q[x][y] = drand48() * 1;
		}
	}
	
	//Imprimir matriz Q:
	cout << endl;
	cout << "Matriz Q: " << endl;
	for(int x=0; x<M; x++){
		for(int y=0; y<K; y++){
			cout << Q[x][y] << " ";
		}
		cout << endl;
	}
	
	//Factorizar:
	factorizar(R,P,Q,K,5000,0.0002,0.02,N,M);
	
	//Imprimir matriz P nueva:
	cout << endl;
	cout << "Matriz Nueva P: " << endl;
	for(int x=0; x<N; x++){
		for(int y=0; y<K; y++){
			cout << P[x][y] << " ";
		}
		cout << endl;
	}
	
	//Imprimir matriz Q nueva:
	cout << endl;
	cout << "Matriz Nueva Q: " << endl;
	for(int x=0; x<M; x++){
		for(int y=0; y<K; y++){
			cout << Q[x][y] << " ";
		}
		cout << endl;
	}
	
	//Imprimir matriz R nueva:
	cout << endl;
	cout << "Matriz Nueva R: " << endl;
	for(int x=0; x<N; x++){
		for(int y=0; y<M; y++){
			cout << R[x][y] << " ";
		}
		cout << endl;
	}
	
	return 0;
	
}
