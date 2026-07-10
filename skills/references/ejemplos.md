# Ejemplos de Presentaciones C-Slides

## Ejemplo 1: Presentación técnica / arquitectura

```markdown
# Sistema de Microservicios
## Arquitectura y despliegue en producción

Una guía práctica para equipos de backend.

---
# El Problema
## Monolitos que crecen demasiado

- Deploys lentos y riesgosos
- Equipos bloqueados entre sí
- Escalar = escalar todo

> "Un monolito bien hecho siempre gana... hasta que no."

---
# La Solución
## Dividir por dominio de negocio

| Servicio | Responsabilidad | Tecnología |
| :--- | :--- | :---: |
| Auth | Autenticación y tokens | Go |
| Users | Perfil y preferencias | Python |
| Orders | Pedidos y pagos | Java |
| Notify | Emails y push | Node.js |

---
# Comunicación
## Sync vs Async

- **Sync (REST/gRPC)**: cuando necesitas respuesta inmediata
  - Login, consulta de saldo
- **Async (Kafka/RabbitMQ)**: cuando puedes procesar después
  - Notificaciones, reportes, auditoría

---
# Código de Ejemplo
## Health check estándar

```go
func healthHandler(w http.ResponseWriter, r *http.Request) {
    // Verificar dependencias críticas
    if !db.Ping() {
        w.WriteHeader(503)
        return
    }
    w.WriteHeader(200)
}
```

---
# Conclusión
## Puntos clave

- [x] Dividir por dominio, no por capa técnica
- [x] API Gateway como punto de entrada único
- [ ] Observabilidad desde el día uno
- [ ] Circuit breakers en todas las llamadas externas
```

---

## Ejemplo 2: Presentación de producto / demo

```markdown
# Nombre del Producto
## Resolviendo X para Y tipo de usuario

---
# El Problema
## Qué duele hoy

- Los usuarios pierden N horas en X
- Las soluciones actuales son complejas o caras
- No existe una opción para Z caso de uso

---
# Nuestra Solución
## Cómo funciona en 3 pasos

- **Paso 1**: El usuario hace A
- **Paso 2**: El sistema procesa B automáticamente
- **Paso 3**: Resultado listo en segundos

---
# Demo
## Lo que acabas de ver

!screenshot_demo.png

---
# Métricas
## Resultados en beta

| Métrica | Antes | Después | Mejora |
| :--- | :---: | :---: | :---: |
| Tiempo de setup | 2h | 5min | 96% ↓ |
| Errores manuales | 23% | 1% | 95% ↓ |
| Satisfacción | 3.1/5 | 4.8/5 | 55% ↑ |

---
# Conclusión
## Próximos pasos

> "Simple de adoptar, poderoso en producción."

- Beta abierta disponible hoy
- Plan gratuito hasta 1,000 usuarios
- Soporte directo en Slack
```

---

## Ejemplo 3: Presentación educativa / clase

```markdown
# Algoritmos de Ordenamiento
## Clase 4 — Estructuras de Datos II

---
# ¿Qué veremos hoy?
## Agenda

- Repaso de complejidad O(n)
- Bubble Sort — el clásico ineficiente
- Merge Sort — divide y vencerás
- Cuándo usar cada uno

---
# Complejidad
## Notación Big O resumida

| Algoritmo | Mejor caso | Peor caso | Estable |
| :--- | :---: | :---: | :---: |
| Bubble Sort | O(n) | O(n²) | ✅ |
| Merge Sort | O(n log n) | O(n log n) | ✅ |
| Quick Sort | O(n log n) | O(n²) | ❌ |

---
# Bubble Sort
## La idea base

```python
def bubble_sort(arr):
    n = len(arr)
    for i in range(n):
        for j in range(0, n-i-1):
            if arr[j] > arr[j+1]:
                # intercambiar
                arr[j], arr[j+1] = arr[j+1], arr[j]
```

- Simple de entender e implementar
- Útil para listas casi ordenadas
- **Evitar** en datasets grandes

---
# Merge Sort
## Divide y vencerás

```python
def merge_sort(arr):
    if len(arr) <= 1:
        return arr
    mid = len(arr) // 2
    left = merge_sort(arr[:mid])
    right = merge_sort(arr[mid:])
    return merge(left, right)
```

---
# ¿Cuándo usar cuál?
## Guía práctica

- **Bubble Sort**: arrays pequeños (<50 elementos), código educativo
- **Merge Sort**: cuando necesitas estabilidad garantizada
- **Quick Sort**: uso general en producción (mejor constante)

> "Conoce tus datos antes de elegir tu algoritmo."

---
# Tarea
## Para la próxima clase

- [x] Implementar Bubble Sort en tu lenguaje favorito
- [ ] Implementar Merge Sort con recursión
- [ ] Comparar tiempos con arrays de 100, 1000 y 10000 elementos
- [ ] Llevar preguntas sobre Quick Sort
```
