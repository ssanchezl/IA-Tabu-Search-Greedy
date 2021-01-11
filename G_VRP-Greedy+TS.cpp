#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <string>
#include <vector>
#include <iomanip>
#include <math.h>
#include <fstream>
#include <algorithm>


using namespace std; 

#define _USE_MATH_DEFINES


struct Instancia
{  
    string id;  
    string type;  
    double Lon;
    double Lat;  
};

struct Distancia
{  
    string id1;
    string id2;
    double dist;
};


//------------------------------------------------------------------//
//--- FUNCION: SEPARAR LINEA ---------------------------------------//
//------------------------------------------------------------------//
//--- RECIBE una linea en forma de string, y -----------------------//
//--- la direccion de memoria de un arreglo para guardar por -------//
//--- separado los elementos presentes en el string ----------------//
//------------------------------------------------------------------//
//--- CAMBIA el valor del arreglo rellenandolo con los datos -------//
//------------------------------------------------------------------//
void splitLinea(string s, string (&r)[4]) { 
    
    int elem = 0;    
    for (int i = 0; i < s.size(); i++) {  

        if (i>0) {
            if ((isspace(s[i]) == 0) && (isspace(s[i-1]) != 0)) {
                elem++;                
            }
        }    
        if (isspace(s[i]) == 0) {
                  
            r[elem] = r[elem] + s[i];                                   
        } 
    }         
}




//------------------------------------------------------------------//
//--- FUNCION: LEER ARCHIVO ----------------------------------------//
//------------------------------------------------------------------//
//--- RECIBE el nombre del archivo entregado -----------------------//
//--- como parametro en la ejecucion del programa, un vector -------//
//--- inicializado vacio y un arreglo estatico de parametros -------//
//------------------------------------------------------------------//
//--- CAMBIA el valor del vector rellenandolo con instancias -------//
//------------------------------------------------------------------//
void Read(int argc, char **argv, vector<Instancia> &inst, float *Params) {

    // Abre el archivo especificado
    fstream myfile;
    myfile.open(argv[1], ios::in);

    
    int notvacio = 1;
    int linea = 0;
    int pos = 0;    

    // Lee linea por linea
    string line;    
    if (myfile)
    {
        while (getline( myfile, line ))  
        {
            if (linea == 0) {
                linea++;
                continue;   
            }

            // Salto de linea
            if (isspace(line[0])) {                
                notvacio = 0;         
            }               

            if (notvacio){
                // Lee primeras lineas con instancias
                string arr[4];

                // Separa la linea y guarda en arreglo
                splitLinea(line, arr);   
                
                if (linea != 2)
                {     
                    
                    // Guarda elemento en estructura: id, type, Longitude, Latitude
                    inst.push_back(Instancia{arr[0], arr[1], stod(arr[2]), stod(arr[3])});
                }                        
            } 
            
            else {    

                // Lee las ultimas lineas (parametros)
                int slash = 0;                        
                string prenum = "";
                for (int i = 0; i < line.size(); i++)
                {                   
                    if (i > 0 && i < line.size()){
                        if (line[i] == '/' && isspace(line[i-1]) != 0) {
                            slash = 1;
                            continue;   
                        }
                    }
                    if (slash) {

                        if (line[i] == '/')
                        {
                            Params[pos] = stof(prenum);
                            slash = 0;
                            pos++;                        
                        }
                        // Guarda los parametros
                        prenum = prenum + line[i];
                    }                                
                }                             
            }                            
            linea++;
        }
        myfile.close();
    }    
}




//------------------------------------------------------------------//
//--- FUNCION: OBTENER DISTANCIAS ----------------------------------//
//------------------------------------------------------------------//
//--- RECIBE coordenadas de 2 puntos -------------------------------//
//------------------------------------------------------------------//
//--- DEVUELVE: la distancia de harvesine entre ambos --------------//
//------------------------------------------------------------------//
double Harvesine(Instancia p1, Instancia p2){
    const static double radiusOfEarth = 4182.44949; // miles, 6371km (más exacto con millas)
    
    // Convertir coordenadas a radianes
    double lat1 = p1.Lat / 57.29578;
    double lat2 = p2.Lat / 57.29578;
    double lon1 = p1.Lon / 57.29578;
    double lon2 = p2.Lon / 57.29578;

    // Calcular deltas
    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;                

    // Formula de Harvesine
    double a = sin(dLat/2) * sin(dLat/2) + cos(lat1) *cos(lat2) * sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));       
    double distance = radiusOfEarth * c;
                     
    return distance;                 
}




