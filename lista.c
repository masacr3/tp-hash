#include "lista.h"
#include <stdlib.h>

typedef struct nodo nodo_t;

/***  Estructuras ***/
struct nodo {
  void *dato;
  nodo_t* siguiente;
};

struct lista{
  nodo_t* primero;
  nodo_t* ultimo;
  size_t cantidad;
};

struct lista_iter{
  lista_t* lista;
  nodo_t* actual;
  nodo_t* anterior;
};
/* Primitivas Nodo */

nodo_t* nodo_crear(void* valor){
  nodo_t* nodo = malloc(sizeof(nodo_t));

  if(!nodo) return NULL;

  nodo->dato = valor;
  nodo->siguiente = NULL;

  return nodo;
}

/* Primitivas Lista */
lista_t *lista_crear(void){
  lista_t* lista = malloc(sizeof(lista_t));

  if(!lista) return NULL;

  lista->primero = NULL;
  lista->ultimo = NULL;
  lista->cantidad = 0;

  return lista;
}

bool lista_esta_vacia(const lista_t *lista){
  return !lista->primero;
}

bool lista_insertar_primero(lista_t *lista, void *dato){
  nodo_t* nodo = nodo_crear(dato);

  if(!nodo) return false;

  if(!lista->primero) lista->ultimo = nodo;

  nodo->siguiente = lista->primero;
  lista->primero = nodo;
  lista->cantidad++;

  return true;
}

bool lista_insertar_ultimo(lista_t *lista, void *dato){
  nodo_t* nodo = nodo_crear(dato);

  if(!nodo) return false;

  if(!lista->primero) lista->primero = nodo;
  else lista->ultimo->siguiente = nodo;

  lista->ultimo = nodo;
  lista->cantidad++;

  return true;
}

void *lista_borrar_primero(lista_t *lista){

  if(!lista->primero) return NULL;

  void* nodo = lista->primero;
  void* dato = lista->primero->dato;

  lista->primero = lista->primero->siguiente;

  if(!lista->primero) lista->ultimo = NULL;

  free(nodo);
  lista->cantidad--;

  return dato;
}

void *lista_ver_primero(const lista_t *lista){
  return !lista->primero ? NULL : lista->primero->dato;
}

void *lista_ver_ultimo(const lista_t* lista){
  return !lista->primero ? NULL : lista->ultimo->dato;
}

size_t lista_largo(const lista_t *lista){
  return lista->cantidad;
}

void lista_destruir(lista_t *lista, void destruir_dato(void *)){

  while(!lista_esta_vacia(lista)){
    void* dato = lista_borrar_primero(lista);

    if(destruir_dato) destruir_dato(dato);

  }
  free(lista);
}

/* Primitivas iterador */
lista_iter_t *lista_iter_crear(lista_t *lista){
  lista_iter_t* iter = malloc(sizeof(lista_iter_t));

  if(!iter) return NULL;

  iter->lista = lista;
  iter->actual = lista->primero;
  iter->anterior = NULL;

  return iter;
}
bool lista_iter_avanzar(lista_iter_t *iter){

  if(!iter->actual) return false;

  iter->anterior = iter->actual;
  iter->actual = iter->actual->siguiente;

  return true;
}

void *lista_iter_ver_actual(const lista_iter_t *iter){
  return !iter->actual ? NULL : iter->actual->dato;
}

bool lista_iter_al_final(const lista_iter_t *iter){
  return !iter->actual;
}

void lista_iter_destruir(lista_iter_t *iter){
  free(iter);
}

bool lista_iter_insertar(lista_iter_t *iter, void *dato){
  nodo_t* nodo = nodo_crear(dato);

  if(!nodo) return false;

  nodo->siguiente = iter->actual;  //siempre se acopla al actual
  if(!iter->actual) iter->lista->ultimo = nodo;

  if(!iter->anterior) iter->lista->primero = nodo;
  else iter->anterior->siguiente = nodo;

  iter->actual = nodo;
  iter->lista->cantidad ++; //actualizo cantidad

  return true;

}

void *lista_iter_borrar(lista_iter_t *iter){
  if (!iter->actual) return NULL;

  void* nodo = iter->actual;
  void* dato = iter->actual->dato;

  iter->actual = iter->actual->siguiente;

  if(!iter->actual) iter->lista->ultimo = iter->anterior;

  if(!iter->anterior) iter->lista->primero = iter->actual;
  else iter->anterior->siguiente = iter->actual;

  free(nodo);
  iter->lista->cantidad--;

  return dato;

}

/* iterador interno */
void lista_iterar(lista_t *lista, bool visitar(void *dato, void *extra), void *extra){
  nodo_t* actual = lista->primero;
  while(actual && visitar(actual->dato,extra)) actual = actual->siguiente;
}
