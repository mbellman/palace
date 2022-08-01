## Engine
* reduce ObjectPool memory usage (indices)
* mesh roughness (will require a new g-buffer channel)
* visibility culling dot threshold
* log mesh instance counts on init
* 2D sprite rendering/UI system

## Graphics
* bloom lighting
* volumetric lighting (for shadowcasters)
* point + spot light center darkening fix/energy conservation tweaks

## Game internals
* move orientationTo/From/startTime to cameraState
* finish up mesh/zone configs
* procedures for console commands

## World design
* lower LoD for rose bushes
* garden boundary hedges
* garden boundary iron gates
* change lighting upon entering palace 1
* staircase mesh for palace 1
* staircase railing mesh for palace 1
* wall mesh for palace 1
* atrium wall structure for palace 1