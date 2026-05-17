#let short = [DGI Project]
#let title = [Voxel Cone Tracing]
#let authors = [_
Corentin Jeanne \<cjeanne\@kth.se>#linebreak()
_]
#let authors_short = [_Corentin Jeanne_]
#let topic = [DH2323]

#set heading(numbering: "I.1.")

#set page(
  header: context {
    if counter(page).get().first() != 1 {
      return [_ #authors_short _ #h(1fr) #topic - #short]
    }
  },
  footer: context {
    align(center)[#counter(page).get().first() / #counter(page).final().first()]
  },
)

#show link: set text(fill: blue)
#show link: underline
#set par(justify: true)

#show bibliography: set heading(numbering: "I.")

#align(center)[
  #text(size: 20pt)[*#topic - #short*]

  #text(size: 13pt)[_ #title _]
]
#v(10pt)
#line(length: 100%, stroke: gray)
#v(10pt)
#align(center)[
  *#authors_short* #linebreak()
  #authors
  #align(horizon, figure(
    image("res/vct-64-full.png", width: 90%),
    caption: [Rendering of the Voxel Cone Tracing algorithm implemented using SDL2],
  ))
]
#v(30pt)

#pagebreak()

#align(horizon)[
  #outline()
]

#pagebreak()


= Introduction

Voxel Cone Tracing is a rasterization algorithm to approximate global illumination. Unlike a ray tracer, where shadows are easy to compute because light is fully simulated, a rasterizer has no direct ways to compute them as it processes one triangle at a time. Global illumination is not limited to shadows as well. It also includes ambient color from nearby objects, reflection, and glare, for example.
While none of this can easily be computed by a rasterizer, it can be approximated by algorithms such as voxel cone tracing.

My goal for this project was to reimplement the algorithm in SDL2, based on the degree project _Voxel Cone Tracing Evaluation for Real-Time Applications_ by Alvar Trojenborg, Filip Northman @vct and on their OpenGL implementation @git. My final implementation is available on GitHub @cj-vct.

= The Voxel Cone Tracing Algorithm

The Voxel Cone Tracing Algorithm (VCT) aims to approximate global illumination by first computing the correct direct light and approximating the indirect light each pixel receives.

This is done in three steps. First, a shadow map is used to generate the direct lighting data for the scene, then the scene is voxelized, and finally those voxels are used in combination with cone-tracing to compute indirect light. Each individual step is detailed below.

== Computing the Shadows

The first step in VCT is to compute a shadow map. It is a destructure used to check if a point receives direct light. It consists of a depth buffer associated with a direct light. Using the depth data, we can easily check the visibility of a pixel @yt-shadow.

To compute and use a shadow map, we first render the scene using a light camera and only preserve the depth buffer. The resolution of this camera can differ from the resolution of the main camera, as in @shadowmap. Then, to check the visibility of a 3D point, we compute the corresponding pixel on the shadow map and realize a depth check: if the depth of the pixel is greater than the stored value, this point is behind an object from the light's point of view and is not visible; otherwise, the point is fully visible. When testing, the data structure returns a float, usually 0 or 1, where 1 means fully visible and 0 fully hidden. The value can be between when interpolating points to de-noise the texture @yt-shadow.

#figure(
  grid(
    columns: (1fr, 1fr),
    column-gutter: 10,
    image("res/shadowmap-scene.png", width: 80%), image("res/shadow-map-light.png", width: 80%),
  ),
  caption: [Full scene (left), Depth view from the light (right) @wiki-shadow],
) <shadowmap>


Only directional light can be mapped one-to-one to a shadow map. Point lights need to split into multiple directional lights to compute a shadow cube. To check the visibility of a point, you must first get the correct map and then do the depth test.

== Voxelizing the scene <sec-voxelization>

The second step of VCT is to voxelize the scene and turn it into a 3-dimensional grid of voxels. Each voxel is assigned a color, taking into account the reflectance of the material and the direct light it receives. This uses the shadow map computed earlier in the process.