//------------------------------------------------------------------//
//-- FUNCION: Obtener nodo mas cercano -----------------------------//
//------------------------------------------------------------------//
//-- RECIBE: las distancias, las instancias y el i-esimo nodo ------//
//------------------------------------------------------------------//
//-- DEVUELVE: los indices del node mas cercano al que se pide -----//
//------------------------------------------------------------------//
int min_dist_to_node(vector<Instancia> inst, int i, vector<int> &out){
	double min = 32767;
	int index;
    double dist;     
				
	for (int j = 0; j < inst.size(); j++)
    {
        if ((inst[i]).id != (inst[j]).id && find(out.begin(), out.end(), j) == out.end()){
            dist = Harvesine(inst[i], inst[j]);            
            if (dist < min) 
            {
                min = dist;
                index = j;	            		            		
            }
        }
    }	 
    out.push_back(index);    
	return index;
}



//------------------------------------------------------------------//
//-- FUNCION: Chequear si una solucion es factible -----------------//
//------------------------------------------------------------------//
//-- RECIBE: solucion y los parametros de la instancia -------------//
//------------------------------------------------------------------//
//-- DEVUELVE: true si es factible, false si no lo es --------------//
//------------------------------------------------------------------//
//-- IMPORTANTE!!! Esta funcion fue obtenida de otro proyecto 2017 -//
//-- La diferencia es que en este caso debido a la representacion --//
//-- delos datos, la funcion cambia la forma de indexar y acceder --//
//-- Tambien se agrega restriccion de salida desde nodo tipo d o f -//
//------------------------------------------------------------------//
// --Fuente: https://github.com/fchacon20/GVRP_IA/blob/master/IaProject/solutionUtilities.cpp
//------------------------------------------------------------------//
bool sol_valida(vector<Instancia> sol, float *p){
    
    //Parametros    
    /////////////////////////////////
    float Q  = p[0];               //
    float r  = p[1];               //
    float TL = p[2] * 60;          //
    float v  = p[3] / 60;          //
    float m  = p[4];               //
    /////////////////////////////////
    
    int t = 0;    
    for (int i = 1; i < sol.size(); ++i) 
    {
        // Revisar que el mismo nodo no se repita de forma contigua
        if((sol[i-1]).id == (sol[i]).id) 
            return false; 

        
        // Revisar que si el actual no es cliente, el anterior lo sea
        if ((sol[i]).type != "c" && (sol[i-1]).type != "c")
            return false;
        

        Q -= Harvesine(sol[i-1], sol[i])*r;        
        t += Harvesine(sol[i-1], sol[i])/v;  
        
        // Revisar si queda combustible
        if (Q < 0) {            
            return false;
        }
        if ((sol[i]).type == "f") {
            Q = p[0];
            t += 15;
        } else if ((sol[i]).type == "c") {
            t += 30;
        }else{
            Q = p[0];
            t = 0;
        }
        // Revisar si queda tiempo de recorrido
        if(t >= TL) {            
            return false;
        }        
    }
    return true;
} 




//------------------------------------------------------------------//
//-- FUNCION: Revisar si por alguna razon la solucion es infactible-//
//------------------------------------------------------------------//
//-- RECIBE: vector de solucion final ------------------------------//
//------------------------------------------------------------------//
//-- DEVUELVE: solucion final con el depot final donde corresponde -//
//------------------------------------------------------------------//
void revisionFinal(vector<Instancia> &sol)
{
    for (int i = 1; i < sol.size(); i++)
    {            
        if ((sol[i]).type == "d" && (sol[i-1]).type == "d")
        {                        
            sol.erase(sol.begin()+i);
        }
        if ((sol[sol.size()-1]).type != "d")
        {
            sol.push_back(sol[0]);
        }        
    }
}




