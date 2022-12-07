## Model Loading
The models were first exported from blender using the .gltf format, specifically, gltf separate which meant all the textures that
are in the object is also exported. The textures were first baked then, when it was exported, the materials/texture was exported as 
a .png image. The model was then placed in the assets folder and textures in the texture folder. By making the class Content into a vector array
meant that I could define multiple n instances of the model, by using LoadGLTF and referencing the associated model that was to be loaded by Content.

The object was then loaded into the scene through .pushback and the model position, rotation and scale were also defined per object that was loaded through .pushback and specifying their positions in the scene manually.

## Scene Building
The scene was built in reference to the original scene created in Blender and in Unreal, trying to closely replicate the built scene. Taking inspiration from Dune and a dystopian scene, where buildings look destroyed and weathered from the changing climate and the sandstorm engulfing the buildings and objects such as the electrical poles and lamp posts. And a small space ship that is hovering in the air looking for signs of life. The scene was built using the model the vector rotation, scale and position values. 

## Materials used
The materials used were materials that i had created in blender, and baked and exported alongside the corresponding objects. Where a lot of the objects had a metallic material applied, where the material looks weathered through roughness applied to the material. The buildings, and lamp posts uses a Metallic material/texture. The wind mill uses a wooden coloured material. The ship uses a metallic material and the electrical posts also uses a lighter and rougher colour of metal.

## Lighting


## Interactions
There are two interactions implemented in the scene, The first main interaction is the user movement using the WASD and Space keys to move around the scene, as well as the mouse cursor fov movement for the player to view the scene. 

The second interaction implemented is the ability to change the lighting for the scene through the dragsliders implemented, it allows the user to change the light position, ambience, diffusion and specular of the scene to their liking. This can be done by pressing the X key to enable the mouse cursor and then simply pressing and holding down the mouse button on either value and dragging the mouse left to decrease the value and right to increase the value.

## Issues encountered
Some models were omitted from the scene such as some buildings and also the house model due to the fact that the models when exported and loaded were not loading correctly into the scene, often missing large pieces of it, as such the scene lacks a variation of models. 


