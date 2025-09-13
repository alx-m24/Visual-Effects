# Honmoon - Kpop Demon Hunters

This is my attempt at rendering/recreating the Honmoon effect from KPH in OpenGL

## Steps

1. Render height map of area covered by Honmoon
2. Render a large quad:
    1. Read stored terrain height
    2. Compute Normals
    3. For each vertex of quad (in vertex shader) offset quad a bit using normals
    4. Add color and get circular pattern using maths

## In practice

