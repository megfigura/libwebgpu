// These are being managed in the demo base code.
struct Camera {
  projection : mat4x4f,
  view : mat4x4f,
};
@group(0) @binding(0) var<uniform> camera : Camera;

// This comes from the bind groups being created in setupMeshNode in the next section.
struct Model {
  matrix: mat4x4f,
  normalMat: mat4x4f,
}
@group(1) @binding(0) var<uniform> model : Model;

// These locations correspond with the values in the ShaderLocations struct in our JS and, by
// extension, the buffer attributes in the pipeline vertex state.
struct VertexInput {
@location(${ShaderLocations.POSITION}) position : vec3f,
@location(${ShaderLocations.NORMAL}) normal : vec3f,
};

struct VertexOutput {
// Always need to at least output something to the position builtin.
@builtin(position) position : vec4f,

// The other locations can be anything you want, as long as it's consistent between the
// vertex and fragment shaders. Since we're defining both in the same module and using the
// same structure for the input and output, we get that alignment for free!
@location(0) normal : vec3f,
};

@vertex
fn vertexMain(input : VertexInput) -> VertexOutput {
// Determines the values that will be sent to the fragment shader.
var output : VertexOutput;

// Transform the vertex position by the model/view/projection matrices.
output.position = camera.projection * camera.view * model.matrix * vec4f(input.position, 1);

// Transform the normal by the normal and view matrices. Normally you'd just do normal matrix,
// but adding the view matrix in this case is a hack to always keep the normals pointing
// towards the light, so that we can clearly see the geometry even as we rotate it.
output.normal = (camera.view * model.normalMat * vec4f(input.normal, 0)).xyz;

return output;
}

// Some hardcoded lighting constants.
const lightDir = vec3f(0.25, 0.5, 1);
const lightColor = vec3f(1);
const ambientColor = vec3f(0.1);

@fragment
fn fragmentMain(input : VertexOutput) -> @location(0) vec4f {
// An extremely simple directional lighting model, just to give our model some shape.
let N = normalize(input.normal);
let L = normalize(lightDir);
let NDotL = max(dot(N, L), 0.0);

// Surface color will just be the light color, so everything will appear white/grey.
let surfaceColor = ambientColor + NDotL;

// No transparency at this point.
return vec4f(surfaceColor, 1);
}