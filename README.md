# LightningEngine
LightningEngine is a custom C++20 game engine, built mostly as a playground for exploring low-level systems and rendering techniques. 

At the moment:
 - ECS core – SparseSet-based entity/component system with views, iteration, and some observer mechanics.
 - Job system – update passes, auto-scheduling of jobs with dependency tracking, and groundwork for multithreading.
 - Rendering pipeline – custom RHI layer with a D3D11 backend and shader reflection/compilation. Early groundwork for render passes and a material system with permutations is there, but right now they’re still bare-bones and incomplete.
 - Threading model – early render thread separation (game thread updates proxies, render thread consumes them), with command queues and fences.

Stuff currently in progress:
 - More serious parallelization of rendering work.
 - Tooling (asset pipeline, editor).
 - Expanding the material/shader system to be more user-friendly.
 - Better documentation.

The end goal? A stable sandbox for experimenting with engine design, graphics features, and gameplay systems — something lightweight enough to hack on, but structured enough to scale.
