## Engine
* rename NON_EMISSIVE -> STANDARD or DEFAULT
* reduce ObjectPool memory usage (indices)
* mesh roughness (will require a new g-buffer channel)
* visibility culling dot threshold
* light.serializable
* log mesh instance counts on init

## Graphics
* bloom lighting
* volumetric lighting (for shadowcasters)

## Game internals
* move orientationTo/From/startTime to cameraState

## World design:
* lower LoD for rose bushes
* garden boundary hedges
* change lighting upon entering palace 1
* column mesh for palace 1
* staircase mesh for palace 1
* staircase railing mesh for palace 1
* wall mesh for palace 1
* atrium wall structure for palace 1