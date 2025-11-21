struct Camera {
  projection : mat4x4f,
  view : mat4x4f,
  position : vec3f,
  time : f32
};
@group(0) @binding(0) var<uniform> camera : Camera;

struct Model {
  worldMat : mat4x4f,
  normalMat : mat4x4f
};
@group(1) @binding(0) var<uniform> model : Model;

struct VertexInput {
  @location(0) position: vec3f,
  @location(1) normal: vec3f,
};

struct VertexOutput {
  @builtin(position) position: vec4f,
  @location(0) normal: vec3f,
  @location(1) cameraPos: vec3f
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out : VertexOutput;
	out.position = camera.projection * camera.view * model.worldMat * vec4f(in.position, 1);
	out.normal = (camera.view * model.normalMat * vec4f(in.normal, 0)).xyz;
	out.cameraPos = in.position;
	return out;
}

// Some hardcoded lighting
const lightDir = vec3f(0.25, 0.5, 1);
const lightColor = vec3f(1);
const ambientColor = vec3f(0.1);

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	// An extremely simple directional lighting model, just to give our model some shape.
    let N = normalize(in.normal);
    let L = -normalize(in.cameraPos);
    let NDotL = max(dot(N, L), 0.0);
    let surfaceColor = ambientColor + NDotL;

    return vec4f(surfaceColor, 1);
}