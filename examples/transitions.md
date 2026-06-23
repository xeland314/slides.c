# Efectos de Transición
## Navega con las flechas → para verlos

Cada slide define la transición al ENTRAR.

La primera slide no tiene transición (es la inicial).

---

<!-- transition: fade -->

# Fade
## Fundido cruzado

`<!-- transition: fade -->`

Desvanecimiento suave entre slides.

- Limpio y profesional
- Ideal para cambios sutiles
- No distrae a la audiencia

---

<!-- transition: slide-left -->

# Slide Left
## Entrada desde la derecha

La nueva slide entra deslizándose desde la derecha mientras la anterior sale hacia la izquierda.

- Sensación de avance horizontal
- Natural para presentaciones lineales
- Similar a pasar páginas

---

<!-- transition: slide-right -->

# Slide Right
## Entrada desde la izquierda

La nueva slide entra desde la izquierda. Ideal para cuando retrocedes y vuelves a avanzar.

- Útil como efecto de "regreso"
- Da contexto direccional

---

<!-- transition: slide-up -->

# Slide Up
## Entrada desde abajo

La nueva slide sube desde abajo, como un telón.

- Efecto de revelado
- Bueno para secciones nuevas
- Sensación de progreso vertical

---

<!-- transition: slide-down -->

# Slide Down
## Entrada desde arriba

La nueva slide baja desde arriba.

---

<!-- transition: none -->

# Sin Transición
## Cambio instantáneo

`<!-- transition: none -->`

Útil para slides de referencia rápida donde la animación distraería.

---

# Resumen
## Tabla de tipos disponibles

| Directiva | Efecto |
| :--- | :--- |
| `transition: fade` | Fundido cruzado |
| `transition: slide-left` | Deslizar a la izquierda |
| `transition: slide-right` | Deslizar a la derecha |
| `transition: slide-up` | Deslizar hacia arriba |
| `transition: slide-down` | Deslizar hacia abajo |
| `transition: none` | Sin transición |

> La directiva debe ir ANTES del contenido del slide (después del `---`).
> Duración: 300ms con easing smoothstep.
