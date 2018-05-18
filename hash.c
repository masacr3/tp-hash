#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "hash.h"
#include "lista.h"
/*
    estructura del hash abierto
*/

#define TAMANIO_INICIAL 100
#define TAMANIO_REDIMENSION 2
#define VALOR_REDIMENSION 3

struct hash{
  lista_t** tabla;
  size_t capacidad;
  size_t cantidad;
  hash_destruir_dato_t destructor;
};

typedef struct campo{
  char* clave;
  void* dato;
} hash_campo_t;

struct hash_iter {
  const hash_t* hash;
  lista_iter_t* iter_lista;
  size_t iterados;
  size_t indice_actual;
};

/* Crea el campo */
hash_campo_t* hash_campo_crear(const char* clave,void* dato){
  hash_campo_t* campo=malloc(sizeof(hash_campo_t));

  if (!campo) return NULL;

  size_t largo=strlen(clave);
  char* clave_copia=malloc(sizeof(char)*largo+1);

  if (!clave_copia){
    free(campo);
    return NULL;
  }

  strcpy(clave_copia,clave);

  campo->clave=clave_copia;
  campo->dato=dato;
  return campo;
}

/* Libera la memoria requerida para la estructura campo.
 * Pre: el campo fue creado.
 * Post: la memoria requerida para la estructura campo fue liberada.
 */
void hash_campo_destruir(hash_campo_t* campo,hash_destruir_dato_t destruir_dato){

  if (destruir_dato) destruir_dato(campo->dato);

  free(campo->clave);
  free(campo);
}