//------------------------------------------------------------------//
//-- FUNCION: Aplicar las restricciones a la solucion --------------//
//------------------------------------------------------------------//
//-- RECIBE distancia, solucion, combustible y parametros ----------//
//------------------------------------------------------------------//
//-- DEVUELVE: el vector solucion restringido ----------------------//
//------------------------------------------------------------------//
void restringir(vector<Instancia> &sol, vector<Instancia> fuel, float *params){
	
	//Parametros	
	////////////////////////////////
	float Q = params[0];	      //
	float r = params[1];	      //
	float TL = params[2] * 60;    //
	float v = params[3] / 60;     //
	float m = params[4];          //
	////////////////////////////////

	double dist;
	double min_dist;	
	int index;
	vector<Distancia> min_fuel_dist;			

	// Obtener distancia de cada cliente a su estacion de servicio mas cercana
	for (int i = 0; i < sol.size(); i++)
	{															
		min_dist = 32767;
		for (int j = 0; j < fuel.size(); j++)
		{			                
			dist = Harvesine(sol[i], fuel[j]);
			if (dist < min_dist && i!=j)
			{
				min_dist = dist;
				index = j;
			}													
		}				
		min_fuel_dist.push_back(Distancia{(sol[i]).id, (fuel[index]).id, min_dist});				
	}


	// Inicio del recorrido 
	bool nodo_ok = false;
	vector<Instancia> comp_dist;
	comp_dist.push_back(sol[0]);	

    vector<int> sol_index;
    sol_index.push_back(0);

	int ultimo = comp_dist.size()-1;	
	float t = 0;    
	double actualQ = 0;	
	vector<string> ids;

	int idx;    

    double dist_i;
    double dist_d;
    int vuelta = 1;
    double nodo_cercano = 32767;

    

	while (ids.size() != sol.size() - fuel.size() - 1) {
			        
        nodo_ok = false;
        nodo_cercano = 32767;
        idx = 0;
		for (int i = 1; i < sol.size(); i++)
		{	
			// Distancia entre nodo actual y ultimo visitado
			dist_i = Harvesine(comp_dist[ultimo], sol[i]);

			// Distancia entre nodo actual y deposito
			dist_d = Harvesine(sol[i], comp_dist[0]);	

            // Tiempo de viaje
            t += dist_i/v + min(dist_d/v, (min_fuel_dist[i]).dist/v);

            // Revisar que el anterior no sea el mismo y que si es cliente no se repita
			if ((comp_dist[ultimo]).id != (sol[i]).id && find(ids.begin(), ids.end(), (sol[i]).id) == ids.end() )
			{					
				// Si queda f para volver al depósito o si queda f para llegar a AFS o (si es AFS o d y el anterior no lo es)
				if (dist_d*r + actualQ <= Q || (min_fuel_dist[i]).dist*r + actualQ <= Q || (sol[i]).type !="c" )
				{		                    	                   
					// Verificar si alcanza el tiempo para visitar un cliente o un AFS
					if (((sol[i]).type == "c" && (t + 30)<=TL) || ((sol[i]).type == "f" && (t +15)<=TL)) 
					{	
                        // El nodo siguiente es el mas cercano
						if(dist_i < nodo_cercano)
						{	
                            // Si el nodo anterior no es cliente el actual debe serlo                                    
							if (!((sol[i]).type != "c" && (comp_dist[ultimo]).type != "c"))
							{		                                
                                nodo_cercano = dist_i;
                                nodo_ok = true;                              
                                idx = i;
							}	
                                                         
						}
					}
				}
			}	
            t += - dist_i/v - min(dist_d/v, (min_fuel_dist[i]).dist/v);		
		}          

		// Una vez cumplidas las restricciones, se procede a agregar        

        // Si es cliente no se repita
        if (find(ids.begin(), ids.end(), (sol[idx]).id) == ids.end())
        {
            // Si alcanza el f 
            if(nodo_cercano*r + actualQ < Q && nodo_ok)
            {                   
                actualQ += nodo_cercano*r;
                t += nodo_cercano/v;

                
                // Se agrega el nodo a la solucion
                comp_dist.push_back(sol[idx]);                    

                ultimo = comp_dist.size()-1;                          
                                    
                if ((comp_dist[ultimo]).type == "c")
                {                           
                    ids.push_back((sol[idx]).id);   
                    t += 30;
                }
                else if ((comp_dist[ultimo]).type == "f")
                {
                    t += 15;
                    actualQ = 0;
                }
                else
                {
                    t = 0;
                    actualQ = 0;
                }           
            }
            else
            {                         
                if((comp_dist[ultimo]).type != "d")
                {                
                    comp_dist.push_back(sol[0]);                 
                    actualQ = 0;
                    t = 0;
                }                              
            }
        }
			                                      
        vuelta++;                
	}	    
	sol = comp_dist; 
    sol.push_back(sol[0]);
}




