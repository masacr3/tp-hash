#include <stdlib.h>
#include <stdbool.h>
#include "lista.h"
#include "hash.h"
/*
    estructura del hash abierto
*/

#define TAM_INICIAL 100 // Modificado TAMANIO_INICIAL
#define CANTIDAD_INICIAL 0

struct hash{
  lista_t** tabla;
  size_t capacidad; //tamaño del tabla filas
  size_t cantidad; // cantidad de elementos totales
  hash_destruir_dato_t destructor; // Modificado destruir_dato
};

typedef struct campo{
  char* clave;
  void* dato;
} campo_t; // Se agrego

struct hash_iter {
  const hash_t hash;
  lista_iter_t* iter_lista;
  size_t cantidad; // cantidad de elemento que itero
  size_t indice_actual; // indice actual;
};

size_t hashing (const char* clave, size_t capacidad){
  unsigned int n1 = 378551;
  unsigned int n2 = 63689;
  unsigned int clave_numerica = 0;
  for (int i =0; *clave; clave++; i++){
    clave_numerica = clave_numerica * n2 + (*clave);
    n2 = n2 * n1;
  }
  return (clave_numerica % capacidad);
}

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
  hash_t* hash = malloc(sizeof(hash_t));

  if(!hash) return NULL;

  //te conviene usar una estructura aux marto
  lista_t** tabla = (lista_t**) malloc(sizeof(lista_t*) * TAM_INICIAL);

  if(!tabla){
    free(hash);
    return NULL;
  }

  hash->tabla = tabla;

  //te corrijo que no inicializaste las posiciones del hash
  for(int i=0 ; i < TAM_INICIAL ; i++) hash->tabla[i] = lista_crear();

  hash->capacidad = TAM_INICIAL;
  hash->cantidad = CANTIDAD_INICIAL; // Modificado 0
  hash->destructor = destruir_dato;

  return hash;
}

size_t hash_cantidad(const hash_t *hash){
  return hash->cantidad;
}


/*
 + iteradores hash
 +
 */

//leo -> agregue esto
hash_iter_t *hash_iter_crear(const hash_t *hash){

  if(!hash) return NULL;

  hash_iter_t* iter = malloc(sizeof(hash_iter_t));

  if(!iter) return NULL;

  iter->hash = hash;
  iter->iter_lista = NULL;

  for (int pos = 0; pos < hash->capacidad; pos++){

    if( iter->hash->tabla[pos] && !lista_esta_vacia(iter->hash->tabla[pos])){
      iter->indice_actual = pos;
      iter->iter_lista = lista_iter_crear(iter->hash->tabla[pos]);

      break;
    }
  }

  return iter;
}

//leo -> me costo eh. estube 10 min pensandolo :P
bool hash_iter_avanzar(hash_iter_t *iter){
  //pregunto si estoy al final
  if(iter->iterados == iter->hash->cantidad) return false

  //update iterados
  iter->iterados++;

  //avanzo
  bool avanzo = lista_iter_avanzar(iter->iter_lista);

  if(avanzo) return true;

  //sino avanzo estaba en el final de lista
  //La destruyo y busco la nueva lista para iterar

  lista_iter_destruir(iter->iter_lista);

  //antes de recorrer me fijo si hay mas datos
  //sino hay no puedo avanzar

  //actualizo iter_lista
  iter->iter_lista = NULL;

  if(iter->iterados == iter->hash->cantidad ) return false;

  //exite un dato entonces lo busco .. no actualizo indice_actual para evitarme un if..
  for (int pos = iter->indice_actual+1; pos < iter->hash->capacidad; pos++  ){

    if( iter->hash->tabla[pos] && !lista_esta_vacia(iter->hash->tabla[pos])){
      iter->indice_actual = pos;
      iter->iter_lista = lista_iter_crear(iter->hash->tabla[pos]);
      return true;
    }
  }
  return false;
}

//leo -> agregue esto
bool hash_iter_al_final(const hash_iter_t *iter) {
    return iter->iterados == iter->hash->cantidad;
}

//leo -> agregue esto
const char *hash_iter_ver_actual(const hash_iter_t *iter){
  if ( iter->iterados == iter->hash->cantidad) return NULL;

  campo_t* campo = lista_iter_ver_actual(iter->iter_lista);

  return campo->clave;
}

//leo -> agregue esto
void hash_iter_destruir(hash_iter_t *iter) {
    if (iter->iter_lista) lista_iter_destruir(iter->iter_lista);
    free(iter);
}
 /** lee los comit te voy a cagar a trompadas marto gato **/