One easy way to voxelize the entire scene is to render it using an orthographic camera with the same resolution as the voxel grid. This way the fragment shader can be used to store the voxel information, as each pixel directly maps to a voxel. Since the fragment shader also has access to the corresponding triangle's normal and interpolated 3D position, computing the direct light of that voxel is possible.

To find out the amount of light reaching the point, we can assume a point of power $p$ radiating light in a perfect sphere. In that case, the amount of energy $e$ received by a point at a distance $r$ would be $e = p / (4pi r^2)$. The final direct light equation, taking into account the normal of the painted triangle and a point light, would be the following:
$
  italic("color") = italic("reflectance") * italic("v") * (max(0, arrow(n) dot arrow(l)) * italic("light")) / (4pi norm(italic("pos")_"light" - italic("pos")_"point")^2)
$

#figure(
  grid(
    columns: (1fr, 1fr),
    column-gutter: 10,
    image("res/lab3.png", width: 80%), image("res/voxels with axis selection.png", width: 80%),
  ),
  caption: [Example scene (left), Voxelized scene (right)],
) <voxelization>

== Cone Tracing

The third and final step of VCT is to use cone tracing to compute the indirect light received by a pixel. Here we take full benefit of the voxel grid by using it to quickly check for the surrounding light. This is done by casting a cone from the original position in a half-sphere around our surface, as shown in @cones. Each cone samples color and occlusion from the voxels on its path. The results are then combined to find the indirect illumination of a pixel.

#figure(
  image("res/cones.png", width: 60%),
  caption: [Multiple diffuse cones over one smaller specular cone sampling indirect diffuse and
    specular lighting respectively. @crassin],
) <cones>

Cones also use Level Of Detail (LOD) when sampling, as shown in @cones-lod. The first simple has an LOD of 1 and samples a single voxel; the second has an LOD of 2 and samples $2*2*2=8$ voxels, etc. At each step, the color and occlusion are updated using front-to-back compositing using the following formulas, with $c_n, a_n$ the color and occlusion after $n$ samples, and $c_s, a_s$ the sampled color and occlusion:
$
  c_n = c_(i-1) + (1-a_n) * a_s * c_s \
  a_n = a_(i-1) + (1-a_n) * a_s
$

#figure(
  image("res/cones-lod.png", width: 40%),
  caption: [Cones level of details. @crassin],
) <cones-lod>

== Final Color

To get the final color of the voxel, we compose both the direct light and indirect light by the reflectance of the color.
$
  italic("color") = (italic("direct light") + italic("indirect light")) * italic("reflectance")
$

The direct light can be computed using the same formula as in @sec-voxelization, and the indirect light is found by multiplying the cone-traced color by its ambiant occlusion.

= Implementation
// - imple details
// - issues
// - results
== Base architecture

The root of the code was based on the SDL2 rasterizer implemented in lab 3. It was heavily refactored to be easier to modify. The whole rendering part is now comprised of the `Renderer` class. This class exposes an API to render points, lines, and polygons using a specific shader and render mode.

The draw mode can be either `Vertices` to only draw the points, `Wireframe` to draw the outlines, or `Default` to fill in the shapes. A `Shader` is a C++ class providing a `VertexShader` and a `FragmentShader` method. The vertex shader translates world coordinates (a `Vertex`) to screen position (a `Pixel`). The renderer then interpolates all the drawn pixels of the shapes and calls the fragment shader for each one.

The definitions of those classes are visible in @renderer-setup.
#figure(
  kind: "raw",
  text(size: 10pt)[
    ```cpp
    struct Vertex {
        glm::vec3 position;
    };
    struct Pixel {
        int x; int y;
        float z;
        glm::vec3 pos3d;
    };

    class Shader {
    public:
        virtual Pixel VertexShader(const Vertex&) = 0;
        virtual void FragmentShader(const Pixel&) = 0;
    };

    enum class DrawMode { Default, Wireframe, Vertices, };

    class Renderer {
    public:
        void DrawPolygon(const std::vector<Vertex>&, Shader&, DrawMode);
        void DrawLine(Vertex, Vertex, Shader&, DrawMode);
        void DrawPoint(Vertex, Shader&, DrawMode);
    };
    ```
  ],
  supplement: "Excerpt",
  caption: [Definitions from `Renderer.hpp`],
) <renderer-setup>