/* Funcion de hashing para la clave. */
size_t djb2_hash(const char* cp){
    unsigned int hash = 5381;
    int c;

    while ((c = *cp++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

/* Esta es la funcion que se deberia usar para hashear la clave.
 * LLama a la funcion de hashing y le aplica el modulo del tamaño del hash.
 */
size_t hashing(const char *clave, size_t capacidad){
    return djb2_hash(clave) % capacidad;
}

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
  hash_t* hash = malloc(sizeof(hash_t));

  if(!hash) return NULL;

  lista_t** tabla = malloc(sizeof(lista_t*)*TAMANIO_INICIAL);

  if(!tabla){
    free(hash);
    return NULL;
  }

  hash->tabla = tabla;

  for (int i=0; i<TAMANIO_INICIAL; i++) hash->tabla[i]=lista_crear();

  hash->capacidad=TAMANIO_INICIAL;
  hash->cantidad=0;
  hash->destructor=destruir_dato;

  return hash;
}

size_t hash_cantidad(const hash_t *hash){
  return hash->cantidad;
}

/* Busca un elemento en particular en la lista y devuelve un iterador en la
 * posicion de dicho elemento. Si el elemento no esta en la lista devuelve
 * un iterador al final.
 */
lista_iter_t* buscar(lista_t* lista, const char* clave) {
    lista_iter_t* iter = lista_iter_crear(lista);
    if (!iter) return NULL;
    while (!lista_iter_al_final(iter) && strcmp(((hash_campo_t*)lista_iter_ver_actual(iter))->clave,clave) != 0){
        lista_iter_avanzar(iter);
    }
    return iter;
}

/* Redimensiona la estructura hash al nuevo tamanio que es tamanio_nuevo.
 * Pre: la estructura hash fue inicializada.
 * Post: se tiene un nuevo tamanio de la estructura hash el cual es el mismo que
 * el tamanio_nuevo recibido.
*/
bool hash_redimensionar(hash_t* hash,size_t tamanio_nuevo){
  lista_t** tabla_vieja=hash->tabla;
  size_t tamanio_antiguo=hash->capacidad;
  lista_t** tabla_nueva=malloc(sizeof(lista_t*)*tamanio_nuevo);

  if (!tabla_nueva) return false;

  hash->tabla=tabla_nueva;

  for (int i=0; i<tamanio_nuevo; i++) hash->tabla[i]=lista_crear();

  hash->capacidad=tamanio_nuevo;

  for (int i=0; i<tamanio_antiguo; i++){
    lista_t* lista_actual=tabla_vieja[i];

    if (lista_esta_vacia(lista_actual)){
    lista_destruir(lista_actual,NULL);
    continue;
    }

    while (!lista_esta_vacia(lista_actual)){
      hash_campo_t* campo_actual=lista_borrar_primero(lista_actual);
      size_t posicion=hashing(campo_actual->clave,hash->capacidad);
      lista_insertar_ultimo(hash->tabla[posicion],campo_actual);
    }
    lista_destruir(lista_actual,NULL);
  }

  free(tabla_vieja);
  return true;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){
  size_t factor_de_carga=hash->cantidad/hash->capacidad;
  size_t tamanio_nuevo=hash->capacidad*TAMANIO_REDIMENSION;

  if (factor_de_carga>=VALOR_REDIMENSION) hash_redimensionar(hash,tamanio_nuevo);

  size_t posicion=hashing(clave,hash->capacidad);
  lista_t* lista_actual=hash->tabla[posicion];
  lista_iter_t* iter=buscar(lista_actual,clave);

  if (!iter) return false;

  if (!lista_iter_al_final(iter)){
    hash_campo_t* campo_actual=lista_iter_ver_actual(iter);

    if (hash->destructor) hash->destructor(campo_actual->dato);

    campo_actual->dato=dato;
  }

  else{
    hash_campo_t* campo=hash_campo_crear(clave,dato);

    if (!campo) return false;

    lista_insertar_ultimo(lista_actual,campo);
    hash->cantidad++;
  }

  lista_iter_destruir(iter);
  return true;
}

void* hash_borrar(hash_t* hash,const char* clave){
  size_t factor_de_carga=hash->cantidad/hash->capacidad;
  size_t tamanio_nuevo=hash->capacidad/TAMANIO_REDIMENSION;

  if (factor_de_carga>=VALOR_REDIMENSION && tamanio_nuevo>=TAMANIO_INICIAL) hash_redimensionar(hash,tamanio_nuevo);

  size_t posicion=hashing(clave,hash->capacidad);
  lista_t* lista_actual=hash->tabla[posicion];
  lista_iter_t* iter=buscar(lista_actual,clave);

  if (!iter) return NULL;

  void* dato;

  if (lista_iter_al_final(iter)) dato=NULL;

  else{
    hash_campo_t* campo_actual=lista_iter_borrar(iter);
    dato=campo_actual->dato;
    hash_campo_destruir(campo_actual,NULL);
    hash->cantidad--;
  }

  lista_iter_destruir(iter);
  return dato;
}

bool hash_pertenece(const hash_t* hash,const char* clave){
  size_t coincidencia=0;
  size_t posicion=hashing(clave,hash->capacidad);
  lista_t* lista_actual=hash->tabla[posicion];

  if (lista_esta_vacia(lista_actual)) return false;

  lista_iter_t* iter=lista_iter_crear(lista_actual);

  if (!iter) return false;

  while (!lista_iter_al_final(iter)){
    hash_campo_t* campo_actual=lista_iter_ver_actual(iter);
    char* clave_actual=campo_actual->clave;
    if (strcmp(clave,clave_actual)==0) coincidencia++;
    lista_iter_avanzar(iter);
  }

  lista_iter_destruir(iter);

  if (!coincidencia) return false;

  return true;
}

void* hash_obtener(const hash_t* hash,const char* clave){
  size_t posicion=hashing(clave,hash->capacidad);
  lista_t* lista_actual=hash->tabla[posicion];
  lista_iter_t* iter=buscar(lista_actual,clave);

  if (!iter) return NULL;

  void* dato;

  if (lista_iter_al_final(iter)) dato=NULL;

  else{
    hash_campo_t* campo_actual=lista_iter_ver_actual(iter);
    dato=campo_actual->dato;
  }

  lista_iter_destruir(iter);
  return dato;
}

void hash_destruir(hash_t* hash){

  for (int i=0; i<hash->capacidad; i++){

    if (!hash->tabla[i]) continue;

    lista_t* lista_actual=hash->tabla[i];

    while (!lista_esta_vacia(lista_actual)){
      hash_campo_t* campo_actual=lista_borrar_primero(lista_actual);
      hash_campo_destruir(campo_actual,hash->destructor);
    }

    lista_destruir(lista_actual,NULL);
  }

  free(hash->tabla);
  free(hash);
}

/*******************************************************************************
 *                            ITERADOR
 ******************************************************************************/

hash_iter_t* hash_iter_crear(const hash_t *hash){
  hash_iter_t* iter=malloc(sizeof(hash_iter_t));

  if (!iter) return NULL;

  iter->hash=hash;
  iter->indice_actual = hash->capacidad+1;
  iter->iterados = 0;
  if (!hash->cantidad) return iter;

  for (size_t i=0; i<hash->capacidad; i++){
    lista_t* lista_actual=hash->tabla[i];

    if (!lista_esta_vacia(lista_actual)){
      iter->iter_lista=lista_iter_crear(lista_actual);
      iter->indice_actual=i;
      break;
    }

  }
  return iter;
}

bool hash_iter_avanzar(hash_iter_t *iter){

  if (iter->iterados == iter->hash->cantidad) return false;

  iter->iterados++;
  lista_iter_avanzar(iter->iter_lista);

  if (!lista_iter_al_final(iter->iter_lista)) return true;

  lista_iter_destruir(iter->iter_lista);
  iter->iter_lista=NULL;

  for (size_t pos = iter->indice_actual+1; pos < iter->hash->capacidad; pos++){
    lista_t* lista_actual=iter->hash->tabla[pos];

    if (!lista_esta_vacia(lista_actual)){
      iter->indice_actual = pos;
      iter->iter_lista = lista_iter_crear(lista_actual);
      return true;
    }

  }

  return false;
}

bool hash_iter_al_final(const hash_iter_t *iter) {
    return iter->iterados == iter->hash->cantidad;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter){
  if (iter->iterados == iter->hash->cantidad) return NULL;

  hash_campo_t* campo = lista_iter_ver_actual(iter->iter_lista);

  return campo->clave;
}

void hash_iter_destruir(hash_iter_t *iter) {
  if (!(iter->iterados == iter->hash->cantidad)) lista_iter_destruir(iter->iter_lista);
  free(iter);
}
