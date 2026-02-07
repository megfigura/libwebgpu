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
@group(1) @binding(1) var texSampler : sampler;
@group(1) @binding(2) var tex : texture_2d<f32>;
@group(1) @binding(3) var metallicRoughnessTexture : texture_2d<f32>;
@group(1) @binding(4) var emissiveTexture : texture_2d<f32>;
@group(1) @binding(5) var occlusionTexture : texture_2d<f32>;
@group(1) @binding(6) var normalTexture : texture_2d<f32>;

struct VertexInput {
  @builtin(vertex_index) vertex_index: u32,
  @location(0) position: vec3f,
  @location(1) normal: vec3f,
  @location(2) tangent: vec3f,
  @location(3) bitangent: vec3f,
  @location(4) texCoord: vec2f
};

struct VertexOutput {
  @builtin(position) position: vec4f,
  @location(0) worldPos: vec3f,
  @location(1) worldNormal: vec3f,
  @location(2) worldTangent: vec3f,
  @location(3) worldBitangent: vec3f,
  @location(4) texCoord: vec2f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out : VertexOutput;
	out.position = camera.projection * camera.view * model.worldMat * vec4f(in.position, 1);
	out.worldPos = (model.worldMat * vec4f(in.position, 1)).xyz;
	out.worldNormal = (model.worldMat * vec4f(in.normal, 0)).xyz;
	out.worldTangent = (model.worldMat * vec4f(in.tangent, 0)).xyz;
	out.worldBitangent = (model.worldMat * vec4f(in.bitangent, 0)).xyz;
	out.texCoord = in.texCoord;
	return out;
}

const PI = 3.14159265359;

fn D_GGX(NoH: f32, a: f32) -> f32 {
    let a2 = a * a;
    let f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
}

fn F_Schlick(u : f32, f0 : vec3f) -> vec3f {
    return f0 + (vec3f(1.0) - f0) * pow(1.0 - u, 5.0);
}

fn V_SmithGGXCorrelated(NoV : f32, NoL : f32, a : f32) -> f32 {
    let a2 = a * a;
    let GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    let GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}

fn Fd_Lambert() -> f32 {
    return 1.0 / PI;
}

fn F_Schlick_Scalar(u : f32, f0 : f32, f90 : f32) -> f32 {
    return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

fn Fd_Burley(NoV : f32, NoL : f32, LoH : f32, roughness : f32) -> f32 {
    let f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    let lightScatter = F_Schlick_Scalar(NoL, 1.0, f90);
    let viewScatter = F_Schlick_Scalar(NoV, 1.0, f90);
    return lightScatter * viewScatter * (1.0 / PI);
}

fn light(in: VertexOutput, lightPos : vec3f, intensity : f32) -> vec3f {

    let v = normalize(camera.position - in.worldPos); // view unit vector
    let l = normalize(lightPos - in.worldPos); // light unit vector

    let h = normalize(v + l);

    //let n = normalize(in.worldNormal);
    let localN = (textureSample(normalTexture, texSampler, in.texCoord).rgb * 2.0) - 1.0;
    let normalTransform = mat3x3f(
        normalize(in.worldTangent),
        normalize(in.worldBitangent),
        normalize(in.worldNormal));
    let n = normalize(normalTransform * localN);

    let NoV = abs(dot(n, v)) + 1e-5;
    let NoL = clamp(dot(n, l), 0.0, 1.0) + 1e-5; // Added to prevent NaN
    let NoH = clamp(dot(n, h), 0.0, 1.0);
    let LoH = clamp(dot(l, h), 0.0, 1.0);

    //let metallic = 0.0;
    //let perceptualRoughness = 0.5;
    let MR = textureSample(metallicRoughnessTexture, texSampler, in.texCoord).bg;
    let metallic = MR[0];
    //let perceptualRoughness = MR[1];
    let roughness = MR[1];

    let reflectance = 0.04;
    //let roughness = perceptualRoughness * perceptualRoughness;

    let baseColor = textureSample(tex, texSampler, in.texCoord).rgb;
    //let baseColor = vec3f(1, 0, 0);
    let diffuseColor = (1.0 - metallic) * baseColor;

    //let f0Gold = vec3f(1.00, 0.85, 0.57);
    //let f0 = f0Gold;
    let f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + baseColor * metallic;

    let D = D_GGX(NoH, roughness);
    let F = F_Schlick(LoH, f0);
    let V = V_SmithGGXCorrelated(NoV, NoL, roughness);

    let Fr = (D * V) * F;
    //let Fd = diffuseColor * Fd_Lambert();
    let Fd = diffuseColor * Fd_Burley(NoV, NoL, LoH, roughness);

    return vec3f(Fr + Fd) * NoL * intensity;
}

fn exposureSettings(aperture : f32, shutterSpeed : f32, sensitivity : f32) -> f32 {
    return log2((aperture * aperture) / shutterSpeed * 100 / sensitivity);
}

fn exposure(ev100 : f32) -> f32 {
    return 1.0 / (pow(2.0, ev100) * 1.2);
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {

    var color = light(in, camera.position + vec3f(0, -5, 5), 10); //light(in, vec3f(0, 10, 5));
    color += light(in, vec3f(10, 10, 0), 1);
    color += light(in, vec3f(-10, 5, 10), 1);
    color += light(in, vec3f(2, -5, -10), 1);

    let emissive = textureSample(emissiveTexture, texSampler, in.texCoord).rgb;

    let gammaColor = pow(color / (color + vec3f(1)), vec3f(1.0 / 2.2)); // convert linear -> srgb

    let ev100 = exposureSettings(1.4, 0.2, 1200);
    let exposure = exposure(ev100);

    return clamp(vec4f(gammaColor * exposure + (emissive * 1.0), 1.0), vec4f(0), vec4f(1));
}