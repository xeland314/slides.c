#ifndef BACKEND_H
#define BACKEND_H

#include "../../slider.h"

/**
 * @brief Inicia el backend gráfico (ventana, eventos, loop).
 *        Bloquea hasta que el usuario cierra la aplicación.
 * @param initial     Array de Slider* ya cargados (puede ser NULL si n_initial == 0).
 *                    El backend toma posesión de todos y los libera al salir.
 * @param n_initial   Número de presentaciones iniciales. Con 0 se abre el selector de archivos.
 * @param configure   Callback para aplicar la configuración (CLI) a los Slider que el
 *                    backend cargue después (selector de archivos). Puede ser NULL.
 * @return Código de salida del proceso (0 = OK).
 */
int backend_run(Slider **initial, int n_initial, void (*configure)(Slider *));

#endif // BACKEND_H
