#ifndef BACKEND_H
#define BACKEND_H

#include "../../slider.h"

/**
 * @brief Inicia el backend gráfico (ventana, eventos, loop).
 *        Esta función bloquea hasta que el usuario cierra la aplicación.
 * @param s Puntero al Slider cargado con datos y configuración.
 * @return El puntero final al Slider (puede cambiar durante hot-reload).
 *         El caller es responsable de llamar slider_free() con el puntero devuelto.
 */
Slider* backend_run(Slider *s);

#endif // BACKEND_H
