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
#define VALOR_REDIMENSION_GUARDAR 8
#define TAMANIO_REDIMENSION 2

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
hash_campo_t* campo_crear(const char* clave,void* dato){
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
void campo_destruir(hash_campo_t* campo){
  free(campo->clave);
  free(campo);
}

/* Funcion que enmascara la funcion campo_destruir.
 * Pre: la estructura campo fue inicializada.
 * Post: la memoria requerida para la estructura campo fue liberada  si se
 * pasa un destructor se libera la memoria requerida para el dato.
*/
void campo_destruir_wrapper(void* campo){
  campo_destruir(campo);
}

size_t hashing (const char* clave, size_t tam){
	unsigned int num1 = 378551;
	unsigned int num2 = 63689;
	unsigned int clave_numerica = 0;
	unsigned int clave_como_int = *(unsigned int*)clave;
	for(int i = 0; *clave; clave++, i++){
		clave_numerica = clave_numerica * num2 + clave_como_int;
		num2 = num2 * num1;
	}
	return(clave_numerica%tam);
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

  for(int i=0; i<TAMANIO_INICIAL; i++) hash->tabla[i] = NULL;

  hash->capacidad=TAMANIO_INICIAL;
  hash->cantidad=0;
  hash->destructor=destruir_dato;

  return hash;
}

size_t hash_cantidad(const hash_t *hash){
  return hash->cantidad;
}

/* Funcion privada del hash_guardar.
 * Guarda un elemento en el hash, si la clave ya se encuentra en la
 * estructura, la reemplaza. De no poder guardarlo devuelve false.
 * Pre: La estructura hash fue inicializada.
 * Post: Se almacenó el par (clave, dato).
*/
bool _hash_guardar(hash_t* hash,const char* clave,void* dato){
  size_t posicion=hashing(clave,hash->capacidad);
  hash_campo_t* campo=campo_crear(clave,dato);

  if (!campo) return false;

  if (!hash->tabla[posicion]) hash->tabla[posicion]=lista_crear();

  lista_t* lista_actual=hash->tabla[posicion];

  if (!lista_actual) return false;

  if (hash_pertenece(hash,clave)){
    lista_iter_t* iter=lista_iter_crear(lista_actual);

    if (!iter) return false;

    while (!lista_iter_al_final(iter)){
      hash_campo_t* campo_actual=lista_iter_ver_actual(iter);
      char* clave_actual=campo_actual->clave;

      if (strcmp(clave,clave_actual)==0){
         hash_campo_t* _campo_actual=lista_iter_borrar(iter);

         if (hash->destructor){
           hash_destruir_dato_t destruir=hash->destructor;
           destruir(_campo_actual->dato);
         }

         campo_destruir(_campo_actual);
         lista_iter_insertar(iter,campo);
         break;
      }
      lista_iter_avanzar(iter);
    }

    lista_iter_destruir(iter);
  }

  else{
    lista_insertar_ultimo(lista_actual,campo);
    hash->cantidad++;
  }

  return true;
}

/* Redimensiona la estructura hash al nuevo tamanio que es tamanio_nuevo.
 * Pre: la estructura hash fue inicializada.
 * Post: se tiene un nuevo tamanio de la estructura hash el cual es el mismo que
 * el tamanio_nuevo recibido.
*/
bool hash_redimensionar(hash_t* hash,size_t tamanio_nuevo){
  bool ok=true;
  lista_t** tabla_vieja=hash->tabla;
  size_t tamanio_antiguo=hash->capacidad;
  lista_t** tabla_nueva=malloc(sizeof(lista_t*)*tamanio_nuevo);

  if (!tabla_nueva) return false;

  hash->tabla=tabla_nueva;

  for (int i=0; i<tamanio_nuevo; i++) hash->tabla[i]=NULL;

  hash->capacidad=tamanio_nuevo;
  hash->cantidad=0;

  for (int i=0; i<tamanio_antiguo; i++){

    if (!tabla_vieja[i]) continue;

    lista_t* lista_actual=tabla_vieja[i];

    if (lista_esta_vacia(lista_actual)){
    lista_destruir(lista_actual,NULL);
    continue;
    }

    lista_iter_t* iter=lista_iter_crear(lista_actual);

    if (!iter) return false;

    while (!lista_iter_al_final(iter)){
      hash_campo_t* campo_actual=lista_iter_ver_actual(iter);
      ok &=_hash_guardar(hash,campo_actual->clave,campo_actual->dato);
      lista_iter_avanzar(iter);

    }
    lista_iter_destruir(iter);
    lista_destruir(lista_actual,campo_destruir_wrapper);
  }

  free(tabla_vieja);
  return ok;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){
  size_t factor_de_carga=hash->cantidad/hash->capacidad;
  size_t tamanio_nuevo=hash->capacidad*TAMANIO_REDIMENSION;

  if (factor_de_carga==VALOR_REDIMENSION_GUARDAR) hash_redimensionar(hash,tamanio_nuevo);

  return _hash_guardar(hash,clave,dato);
}

void* hash_borrar(hash_t* hash,const char* clave){
  size_t factor_de_carga=hash->cantidad/hash->capacidad;
  size_t tamanio_nuevo=hash->capacidad/TAMANIO_REDIMENSION;
  size_t un_cuarto_hash=hash->capacidad/4;

  if (factor_de_carga==un_cuarto_hash && tamanio_nuevo>=TAMANIO_INICIAL) hash_redimensionar(hash,tamanio_nuevo);

  if (!hash_pertenece(hash,clave)) return NULL;

  size_t posicion=hashing(clave,hash->capacidad);
  lista_t* lista_actual=hash->tabla[posicion];
  lista_iter_t* iter=lista_iter_crear(lista_actual);

  if (!iter) return NULL;

  void* dato=NULL;

  while (!lista_iter_al_final(iter)){
    hash_campo_t* campo_actual=lista_iter_ver_actual(iter);
    char* clave_actual=campo_actual->clave;

    if (strcmp(clave,clave_actual)==0){
      hash_campo_t* _campo_actual=lista_iter_borrar(iter);
      dato=_campo_actual->dato;
      campo_destruir(_campo_actual);
      hash->cantidad--;
      break;
    }

    lista_iter_avanzar(iter);
  }

  lista_iter_destruir(iter);
  return dato;
}

bool hash_pertenece(const hash_t* hash,const char* clave){
  size_t coincidencia=0;
  size_t posicion=hashing(clave,hash->capacidad);
  lista_t* lista_actual=hash->tabla[posicion];

  if (!lista_actual) return false;

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

  if (!hash_pertenece(hash,clave)) return NULL;

  size_t posicion=hashing(clave,hash->capacidad);
  lista_t* lista_actual=hash->tabla[posicion];
  lista_iter_t* iter=lista_iter_crear(lista_actual);

  if (!iter) return NULL;

  void* dato=NULL;

  while (!lista_iter_al_final(iter)){
    hash_campo_t* campo_actual=lista_iter_ver_actual(iter);
    char* clave_actual=campo_actual->clave;

    if (strcmp(clave,clave_actual)==0){
      dato=campo_actual->dato;
      break;
    }

    lista_iter_avanzar(iter);
  }

  lista_iter_destruir(iter);
  return dato;
}

void hash_destruir(hash_t* hash){

  for (int i=0; i<hash->capacidad; i++){

    if (!hash->tabla[i]) continue;

    lista_t* lista_actual=hash->tabla[i];

    if (lista_esta_vacia(lista_actual)){
      lista_destruir(lista_actual,NULL);
      continue;
    }

    lista_iter_t* iter=lista_iter_crear(lista_actual);

    if (!iter) return;

    while (!lista_iter_al_final(iter)){
      hash_campo_t* campo_actual=lista_iter_borrar(iter);

      if (hash->destructor){
        hash_destruir_dato_t destruir=hash->destructor;
        destruir(campo_actual->dato);
      }

      campo_destruir(campo_actual);
    }

    lista_iter_destruir(iter);
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

    if (!lista_actual) continue;

    iter->iter_lista=lista_iter_crear(lista_actual);
    iter->indice_actual=i;
    break;
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

    if (!lista_actual) continue;

    iter->indice_actual = pos;
    iter->iter_lista = lista_iter_crear(lista_actual);
    return true;
  }

  return false;
}

bool hash_iter_al_final(const hash_iter_t *iter) {
    return iter->iterados == iter->hash->cantidad;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter){
  if (hash_iter_al_final(iter)) return NULL;

  hash_campo_t* campo = lista_iter_ver_actual(iter->iter_lista);

  return campo->clave;
}

void hash_iter_destruir(hash_iter_t *iter) {
  if (!hash_iter_al_final(iter)) lista_iter_destruir(iter->iter_lista);
  free(iter);
}
