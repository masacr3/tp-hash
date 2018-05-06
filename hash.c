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
