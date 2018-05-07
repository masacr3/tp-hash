#include <stdlib.h>
#include <stdbool.h>
#include "lista.h"
#include "hash.h"
/*
    estructura del hash abierto
*/

#define TAMANIO_INICIAL 100 // Modificado TAMANIO_INICIAL
#define CANTIDAD_INICIAL 0
#define VALOR_REDIMENSION_GUARDAR 4
#define TAMANIO_REDIMENSION 2
#define VALOR_REDIMENSION_BORRAR 1

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

/* Crea el campo */
campo_t* campo_crear(const char* clave,void* dato){
  campo_t* campo=malloc(sizeof(campo_t));
  if (!campo) return NULL;
  campo->clave=clave;
  campo->dato=dato;
  return campo;
}

/* Libera la memoria requerida para la estructura campo.
 * Pre: el campo fue creado.
 * Post: la memoria requerida para la estructura campo fue liberada.
*/
void campo_destruir(campo_t* campo){
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
  lista_t** tabla = (lista_t**) malloc(sizeof(lista_t*) * TAMANIO_INICIAL);

  if(!tabla){
    free(hash);
    return NULL;
  }

  hash->tabla = tabla;

  //te corrijo que no inicializaste las posiciones del hash
  for(int i=0 ; i < TAMANIO_INICIAL ; i++) hash->tabla[i] = lista_crear();

  hash->capacidad = TAMANIO_INICIAL;
  hash->cantidad = CANTIDAD_INICIAL; // Modificado 0
  hash->destructor = destruir_dato;

  return hash;
}

size_t hash_cantidad(const hash_t *hash){
  return hash->cantidad;
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
  for (int i=0; i<tamanio_nuevo; i++) hash->tabla[i]=lista_crear();
  hash->capacidad=tamanio_nuevo;
  hash->cantidad=CANTIDAD_INICIAL;
  for (int i=0; i<tamanio_antiguo; i++){
    lista_t* lista_actual=tabla_vieja[i];
    if (lista_esta_vacia(lista_actual)) lista_destruir(lista_actual,NULL) continue;
    lista_iter_t* iter=lista_iter_crear(lista_actual);
    if (!iter) return false;

    // marto-> no estoy seguro de que no se rompa el dato asi que cuando lo corramos
    // va a saltar la ficha, mas que nada porque destruyo el dato que guardo pero
    // creo que para C si lo hago de esa manera no es el mismo, vivirian en otro lado.
    
    while (!lista_iter_al_final(iter)){
      char* clave_actual=lista_iter_ver_actual(iter)->clave;
      void* dato_actual=lista_iter_ver_actual(iter)->dato;
      ok &=_hash_guardar(hash,clave_actual,dato_actual);
      if (hash->destructor){
        hash_destruir_dato_t destruir=hash->destructor;
        destruir(lista_iter_ver_actual(iter)->dato);
      }
      lista_iter_avanzar(iter);
    }
    lista_iter_destruir(iter);
    lista_destruir(lista_actual,campo_destruir_wrapper);
  }
  free(tabla_vieja);
  return ok;
}

/* Funcion privada del hash_guardar.
 * Guarda un elemento en el hash, si la clave ya se encuentra en la
 * estructura, la reemplaza. De no poder guardarlo devuelve false.
 * Pre: La estructura hash fue inicializada.
 * Post: Se almacenó el par (clave, dato).
*/
bool _hash_guardar(hash_t* hash,const char* clave,void* dato){
  bool ok=true;
  size_t posicion=hashing(clave,hash->capacidad);
  campo_t* campo=campo_crear(clave,dato);
  if (!campo) return false;
  lista_t* lista_actual=hash->tabla[posicion];
  if (lista_esta_vacia(lista_actual)) ok &=lista_insertar_primero(lista_actual,campo);
  if (hash_pertenece(hash,clave)){
    lista_iter_t* iter=lista_iter_crear(lista_actual);
    if (!iter) return false;
    while (!lista_iter_al_final(iter)){
      if (lista_iter_ver_actual(iter)->clave==clave){
         void* campo_actual=lista_iter_borrar(iter);

         // marto-> pregunto si hay un destructor para luego destruir el dato si
         // es que existe dicho destructor del dato.

         if (hash->destructor){
           hash_destruir_dato_t destruir=hash->destructor;
           destruir(campo_actual->dato);
         }

         // marto-> destruyo el campo actual dado que se va a tener que reemplazar
         // la clave y el dato.

         destruir_campo(campo_actual);
         ok &=lista_iter_insertar(iter,campo);
         break;
      }
      lista_iter_avanzar(iter);
    }
    lista_iter_destruir(iter);
  }
  else ok &=lista_insertar_ultimo(lista,campo);
  if (ok) hash->cantidad++;
  return ok;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){

  // marto-> da lo mismo redimensionar antes o despues, lo hago antes para evitar
  // algun mal funcionamiento del hash.

  size_t factor_de_carga=hash->cantidad/hash->capacidad;
  size_t tamanio_nuevo=hash->capacidad*TAMANIO_REDIMENSION;
  if (factor_de_carga==VALOR_REDIMENSION_GUARDAR) hash_redimensionar(hash,tamanio_nuevo);
  return _hash_guardar(hash,clave,dato);
}