//------------------------------------------------------------------//
//-- FUNCION: Revisar si un par de soluciones son iguales ----------//
//------------------------------------------------------------------//
//-- RECIBE: 2 vectores de instancias ------------------------------//
//------------------------------------------------------------------//
//-- DEVUELVE: la solucion inicial candidata en forma de instancias //
//------------------------------------------------------------------//
bool misma_solucion( vector<Instancia>sol_A, vector <Instancia>sol_B)
{   
    // Revisar si alguno esta vacio
    if((sol_A.size() == 0 || sol_B.size()== 0) || (sol_A.size() != sol_B.size()))
    {
        return false;
    }    
    // Se asume que ambos son del mismo tamaño
    for(int i=0; i < sol_A.size(); i++)
    {
        if( (sol_A[i]).id != (sol_B[i]).id)
        {
            return false;
        }        
    }
    return true;
}




//------------------------------------------------------------------//
//-- FUNCION: Obtener solución inicial -----------------------------//
//------------------------------------------------------------------//
//-- RECIBE: instancias, distancias, combustibles y parametros -----//
//------------------------------------------------------------------//
//-- DEVUELVE: la solucion inicial candidata en forma de instancias //
//------------------------------------------------------------------//
void solucionInicial(vector<Instancia> &inst, vector<Distancia> &distancia, vector<Instancia> &fuel, vector<Instancia> &sol, float *Parametros){
    double dist;
    int index;

    vector<int> sol_index;
    sol_index.push_back(0);

    // Primero el deposito
    sol.push_back(inst[0]);

    for(int i=0; i < inst.size(); i++) 
    { 
    	// Obtener nodos de combustible
        if ((inst[i]).type == "f" || (inst[i]).type == "d") fuel.push_back(inst[i]);        
 		
 		// Calcular distancias
        for(int j=i+1; j < inst.size(); j++) 
        {
            dist = Harvesine(inst[i], inst[j]);
            distancia.push_back(Distancia{(inst[i]).id, (inst[j]).id, dist});
        }    
    }         

    // Agregar a la solucion inicial el swaped mas cercano
    for (int i = 0; i < inst.size()-1; i++)
    {        
        index = min_dist_to_node(inst, i, sol_index);        
        sol.push_back(inst[index]);
    }          
    

    //limpiar el vector instancia, distancia y sol_index, ya no es necesario
    inst.clear();    
    distancia.clear();
    sol_index.clear();
    restringir(sol, fuel, Parametros);    
}




//------------------------------------------------------------------//
//-- FUNCION: Obtener distancia recorrida --------------------------//
//------------------------------------------------------------------//
//-- RECIBE: solucion y la distancia a modificar -------------------//
//------------------------------------------------------------------//
//-- DEVUELVE: la distancia total del recorrido --------------------//
//------------------------------------------------------------------//
double distanciaRecorrida(vector<Instancia> sol)
{    
    double dist_recor = 0;
    for (int i = 0; i < sol.size()-1; ++i)
    {
        dist_recor += Harvesine(sol[i], sol[i+1]);
    }    
    return dist_recor;
}


vector<Instancia> manual_swap(vector<Instancia> sol, int i, int j)
{
    vector<Instancia> aux;
    for (int k = 0; k < sol.size(); k++)
    {
        if (k == i)
        {
            aux.push_back(sol[j]);
        }
        else if (k == j)
        {
            aux.push_back(sol[i]);
        }
        else
        {
            aux.push_back(sol[k]);
        }
    }
    return aux;
}




