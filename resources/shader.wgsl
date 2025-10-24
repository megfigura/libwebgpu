struct VertexInput {
  @location(0) position: vec3f,
  @location(1) color: vec3f,
};

struct VertexOutput {
  @builtin(position) position: vec4f,
  @location(0) color: vec4f,
};

struct UniformInput {
  @location(0) uTime: f32,
  @location(1) opacity: f32,
};

@group(0) @binding(0)
var<uniform> uniformStruct: UniformInput;
@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	let ratio = 800.0 / 600.0;
	let angle = uniformStruct.uTime;
	let alpha = cos(angle);
	let beta = sin(angle);
	var position = vec3f(
		in.position.x,
		alpha * in.position.y + beta * in.position.z,
		alpha * in.position.z - beta * in.position.y,
	);
	out.position = vec4f(position.x, position.y * ratio, position.z * 0.5 + 0.5, 1.0);
	out.color = vec4f(in.color[0] * uniformStruct.opacity, in.color[1] * uniformStruct.opacity, in.color[2] * uniformStruct.opacity, uniformStruct.opacity);
	return out;
}
// Add this in the same shaderSource literal than the vertex entry point
@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	//return vec4f(0.5 + ((in.color[0] * sin(uTime)) / 2.0), 0.5 + ((in.color[1] * cos(uTime)) / 2.0), 0.5 + ((in.color[2] * sin(uTime) * sin(uTime)) / 2.0), 0.01);
	return vec4f(in.color);
}