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

// COMPILACION: g++ main.cpp -lpqxx -lpq
// ES NECESARIA LA LIBRERÍA DE LBPQXX PARA CONECTAR PROGRAMAS EN C++ Y BASES DE DATOS POSTGRES

#include <iostream>
#include <pqxx/pqxx> 
#include <string> 
#include <time.h>
#include <cstdlib>
#include <algorithm>
#include <vector>

using namespace std;
using namespace pqxx;

struct bookRating { 
    string name;
    double rating;
};

struct by_rating { 
    bool operator()(bookRating const &a, bookRating const &b) { 
        return a.rating < b.rating;
    }
};

// Transpone una matriz:
void transponer(double ** Q, int M, int K, double ** T){
	for(int x=0; x<K; x++){
		for(int y=0; y<M; y++){
			T[x][y] = Q[y][x];
		}
	}
}

// Retorna la i-ésima fila de una matriz:
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

int main(int argc, char* argv[]) {
	
   // Arreglo para guardar IDs de los usuarios. -->1101 usuarios;
	double* users;
	
	users = 0;
	users = new double[1101];
	for(int i = 0; i<1101; i++){
		users[i] = 0;
	}
	
	char * sql;
   
	try {
		
		connection C("dbname = tenkuryu user = tenkuryu password = shinkansen7 \
		hostaddr = 127.0.0.1 port = 5432");
		if (C.is_open()) {
			cout << "Opened database successfully: " << C.dbname() << endl;
		} else {
			cout << "Can't open database" << endl;
			return 1;
		}

		/* Create SQL statement */
		//sql = "SELECT * from electrodomestico";
		sql = "SELECT userid FROM finalratings group by userid HAVING COUNT (USERID)>0 ORDER BY userid;";
		
		/* Create a non-transactional object. */
		nontransaction N(C);
		
		/* Execute SQL query */
		result R( N.exec( sql ));
		
		/* List down all the records */
		int counter = 0;
		for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
			double userid = 0;
			userid = c[0].as<double>();
			users[counter] = userid;
			counter++;
		}
		cout << "Generate users array: success" << endl;
		C.disconnect ();
	} catch (const std::exception &e) {
		cerr << e.what() << std::endl;
		return 1;
	}
	/*
	for (int i=0; i<1101; i++) {
		cout << users[i] << endl;
	}*/
	
	// Arreglo para guardar ISBNs de los libros. --> 264 libros
	string* books;
	
	books = 0;
	books = new string[264];
	for(int i = 0; i<264; i++){
		books[i] = "";
	}
	
	try {
		
		connection C("dbname = tenkuryu user = tenkuryu password = shinkansen7 \
		hostaddr = 127.0.0.1 port = 5432");
		if (C.is_open()) {
			cout << "Opened database successfully: " << C.dbname() << endl;
		} else {
			cout << "Can't open database" << endl;
			return 1;
		}

		/* Create SQL statement */
		//sql = "SELECT * from electrodomestico";
		sql = "SELECT isbn FROM finalratings group by isbn HAVING COUNT (isbn)>0 ORDER BY isbn;";
		
		/* Create a non-transactional object. */
		nontransaction N(C);
		
		/* Execute SQL query */
		result R( N.exec( sql ));
		
		/* List down all the records */
		int counter = 0;
		for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
			string isbn = "";
			isbn = c[0].as<string>();
			books[counter] = isbn;
			counter++;
		}
		cout << "Generate ISBNs array: success" << endl;
		C.disconnect ();
	} catch (const std::exception &e) {
		cerr << e.what() << std::endl;
		return 1;
	}
	
	/*
	for (int i=0; i<264; i++) {
		cout << books[i] << endl;
	}*/
	
	// Arreglo para guardar ratings, filas->usuarios, columnas->libros. --> 1101*264
	double** ratings;
	
	ratings = 0;
	ratings = new double*[1101];
	for(int a = 0; a<1101; a++){
		ratings[a] = new double[264];
	}
	for(int a = 0; a<1101; a++){
		for(int b = 0; b<264; b++){
			ratings[a][b] = 0;
		}
	}
	
	// Arreglo para guardar si los ratings estaban desde el principio, filas->usuarios, columnas->libros.
	// 1->Estaba desde el principio, 0->No estaba (es decir, este valor será predicho).
	double** auxiliar;
	
	auxiliar = 0;
	auxiliar = new double*[1101];
	for(int a = 0; a<1101; a++){
		auxiliar[a] = new double[264];
	}
	for(int a = 0; a<1101; a++){
		for(int b = 0; b<264; b++){
			auxiliar[a][b] = 0;
		}
	}
	
	try {
		
		connection C("dbname = tenkuryu user = tenkuryu password = shinkansen7 \
		hostaddr = 127.0.0.1 port = 5432");
		if (C.is_open()) {
			cout << "Opened database successfully: " << C.dbname() << endl;
		} else {
			cout << "Can't open database" << endl;
			return 1;
		}
		
		for(int a = 0; a<1101; a++)
		{
			for(int b = 0; b<264; b++){
				int userDouble = users[a];
				string userString = to_string(userDouble);
				string sqlRatings = "SELECT bookrating FROM finalratings WHERE userid=";
				sqlRatings += userString;
				sqlRatings += " and isbn='";
				sqlRatings += books[b];
				sqlRatings += "';";
				/*sql = "SELECT bookrating FROM finalratings WHERE userid='" + userString +"' and isbn='" + books[b] +"';";*/
				nontransaction N(C);
				result R( N.exec( sqlRatings ));
				
				if(R.empty()){
					ratings[a][b] = 0;
					auxiliar[a][b] = 0;
				}else{
					for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
						double rating = c[0].as<double>();
						
						ratings[a][b] = rating;
						auxiliar[a][b] = 1;
					}
				}
				
			}
		}
		
		cout << "Generate original ratings matrix: success" << endl;
		C.disconnect ();
	} catch (const std::exception &e) {
		cerr << e.what() << std::endl;
		return 1;
	}
	
	cout << endl;
	cout << "Matriz original R: " << endl;
	for(int x=0; x<8; x++){
		for(int y=0; y<8; y++){
			cout << ratings[x][y] << " ";
		}
		cout << endl;
	}
	
	cout << "Initiating factorization routine..." << endl;
	
	//Semilla para el random:
	srand48(time(NULL));
	
	//Dimensiones de R:
	int N = 1101 /*usuarios*/, M = 264 /*isbns*/, K = 5;
	
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
	
	// Factorizar:
	factorizar(ratings,P,Q,K,5000,0.0002,0.02,N,M);
	
	cout << "Factorization: success" << endl;
	
	cout << endl;
	cout << "Matriz Nueva R: " << endl;
	for(int x=0; x<8; x++){
		for(int y=0; y<8; y++){
			cout << ratings[x][y] << " ";
		}
		cout << endl;
	}
	
	// ONGOING
	// Proceso para seleccionar mejores ratings predichos
	cout << "Trying recommendation" << endl;
	
	try {
		
		vector<bookRating> bookRatingsVector;
		
		int userIndex = 333;
		int numRecommendations = 5;
		// sort(ratings[0], ratings[0]+SIZE);
		
		connection C("dbname = tenkuryu user = tenkuryu password = shinkansen7 \
		hostaddr = 127.0.0.1 port = 5432");
		if (C.is_open()) {
			cout << "Opened database successfully: " << C.dbname() << endl;
		} else {
			cout << "Can't open database" << endl;
			return 1;
		}
		
		for(int isbnIndex = 0; isbnIndex<264; isbnIndex++)
		{
			int newValue = auxiliar[userIndex][isbnIndex];
			
			if(!newValue){
				string bookTitle="";
				string sqlBookTitle = "SELECT booktitle FROM bxbooks WHERE isbn='";
				sqlBookTitle += books[isbnIndex];
				sqlBookTitle += "';";
				
				nontransaction N(C);
				result R( N.exec( sqlBookTitle ));
				
				for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
					bookTitle += c[0].as<string>();
				}
				
				bookRating tempBookRating={};
				tempBookRating.name = bookTitle;
				tempBookRating.rating = ratings[userIndex][isbnIndex];
				
				bookRatingsVector.push_back(tempBookRating);
			}
			
		}
		
		sort(bookRatingsVector.begin(), bookRatingsVector.end(), by_rating());
			
		double userID = users[userIndex];
		cout << "Recommendations for user: " << userID << endl;
		cout << "----------------------------------" << endl;
		for(int i=bookRatingsVector.size(); i>bookRatingsVector.size()-numRecommendations; i--){
			string bookTitle = bookRatingsVector.at(i-1).name;
			double rating = bookRatingsVector.at(i-1).rating;
			cout << bookTitle << " - Predicted score: " << rating << endl;
		}
		
		cout << "Recommend: success" << endl;
		C.disconnect ();
	} catch (const std::exception &e) {
		cerr << e.what() << std::endl;
		return 1;
	}
	
	return 0;
}