void* hash_borrar(hash_t* hash,const char* clave){

  // marto-> en este caso es lo mismo que en hash_guardar.

  size_t factor_de_carga=hash->cantidad/hash->capacidad;
  size_t tamanio_nuevo=hash->capacidad/TAMANIO_REDIMENSION;
  if (factor_de_carga==VALOR_REDIMENSION_BORRAR) hash_redimensionar(hash,tamanio_nuevo);
  if (!hash_pertenece(hash,clave)) return NULL;
  size_t posicion=hashing(clave,hash->capacidad);
  lista_t* lista_actual=hash->tabla[posicion];
  lista_iter_t* iter=lista_iter_crear(lista_actual);
  if (!iter) return NULL;

  // marto-> inicializo el dato aca para no perderlo. Corto la ejecucion si lo
  // encuentro porque no hay claves repetidas.

  void* dato;
  while (!lista_iter_al_final(iter)){
    if (lista_iter_ver_actual(iter)->clave==clave){
      campo_t* campo_actual=lista_iter_borrar(iter);
      dato=campo->dato;
      campo_destruir(campo_actual);
      hash->cantidad--;
      break;
    }
    lista_iter_avanzar(iter);
  }
  lista_iter_destruir(iter);
  return dato;
}

bool hash_pertenece(const hash_t* hash,const char* clave){
  bool ok=false;
  size_t posicion=hashing(clave,hash->capacidad);
  lista_t* lista_actual=hash->tabla[posicion];
  if (lista_esta_vacia(lista_actual)) return ok;
  lista_iter_t* iter=lista_iter_crear(lista_actual);
  if (!iter) return ok;

  // marto-> atualizo el valor si la clave pertenece pero me parecio mas claro
  // romper el ciclo y destruir el iterador despues y luego devolver true.
  // Corto la ejecucion si lo encuentro porque no hay claves repetidas.

  while (!lista_iter_al_final(iter)){
    if (lista_iter_ver_actual(iter)->clave==clave) ok &=true break;
    lista_iter_avanzar(iter);
  }
  lista_iter_destruir(iter);
  return ok;
}

void* hash_obtener(const hash_t* hash,const char* clave){
  if (!hash_pertenece(hash,clave)) return NULL;
  size_t posicion=hashing(clave,hash->capacidad);
  lista_t* lista_actual=hash->tabla[posicion];
  lista_iter_t* iter=lista_iter_crear(lista_actual);
  if (!iter) return NULL;
  void* dato;

  // marto-> como ver actual es un campo puedo acceder al dato y como antes, me
  // parecio mas claro cortar el ciclo, luego destruir el iterador y despues
  // devolver el dato.

  while (!lista_iter_al_final(iter)){
    if (lista_iter_ver_actual(iter)->clave==clave) dato=lista_iter_ver_actual(iter)->dato break;
    lista_iter_avanzar(iter);
  }
  lista_iter_destruir(iter);
  return dato;
}

void hash_destruir(hash_t* hash){

  // marto-> le pregunte a martin y me dijo que no quedaba otra que eliminar
  // cada elemento de la lista. No la avanzo porque el eliminar la avanza sola si
  // siempre elimino desde el principio.

  for (int i=0; i<hash->capacidad; i++){
    lista_t* lista_actual=hash->tabla[i];
    if (lista_esta_vacia(lista_actual)) lista_destruir(lista_actual,NULL) continue;
    lista_iter_t* iter=lista_iter_crear(lista_actual);
    if (!iter) return;
    while (!lista_iter_al_final(iter)){
      void* campo_actual=lista_iter_borrar(iter);
      if (hash->destructor){
        hash_destruir_dato_t destruir=hash->destructor;
        destruir(campo_actual->dato);
      }
      campo_destruir(campo_actual);
    }
    lista_iter_destruir(iter);
    lista_destruir(lista_actual,NULL);
  }
  free(hash);
}

/*******************************************************************************
 *                            ITERADOR
 ******************************************************************************/

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