When rendering, individual shaders may require additional parameters, either per triangle or globally. The global settings are set up in the function `SetupShaders` and the per-triangle settings are set before the draw call, as seen in @shader-setup.
#figure(
  kind: "raw",
  text(size: 10pt)[
    ```cpp
    // void SetupShader():
    lab3Shader.camera = &camera;
    lab3Shader.light = &light;
    lab3Shader.indirectLight = &ambiantLight;
    lab3Shader.screen = sdlAux;
    lab3Shader.depthBuffer = depthBuffer;

    // void Draw():
    for (const auto& t : triangles) {
      lab3Shader.normal = t.normal;
      lab3Shader.reflectance = t.color;
      renderer.DrawPolygon({ {t.v0}, {t.v1}, {t.v2} }, lab3Shader, mode);
    }
    ```
  ],
  supplement: "Excerpt",
  caption: [Shader setup for the Lab3 shader],
) <shader-setup>

Finally, the objects present in the scene, such as cameras and lights, are defined in `Objects.hpp` as shown in @objects-setup, and the various shaders are defined in `Shaders.hpp`.
#figure(
  kind: "raw",
  text(size: 10pt)[
    ```cpp
    struct Camera {
        glm::vec3 position;
        glm::mat3 rotation;
        glm::ivec2 resolution;
        float focale;
    };

    struct PointLight {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
    };
    ```
  ],
  supplement: "Excerpt",
  caption: [Definitions from `Objects.hpp`],
) <objects-setup>

== Voxelization

Due to challenges when implementing the shadow map, the voxelization comes first in my implementation. The reason for this change is explained in the corresponding in @vct-shadows.

The voxelization is done by the class `VoxelizationShader` and follows the steps from @sec-voxelization: rendering the scene with a custom resolution and storing the color data in a custom class, `VoxelGrid`. This class serves as easy access to the voxel data, especially as the 3D grid is stored as a 1D array.

#figure(
  grid(
    columns: (1fr, 1fr),
    column-gutter: 10,
    image("res/voxels first render.png", width: 80%), image("res/voxels with axis selection.png", width: 80%),
  ),
  caption: [Voxelization without axis selection (left), voxelization using the normal's axis (right)],
) <vox-gaps>

An important point of voxelization is choosing the correct axis when projecting the triangle, as always choosing the same axis will lead to gaps in the texture, as shown in @vox-gaps. An easy way to ensure good results is to project on the axis that would lead to the highest number of pixels. It can be found by using the dominant component of the normal. A normal majorly on the X-axis means that the triangle will be the most "flat" and visible when looking at it from this axis.

Since the shadows have not been computed yet, this step only stores the radiance of the voxels.

== Shadow Mapping <vct-shadows>

Creating a shadow map was a challenge I did not manage to overcome in time. When I started my implementation, I prioritized having a working rendering pipeline to be able to complete the algorithm. As such, the shadows are based on raycasts using the voxel grid as obstacles and are stored per voxel. Due to this process requiring the voxels to already be computed, this step is executed after the voxelization.

Instead of storing the shadow data in a depth buffer, I opted to store it in a grid of the same dimensions as the voxel grid. This way, I could do a second voxelization pass to modify the colors.
Instead of rendering the scene from the light's point of view, I opted to render the scene in the same manner as the voxelization. This allows me to, per voxel, raycast towards the light to see if it is visible and then store this data in the shadow grid and modify the voxel color as shown in @vox-shadows.
#figure(
  image("res/voxel shadows.png", width: 53%),
  caption: [Voxels with shadows applied],
) <vox-shadows>

== Cone Tracing

Unlike the other two steps, Cone Tracing is no a shader, but the part of the final shader, `RenderShader`, responsible for computing indirect light.

The implementation of the algorithm is straight forward: iterate every cone, and for every cone step until the cone is fully occluded. To improve the quality of the results, the cone starts at an offset. This prevents the cone from hitting the source voxel and returning the wrong color. The results from the cone tracing are visible in @vox-cones.