//------------------------------------------------------------------//
//-- FUNCION: encontrar un optimo local dentro de las iteraciones --//
//------------------------------------------------------------------//
//-- RECIBE: solucion inicial, n iteraciones, tamanio lista tabu y -//
//-- parametros. ---------------------------------------------------//
//------------------------------------------------------------------//
//-- DEVUELVE: la mejor solucion con TabuSearch --------------------//
//------------------------------------------------------------------//
vector <Instancia> TabuSearch(vector <Instancia> sol, int iter, float *p, int tamanio_lista)
{    
    // PARAMETROS    
    ////////////////////////////////////////////////
    float Q = p[0];                               //
    float r = p[1];                               //
    float TL = p[2] * 60;                         //
    float v = p[3] / 60;                          //
    float m = p[4];                               //
    ////////////////////////////////////////////////

    // LISTA TABU que contiene los tamanio_lista ultimos movimientos
    ////////////////////////////////////////////////    
    vector<vector <Instancia>> Tabu;              //
    ////////////////////////////////////////////////


    // VARIABLES ACTUALES
    ////////////////////////////////////////////////
    // Solucion que se convierte en los swap tour //
    vector <Instancia> current_sol;               //
    // Distancia del tour swaped                  //
    double current_dist_swap;                     //
    // Distancia de la solucion actual            //
    double current_dist = distanciaRecorrida(sol);// 
    ////////////////////////////////////////////////

    
    // VARIABLES DE MEJORA
    //////////////////////////////////////////////// 
    // Mejor solucion hasta el momento            //
    vector <Instancia> best_sol;                  //
    // Distancia para auxiliar para comparar      //
    double comp_dist = 32767;                     //
    // Distancia mas corta hasta el momento       //
    double best_dist = distanciaRecorrida(sol);   //
    // Peor solucion encontrada entre las mejores //
    vector <Instancia> worst_sol;                 //    
    // Mejor iteracion hasta el momento           //
    int best_iter;                                //
    ////////////////////////////////////////////////
    

    bool flag;
        
    for(int it = 0; it < iter; it++)
    {
        // Si se agrega un swaped, entonces vale 1
        int agregar_a_Tabu = 0; 
        // Si la solucion ya esta en la lista, entonces vale 1        
        int in_list;   

        for(int i=0; i< sol.size(); i++)
        {
            for (int j=0; j< sol.size(); j++)
            {
                // Flag de control de flujo
                flag = true;
                // Solucion con swap
                vector<Instancia> swaped;

                if(i!=j)
                {
                    // Se hace swap entre dos nodos
                    swaped = manual_swap(sol, i, j);
                    // Calcular distancia del swaped
                    current_dist_swap = distanciaRecorrida(swaped); 
                    in_list = 0;

                    
                    for(int move_it = 0; move_it< Tabu.size(); move_it++)
                    {
                        // Revisar si el movimiento swap ya estaba en la lista
                        if(misma_solucion(Tabu[move_it], swaped))
                        {
                            // Si la solucion ya esta en la lista, entonces vale 1
                            in_list = 1;
                            break;
                        }                        
                    } 

                    // Si la solucion es mejor que la anterior anterior y ademas es la mejor
                    if(current_dist_swap < current_dist && current_dist_swap < best_dist && in_list == 0)
                    {                        
                        // Si se llena la lista tabu, se saca swaped agregado al principio.
                        if(Tabu.size()==tamanio_lista)
                        {
                            Tabu.erase(Tabu.begin());
                        }
                        Tabu.push_back(swaped);
                        best_dist = current_dist_swap;                                 
                        if(sol_valida(swaped, p) == false)
                        {
                            best_dist += 10000;                                
                        }                                                    
                        best_sol = swaped;
                        agregar_a_Tabu = 1;
                        best_iter = iter;                                                                                                    
                    }

                    // Si la solucion es mejor que la anterior anterior
                    else if(current_dist_swap < current_dist && in_list ==0)
                    {
                        // Si se llena la lista tabu, se saca swaped agregado al principio.
                        if(Tabu.size()==tamanio_lista)
                        {
                            Tabu.erase(Tabu.begin());
                        }
                        Tabu.push_back(swaped);
                        current_dist = current_dist_swap;
                        if(sol_valida(swaped, p) == false)
                        {
                            current_dist += 10000;
                        }                                                   
                        current_sol = swaped;
                        agregar_a_Tabu = 1;                                                
                    }                     
                    
                    // Si no hay un swaped mejor que la solucion actual
                    else
                    {
                        if(in_list == 0)
                        {
                            // Si se llena la lista tabu, se saca swaped agregado al principio.
                            if(Tabu.size() == tamanio_lista)
                            {
                                if(comp_dist > distanciaRecorrida(swaped))
                                {                                                                        
                                    Tabu.erase(Tabu.begin());
                                    Tabu.push_back(swaped);                                    
                                }
                            }
                            else
                            {
                                if(comp_dist > distanciaRecorrida(swaped))
                                {                                                                        
                                    if(worst_sol.size() != 0 && swaped.size() != 0)
                                    {
                                        flag = false;
                                    }                              
                                }  
                                else
                                {
                                    Tabu.push_back(swaped);
                                }                                                                                                                                 
                            }
                            if (flag)
                            {    
                                comp_dist = distanciaRecorrida(swaped);                            
                                if (sol_valida(swaped, p) == false)
                                {
                                    comp_dist += 10000;
                                }                                
                                worst_sol = swaped;                                 
                            }                                                                                                                                                                                                                                                          
                        }
                    }
                }                                                
            }                
        }
        
        if( agregar_a_Tabu == 1)
        {
            sol = current_sol;
        }
        else
        {
            current_dist = comp_dist;
            current_sol = worst_sol;
        }
    }
    if(best_dist <= current_dist)
    {
        sol = best_sol;        
    }       
    cout << "\n" << "Mejor solucion encontrada en la iteracion: " << best_iter << endl; 
    return sol;   
}




