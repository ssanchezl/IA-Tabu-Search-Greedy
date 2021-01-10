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
//--- DEVUELVE la distancia de harvesine entre ambos ---------------//
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
//- FUNCION: Ordenar distancias de menor a mayor -------------------//
//------------------------------------------------------------------//
//- RECIBE vector distancias ---------------------------------------//
//------------------------------------------------------------------//
//- DEVUELVE el vector ordenado con distancias de mayor a menor ----//
//------------------------------------------------------------------//
void SortDist(vector<Distancia> &sort_dist){
    vector<Distancia> aux = sort_dist;    
    vector<Distancia> res;        

    int index;
    double min = 32767;        
    while (aux.size()) {
        for (int i = 0; i < aux.size(); i++)
        {
            if ((aux[i]).dist < min) {
                min = (aux[i]).dist;
                index = i;
            }
        }
        min = 32767;
        res.push_back(aux[index]);        
        aux.erase(aux.begin() + index);
    }    
    sort_dist = res; 
    aux.clear();
    res.clear();
}




//------------------------------------------------------------------//
//-- FUNCION: Obtener nodo mas cercano -----------------------------//
//------------------------------------------------------------------//
//-- RECIBE las distancias, las instancias y el i-esimo nodo -------//
//------------------------------------------------------------------//
//-- DEVUELVE los indices del node mas cercano al que se pide ------//
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

        // Revisar que si el nodo anterior no es cliente, el actual sea cliente
        if ((sol[i-1]).type != "c" && (sol[i]).type != "c" )
        
            return false;
        


        Q -= Harvesine(sol[i-1], sol[i])*r;        
        t += Harvesine(sol[i-1], sol[i])/v;  
        
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

        if(t >= TL) {            
            return false;
        }        
    }
    return true;
} 


//------------------------------------------------------------------//
//-- FUNCION: Aplicar las restricciones a la solucion --------------//
//------------------------------------------------------------------//
//-- RECIBE distancia, solucion, combustible y parametros ----------//
//------------------------------------------------------------------//
//-- DEVUELVE el vector solucion restringido -----------------------//
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
	vector<Instancia> aux_sol;
	aux_sol.push_back(sol[0]);	

    vector<int> sol_index;
    sol_index.push_back(0);

	int ultimo = aux_sol.size()-1;	
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
			dist_i = Harvesine(aux_sol[ultimo], sol[i]);

			// Distancia entre nodo actual y deposito
			dist_d = Harvesine(sol[i], aux_sol[0]);	

            // Tiempo de viaje
            t += dist_i/v + min(dist_d/v, (min_fuel_dist[i]).dist/v);

			if ((aux_sol[ultimo]).id != (sol[i]).id && find(ids.begin(), ids.end(), (sol[i]).id) == ids.end() )
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
							if ((sol[i]).type != "c" && (aux_sol[ultimo]).type != "c")
							{		
                                t += - dist_i/v - min(dist_d/v, (min_fuel_dist[i]).dist/v);                                								                                
								continue;
							}	
                            nodo_cercano = dist_i;
							nodo_ok = true;                              
                            idx = i;                             
						}
					}
				}
			}	
            t += - dist_i/v - min(dist_d/v, (min_fuel_dist[i]).dist/v);		
		}          

		// Una vez cumplidas las restricciones, se procede a agregar        

        // Si alcanza el f        
        if (find(ids.begin(), ids.end(), (sol[idx]).id) == ids.end())
        {
            if(nodo_cercano*r + actualQ < Q && nodo_ok)
            {                   
                actualQ += nodo_cercano*r;
                t += nodo_cercano/v;

                
                // Se agrega el nodo a la solucion
                aux_sol.push_back(sol[idx]);                    

                ultimo = aux_sol.size()-1;                          
                                    
                if ((aux_sol[ultimo]).type == "c")
                {       
                    min_fuel_dist.erase(min_fuel_dist.begin() + idx);                       
                    ids.push_back((sol[idx]).id);   
                    t += 30;
                }
                else if ((aux_sol[ultimo]).type == "f")
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
                if((aux_sol[ultimo]).type != "d")
                {                
                    aux_sol.push_back(sol[0]);                 
                    actualQ = 0;
                    t = 0;
                }                              
            }
        }
			                                      
        vuelta++;                
	}	    
	sol = aux_sol; 
    sol.push_back(sol[0]);
}




//------------------------------------------------------------------//
//-- FUNCION: Obtener solución inicial -----------------------------//
//------------------------------------------------------------------//
//-- RECIBE: instancias, distancias, combustibles y parametros -----//
//------------------------------------------------------------------//
//-- DEVUELVE la solucion inicial candidata en forma de instancias -//
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

    // Ordenar distancias de menor a mayor
    SortDist(distancia);  

    // Agregar a la solucion inicial el vecino mas cercano
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
//-- DEVUELVE la distancia total del recorrido ---------------------//
//------------------------------------------------------------------//
void distanciaRecorrida(double &dist_recor, vector<Instancia> sol)
{    
    for (int i = 0; i < sol.size()-1; ++i)
    {
        dist_recor += Harvesine(sol[i], sol[i+1]);
    }    
}




//------------------------------------------------------------------//
//-- FUNCION: Colocar en reverso los nodes desde A hasta B ---------//
//------------------------------------------------------------------//
//-- RECIBE: vector solucion inicial -------------------------------//
//------------------------------------------------------------------//
//-- DEVUELVE la solucion cambiada con el movimiento 2 OPT ---------//
//------------------------------------------------------------------//
vector <Instancia> movimiento(vector<Instancia> sol, int A, int B)
{
    vector <Instancia> move;
    int n;    
    for(n=0; n<A; n++)
    {
        move.push_back(sol[n]);
    }
    for(n=B; n>=A; n--)
    {
        move.push_back(sol[n]);
    }
    for(n=B + 1; n < sol.size(); n++)
    {
        move.push_back(sol[n]);
    }
    sol = move;
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

    //printf("%s\n", argv[2]); NO BORRAR!!!
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

    //agregarDepot(sol, Parametros);

    //--------------------------------------------------------------//
    //---------------- CALCULAR DISTANCIA RECORRIDA ----------------//
    //----- Aqui se calcula la distancia total del recorrido -------//    
    //--------------------------------------------------------------//
    
    double dist_recor = 0;
    distanciaRecorrida(dist_recor, sol);
    
    //cout << "solucion: "<< (sol_valida(sol, Parametros) ? "valida" : "no valida") << "\n";
    for (int i = 0; i < sol.size(); ++i)
    {
        cout << (sol[i]).id << "\n";
    }
    //cout << "\nDistancia recorrida: " << dist_recor;
    
          
    
    cout << "\n";        
    return 0;
}

