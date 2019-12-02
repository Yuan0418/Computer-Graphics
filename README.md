Create 3 teapots in the scene with mesh
1. interactive manipulation & hierarchical transform
  Press 'x' to translate all objects, the 2 teapots are rotating.
2. Lit and shaded
  Set the light color, position, and object color;
  Final color=(ambient+diffuse+specular)*objectColor;
3. multiple camera views
  a. First person view:
  view = look_at(vec3(trans_x, trans_y, trans_z), vec3(0.0, 0.0, 0.0), vec3(0.0, 1. 0,0.0));
  //use keyboard control xyz to move the camera position.
b. top-down view:
  view = look_at(vec3(0,10,0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0));
  //make the camera look to y so that to see the downside objects.
  The first-person view is using perspective projection and the top-down view is using orthographic projection.
  
  
  Here is the link of my YouTube showing the program:
  https://www.youtube.com/watch?v=sI67VOZJm1M