//------------------------------------------------------------------//
//------------------------------------------------------------------//
//------------------------ FUNCION PRINCIPAL -----------------------//
//------------------------------------------------------------------//
//------------------------------------------------------------------//    
int main(int argc, char **argv)
{
    //--------------------------------------------------------------//
    //---------------- LEER ARCHIVO DE INSTANCIAS ------------------//
    //----- Lee linea por linea y las guarda como instancias -------//
    //----- en un vector de instancias. ----------------------------//
    //----- Los parametros son guardados en el vector parametros ---//
    //--------------------------------------------------------------//
    vector<Instancia> inst;    
    float params[5];    
    Read(argc, argv, inst, params);   

    //Parametros    
    ////////////////////////////////
    float Q = params[0];          //
    float r = params[1];          //
    float TL = params[2] * 60;    //
    float v = params[3] / 60;     //
    float m = params[4];          //
    //////////////////////////////// 

    int n_iter = atoi(argv[2]);
	//--------------------------------------------------------------//        




    //--------------------------------------------------------------//
    //--------------------- CALCULAR SOLUCION ----------------------//
    //----- Calcular una solucion inicial greedy luego -------------//
    //----- revisar si es factible ---------------------------------//
    //--------------------------------------------------------------//    
    vector<Instancia> fuel;    
    vector<Instancia> sol;
    vector<Distancia> distancias;        
    solucionInicial(inst, distancias, fuel, sol, params);    
            



    //--------------------------------------------------------------//
    //----------- CALCULAR MEJOR SOLUCION CON TABU SEARCH ----------//
    //-- El tamanio de la lista tabu es ese por que significa que --//
    //-- cada vez que se alcanza la cuarta parte de las iteraciones //    
    //-- es posible volver a repetir una visita a una solucion -----//
    //--------------------------------------------------------------//
    int tamanio_lista_tabu = max(n_iter/4, 10);           
        
    vector <Instancia> TS = TabuSearch(sol, n_iter, params, tamanio_lista_tabu);       
    revisionFinal(TS);
    cout << "\n" << "Con un recorrido de largo: " << distanciaRecorrida(TS) << endl;   

    int vehiculo = 1;
       
    for(int i=0; i<TS.size(); i++)
    {        
        if ((TS[i]).type == "d" && i != TS.size()-1)
        {
            if (i!=0)
            {
                cout << " - "<<(TS[0]).id;
            }
            cout << "\n\n" << "Vehiculo " << vehiculo << ":\n";                                
            vehiculo++;
        }
        else if ((TS[i]).type != "d" || i == TS.size()-1)
        {
            cout << " - ";
        }        
        cout << (TS[i]).id;
        
    }    
    
                     
    cout << "\n\n";        
    return 0;
}


