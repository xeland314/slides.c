# Git para Equipos
## Flujos de trabajo colaborativos

Una guía práctica para trabajar en equipo sin conflictos.

---
# ¿Por qué un flujo de trabajo?
## El caos sin convenciones

- Commits con mensajes como "fix", "arreglos", "cositas"
- `main` roto en producción los lunes
- Conflictos de merge que nadie quiere resolver
  - Especialmente en archivos generados automáticamente
  - O en `package-lock.json`

> "Un equipo sin convenciones de Git es un equipo que reescribe historia... literalmente."

---
# GitFlow Simplificado
## Las ramas que importan

| Rama | Propósito | ¿Merge a? |
| :--- | :--- | :---: |
| `main` | Código en producción | — |
| `develop` | Integración continua | `main` |
| `feature/*` | Nueva funcionalidad | `develop` |
| `hotfix/*` | Parche urgente | `main` |

---
# Commits Semánticos
## Convención estándar

```bash
# Formato: tipo(scope): descripción
git commit -m "feat(auth): agregar login con Google"
git commit -m "fix(api): corregir timeout en endpoint /users"
git commit -m "docs: actualizar README con instrucciones de instalación"
git commit -m "refactor(db): extraer lógica de queries a repositorio"
```

- `feat` — nueva funcionalidad
- `fix` — corrección de bug
- `docs` — solo documentación
- `refactor` — sin cambio de comportamiento

---
# Pull Requests
## Cómo hacer un PR que la gente quiera revisar

- **Tamaño**: máximo 400 líneas de diff
- **Descripción**: qué hace, por qué, cómo probarlo
- **Tests**: incluidos o explicación de por qué no

- [x] Titulo descriptivo en el PR
- [x] Al menos un reviewer asignado
- [ ] CI pasando antes de pedir revisión
- [ ] Sin TODOs sin ticket asociado

---
# Code Review
## Dar y recibir feedback

- Preguntar antes de asumir mala intención
  - "¿Consideraste usar X aquí?" en vez de "esto está mal"
- Aprobar con comentarios menores pendientes
- Distinguir: *blocker* vs *sugerencia* vs *nitpick*

```bash
# Actualizar tu rama con los últimos cambios de develop
git fetch origin
git rebase origin/develop

# Resolver conflictos, luego:
git rebase --continue
```

---
# Conclusión
## Tres reglas de oro

- **Commits pequeños y frecuentes**: más fácil de revisar y revertir
- **Nunca pushear directo a main**: siempre PR, siempre revisión
- **El historial es documentación**: escribe para tu yo del futuro

> "El mejor código es el que tu equipo puede entender a las 3am durante un incidente."
