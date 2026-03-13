#ifndef BACKEND_H
#define BACKEND_H

#include "../../slider.h"

/**
 * @brief Inicia el backend gráfico (ventana, eventos, loop).
 *        Esta función bloquea hasta que el usuario cierra la aplicación.
 * @param s Puntero al Slider cargado con datos y configuración.
 * @return 0 si termina correctamente, >0 si hay error.
 */
int backend_run(Slider *s);

#endif // BACKEND_H