#figure(
  image("res/indirect.png", width: 53%),
  caption: [Indirect light resulting from cone tracing],
) <vox-cones>

== Final color output

The final step in the process is to determine the final color of the pixel.
Since both the direct light and indirect light were already computed, this task is rather simple. All that is needed is to combine both results and multiply them by the reflectance of the material. The different steps can be seen in @vox-color.


#figure(
  grid(
    columns: (1fr, 1fr, 1fr),
    column-gutter: 10,
    image("res/direct.png", width: 90%),
    image("res/indirect.png", width: 90%),
    image("res/vct-16-full.png", width: 90%),
  ),
  caption: [Direct light, indirect light, and final color output],
) <vox-color>

= Results

== Render frames

@res-renders compiles the render of labs 2 and 3, as well as the render of the VCT algorithm with a grid size from 8 to 64 voxels per axis and with full or half cones. When using half-cone, the same cones are cast, but with their angle divided in half to save compute time. The scene has also been remodeled in Blender, and the render is shown in @blender.

#figure(
  image("res/blender.png", width: 60%),
  caption: [Render of the Scene in Blender]
) <blender>

#figure(
  grid(
    columns: (auto, auto, auto),
    rows: (auto, auto, auto, auto, auto),
    row-gutter: 14pt,
    align(horizon + right)[*Labs 2 and 3*], image("res/lab2.png", width: 65%), image("res/lab3.png", width: 65%), 
    align(horizon + right)[*VCT 8*], image("res/vct-8-half.png", width: 65%), image("res/vct-8-full.png", width: 65%), 
    align(horizon + right)[*VCT 16*], image("res/vct-16-half.png", width: 65%), image("res/vct-16-full.png", width: 65%), 
    align(horizon + right)[*VCT 32*], image("res/vct-32-half.png", width: 65%), image("res/vct-32-full.png", width: 65%), 
    align(horizon + right)[*VCT 64*], image("res/vct-64-half.png", width: 65%), image("res/vct-64-full.png", width: 65%), 
  ),
  caption: [Render of the example scene with different configurations],
) <res-renders>

== Render times
@res-times compiles the render of labs 2 and 3, as well as the render of the VCT algorithm with 8 to 64 voxels per axis and with full or half cones. All the programs were compiled in release mode (`set(CMAKE_BUILD_TYPE Release)`) with no additional gcc flags.

#figure(
  table(
    columns: (auto, 1fr, 1fr, 1fr, 1fr, 1fr, 1fr, 1fr, 1fr, 1fr, 1fr),

    table.header(
      table.cell(rowspan: 2, [*Renderer*]),
      table.cell(rowspan: 2, [*Lab 2*]),
      table.cell(rowspan: 2, [*Lab 3*]),
      table.cell(colspan: 2, [*VCT 8*]),
      table.cell(colspan: 2, [*VCT 16*]),
      table.cell(colspan: 2, [*VCT 16*]),
      table.cell(colspan: 2, [*VCT 16*]),
      [*Half*], [*Full*], [*Half*], [*Full*],
      [*Half*], [*Full*], [*Half*], [*Full*],
    ),

    [*Time (ms)*], $110$, $7$, $295$, $2580$, $374$, $4285$, $381$, $4839$, $415$, $5094$,
  ),
  caption: [Average render time of the example scene with different configurations],
) <res-times>

== Render frames using Direct illumination only
Since the labs did not include indirect illumination, I also test the renderer keeping only the direct illumination using only the lab 2 and 3 renderer, as well as VCT with grids from 8x8x8 up to 64x64x64. The results are compiled in @res-renders-direct.
#figure(
  grid(
    columns: (1fr, 1fr, 1fr),
    rows: (auto, auto),
    row-gutter: 14pt,
    image("res/lab2.png", width: 90%), 
    image("res/lab3.png", width: 90%), 
    image("res/direct-8.png", width: 90%), 
    image("res/direct-16.png", width: 90%), 
    image("res/direct-32.png", width: 90%), 
    image("res/direct-64.png", width: 90%), 
  ),
  caption: [Render of the example scene from respectively, Lab 2, Lab 3, VCT 8, VCT 16, VCT 32, VCT 64],
) <res-renders-direct>

