# Jared's Unreal Utilities
A collection of helpful UE utilities

## Multi Spline Mesh
C++ construction script to lay multiple spline meshes along a long spline. This allows you to make things like racing game loops and corkscrews easily, or lay out aesthetic-only repeatable meshes.

Create a blueprint based on MultiSplineMesh and assign a mesh. Then edit it like a normal spline and it will construct SplineMeshes along it.

You can also use the properties under "Spawn Item Along Spline" to spawn things along the spline at intervals, such as pickups.

NOTE: this code gets the length of each mesh from the mesh's `Mesh->GetBoundingBox().GetSize()`, so if that is incorrect for the mesh you are using, you will have to add a float property to the header that you can manually set, and get the length from that instead. Most meshes set up for spline usage have a clear bounding box, so you probably won't need to do this.

