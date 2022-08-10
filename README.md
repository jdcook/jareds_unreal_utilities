# Jared's Unreal Utilities
A collection of helpful UE utilities

## Multi Spline Mesh
C++ construction script to lay multiple spline meshes along a long spline. This allows you to make things like racing game loops and corkscrews easily, or lay out aesthetic-only repeatable meshes.

Create a blueprint based on MultiSplineMesh and assign a mesh. Then edit it like a normal spline and it will construct SplineMeshes along it.

You can also use the properties under "Spawn Item Along Spline" to spawn things along the spline at intervals, such as pickups.