== Render Times using Direct illumination only
@res-times-direct shows the average time over 10 frames for various rendering renderers: the lab 2 and 3 renderer, as well as VCT with grids from 8x8x8 up to 64x64x64. All the programs were compiled in release mode (`set(CMAKE_BUILD_TYPE Release)`) with no additional gcc flags.

#figure(
  table(
    columns: (auto, 1fr, 1fr, 1fr, 1fr, 1fr, 1fr),
    
    table.header(
      [*Renderer*],
      [*Lab 2*],
      [*Lab 3*],
      [*VCT 8*],
      [*VCT 16*],
      [*VCT 32*],
      [*VCT 64*],
    ),
    
    [*Time (ms)*], $110$, $7$, $22$, $26$, $36$, $57$,
  ),
  caption: [Average render time over 10 frames of the example scene with different configurations],
) <res-times-direct>

= Discussion

== Shadows
The logical next step in this implementation is to finalize the implementation of a shadow map and cube. The process was already well on its way, and the code has been pushed to the branch `feat/shadow-map`. The issues I encountered were performance issues when close to triangles and when rendering triangles behind the camera. With the light in the middle of the scene, I couldn't compute a single face of the cube even after a minute. I also encountered issues with the light camera matrices, leading to strange transforms. As this happened towards the end of the project, I preferred to allocate my time to the report and optimizing the cone casting part over finishing fixing the renderer. This is a point to keep in mind as this has a great impact on the quality of the shadows.

== Visual quality of the output
As the voxel grid changed, the fidelity of the output compared to the reference Blender render varied. For larger grid sizes, the shadows are more precise but also much darker. For smaller grids, the opposite is true, but we also see loads of incorrect artifacts on the wall. I would argue that for such a simple scene, a simpler direct light and ambient light-only model would result in much more accurate results while being faster, as seen in the direct light-only renders.

== Impact of the Voxel Grid resolution
When comparing the output of the render with the reference Blender render, the results seem much darker on the floor and the shadows and much brighter on the roof. This issue is more visible as the resolution of the grid increases. I suspect this issue is hardly avoidable and is caused by the LOD when sampling. As the resolution increases, more and more voxels are sampled. However, only the voxels around the objects are filled in. This means that when sampling a face, only some of the voxels from the face's triangle count towards occlusion, and the resulting ambient occlusion is lowered and the ambient light diminished.

== Render speed
As VCT can be implemented on top of a rasterizer, it can benefit from the high performance of the rendering for simple scenarios. This is clearly visible when only rendering direct light. Even for a 64x64x64 voxel grid, the render is twice as fast as the ray tracer, although with slightly pixelated shadows. Since the ray tracer only computed a single bounce, the times are not directly comparable with VCT, as the work is not equivalent. As the render for half cones is at worst 300 ms slower, it is likely that adding a bounce to the ray tracer would make it slower, but without testing, nothing is certain.

== Memory Consumption
What Voxel Cone Tracing gains in speed, it loses in memory. The algorithm needs to store a shadow map as well as the whole scene in voxels. With a grid size of 512 voxels per axis, which appeared frequently in the other papers, the voxel grid would consume around 1.5 GB of memory. Looking at the size of modern 3D environments, it would be nearly impossible to store the entire voxel grid in memory.

= Conclusion

The scope set for this project was to discover and implement indirect illumination in a rasterizer using the Voxel Cone Tracing algorithm. This task was successfully completed except for the shadow map, which had to be worked around due to limitations of the renderer. While this algorithm is not comparable in workload to the ray tracer from Lab 2, as it did not use global illumination. Only using the direct illumination of the algorithm led to much quicker render times, at the cost of high memory consumption. This high memory usage is the main downside of the algorithm, as it requires the whole scene to be voxelized to work correctly and a high voxel grid resolution to achieve more accurate results.

#bibliography("res/sources.yml", style: "association-for-computing-machinery", title: [References])
