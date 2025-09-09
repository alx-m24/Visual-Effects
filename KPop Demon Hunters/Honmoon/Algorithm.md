# Honmoon - Kpop Demon Hunters

This is my attempt at rendering/recreating the Honmoon effect from KPH in OpenGL

## Steps

1. Render scene while storing position and and normal values
2. Render a large quad:
    1. Read stored terrain position and normal
    2. For each vertex of quad (in vertex shader) offset quad a bit
    3. Add color and get grid/circular pattern using mathematical equations

## In practice

