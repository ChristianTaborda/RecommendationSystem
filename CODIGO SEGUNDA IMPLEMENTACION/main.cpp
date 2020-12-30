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
#include <math.h>

using namespace std;
using namespace pqxx;

// Estructura para representar un libro con la puntuacion que le dio un usuario y su titulo
struct bookRating { 
    string name;
    double rating;
};

struct by_rating { 
    bool operator()(bookRating const &a, bookRating const &b) { 
        return a.rating < b.rating;
    }
};

// Suma de vectores
double suma(vector<double> a)
{
	double s = 0;
	for (int i = 0; i < a.size(); i++)
	{
		s = s + a[i];
	}
	return s;
}

double media(vector<double> a)
{
	return suma(a) / a.size(); 
}

double sqsum(vector<double> a)
{
	double s = 0;
	for (int i = 0; i < a.size(); i++)
	{
		s = s + pow(a[i], 2);
	}
	return s;
}

double stdev(vector<double> nums)
{
	double N = nums.size();
	return pow(sqsum(nums) / N - pow(suma(nums) / N, 2), 0.5);
}

vector<double> operator-(vector<double> a, double b)
{
	vector<double> retvect;
	for (int i = 0; i < a.size(); i++)
	{
		retvect.push_back(a[i] - b);
	}
	return retvect;
}

vector<double> operator*(vector<double> a, vector<double> b)
{
	vector<double> retvect;
	for (int i = 0; i < a.size() ; i++)
	{
		retvect.push_back(a[i] * b[i]);
	}
	return retvect;
}

double pearsoncoeff(vector<double> X, vector<double> Y)
{
	return suma((X - media(X))*(Y - media(Y))) / (X.size()*stdev(X)* stdev(Y));
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
	
	//Dimensiones de R:
	int N = 1101 /*usuarios*/, M = 264 /*isbns*/;
	
	//Matriz de similitud
	double ** S = new double * [M];
	for(int x=0; x<M; x++){
		S[x] = new double[M];
	}
	
	// ONGOING
	// Proceso para seleccionar mejores ratings predichos
	cout << "Trying recommendation" << endl;
	
	//Se realiza la correlacion de las columnas(todas con todas) para hayar las similitudes
	vector<double> X(N);
	vector<double> Y(N);
	
	for(int j=0; j<M; j++){
		for (int i = 0; i < X.size() ; i++){
			X[i]=ratings[i][j]; //Columna a comparar 
		}
		for(int k=0; k<M; k++){
			for (int i = 0; i < Y.size() ; i++){
				Y[i]=ratings[i][k]; //demas columnas a comparar
			}
			S[j][k]=pearsoncoeff(X, Y); //Se llena la matriz de coeficientes de Person(similitudes
		}
		
	}
	
	//ya se va a realizar la recomendacion
	vector<double> P(M);
	vector<bookRating> bookRatingsVector;
	
	int userIndex = 888;  //Posicion del ususario
	for (int i = 0; i < M ; i++){
		P[i]=ratings[userIndex][i];//PUNTUACIONES USUARIO
	}
	
	try {
		
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
		
		for(int i=0; i<M; i++){
			for(int j=0; j<M; j++){
				if(isnan(S[i][j]) || S[i][j]<0){
					S[i][j]=0;
				}else{
					double value = S[i][j]*P[i];
					S[i][j]= value;
				}
			}
		}
		
		for(int i=0; i<M; i++){
			double puntajeAcumulado=0;
			int contadorDenominador=0;
			double puntaje;
			
			for(int j=0; j<M; j++){
				puntaje = S[j][i];
				if(!puntaje==0){
					contadorDenominador++;
					puntajeAcumulado+=puntaje;
				}
			}
			
			if(contadorDenominador!=0){
				puntajeAcumulado=puntajeAcumulado/contadorDenominador;
			}
			
			int newValue = auxiliar[userIndex][i];
			if(!newValue){
				string bookTitle="";
				string sqlBookTitle = "SELECT booktitle FROM bxbooks WHERE isbn='";
				sqlBookTitle += books[i];
				sqlBookTitle += "';";
				
				nontransaction N(C);
				result R( N.exec( sqlBookTitle ));
				
				for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
					bookTitle += c[0].as<string>();
				}
				
				bookRating tempBookRating={};
				tempBookRating.name = bookTitle;
				tempBookRating.rating = puntajeAcumulado;
				
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
