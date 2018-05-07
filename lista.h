#include <stdlib.h>
#include <stdbool.h>

#ifndef LISTA_H
#define LISTA_H

/* Se trata de una lista que contiene datos de tipo void*
 * (punteros genéricos).  La lista en sí está definida en el .c.  */
typedef struct lista lista_t;

// Crea una lista
// Post: Devuelve una nueva lista vacia
lista_t *lista_crear(void);

// Devuelve verdadero o falso, según si la lista tiene o no elementos.
// Pre: la pila fue creada.
bool lista_esta_vacia(const lista_t *lista);

#include "lista.h"
#include "testing.h"
#include <stddef.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>

// Inserta un dato en la primera posicion de la lista.
// Pre: La lista esta creada.
// Post: Devuelve true si pudo insertar el dato en caso contrario false
bool lista_insertar_primero(lista_t *lista, void *dato);

// Inserta un dato en la ultima posicion de la lista.
// Pre: La lista esta creada.
// Post: Devuelve true si pudo insertar el dato en caso contrario false
bool lista_insertar_ultimo(lista_t *lista, void *dato);

// Borra el primer dato de la lista y retorna el valor del dato borrado.
// Si no hay ningun dato retorna NULL.
// Pre: la lista esta creada.
// Post: Retorna el valor del dato borrado
void *lista_borrar_primero(lista_t *lista);

// Obtiene el valor del primero elemento de la lista. Si la lista tiene
// elementos, se devuelve el valor del primero, si está vacía devuelve NULL.
// Pre: la lista fue creada.
// Post: se devolvió el primero elemento de la lista, cuando no está vacía.
void *lista_ver_primero(const lista_t *lista);

// Obtiene el valor del ultimo elemento de la lista. Si la lista tiene
// elementos, se devuelve el valor del ultimo, si está vacía devuelve NULL.
// Pre: la lista fue creada.
// Post: se devolvió el ultimo elemento de la lista, cuando no está vacía.
void *lista_ver_ultimo(const lista_t* lista);

// Devuelve el largo de la lista
// Pre: la lista esta creada
// Post: devolvió el largo de la lista
size_t lista_largo(const lista_t *lista);

// Destruye la lista. Si se recibe la función destruir_dato por parámetro,
// para cada uno de los elementos de la lista llama a destruir_dato.
// Pre: la lista fue creada. destruir_dato es una función capaz de destruir
// los datos de la lista, o NULL en caso de que no se la utilice.
// Post: se eliminaron todos los elementos de la lista.
void lista_destruir(lista_t *lista, void destruir_dato(void *));

/* Primitiva del iterador interno */
// Esta estructura itera la lista y para cada elemento de la lista llama a
// funcion visitar
// Pre: La lista esta creada
// Post: Modifica la lista original, si visitar devuelve false la lista se deja de iterar

void lista_iterar(lista_t *lista, bool visitar(void *dato, void *extra), void *extra);


#endif // LISTA_H

#ifndef LISTA_ITER_H
#define LISTA_ITER_H

/* Se trata de un iterador que permite recorrer la lista de forma eficiente.
 * Esta estructura permite agregar y borrar datos de la lista */
typedef struct lista_iter lista_iter_t;

// Crea un iterador, y lo posiciona a este en la primera posicion de la lista.
// Pre: La lista esta creada.
// Post: Retorna un iterador parado en la primera posicion de la lista.
lista_iter_t *lista_iter_crear(lista_t *lista);

// Avanza a la siguiente posicion de la lista , si pudo avanzar devuelve true sino false.
// Pre: El iterador esta creado.
// Post: Si avanzo devuelve true sino false
bool lista_iter_avanzar(lista_iter_t *iter);

// Obtiene el valor actual del iterador . Si el iterador esta al final devuelve NULL.
// Pre: El iterador esta creado.
// Post: Devuelve el valor actual del iterador. Si el iterador esta al final devuelve NULL.
void *lista_iter_ver_actual(const lista_iter_t *iter);

// Verifica si el iterador esta al final de la lista , si esta al final devuelve true sino false.
// Pre: El iterador esta creado.
// Post: Si esta al final devuelve true sino false.
bool lista_iter_al_final(const lista_iter_t *iter);

// Destruye el iterador
// Pre: el iterador esta creado.
// Post: destruyo el iterador
void lista_iter_destruir(lista_iter_t *iter);

// Inserta un dato en la lista en la posicion actual del iterador, si pudo insertar devuelve true sino false.
// Pre: El iterador esta creado.
// Post: Si pudo insertar el dato devuelve true sino false.
bool lista_iter_insertar(lista_iter_t *iter, void *dato);

// Borra el dato de la posicion actual del iterador, retornando el valor del daato borrador.
// Si el iterador esta al final retorna NULL.
// Pre: el iterador esta creado.
// Post: Se devolvio el valor del dato borrado , si el iterador esta al final devuelve false
void *lista_iter_borrar(lista_iter_t *iter);

#endif // LISTA_ITER_H

void pruebas_lista_alumno();
